/*
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */
/* This file contains C code that implements
 * the video decoder model.
 */
#include <setjmp.h>
#include <time.h>

#include "video.h"
#include "decoders.h"

/*
 * We use a lookup table to make sure values stay in the 0..255 range.
 * Since this is cropping (ie, x = (x < 0)?0:(x>255)?255:x; ), wee call this
 * table the "crop table".
 * MAX_NEG_CROP is the maximum neg/pos value we can handle.
 */

#define MAX_NEG_CROP 384
#define NUM_CROP_ENTRIES (256+2*MAX_NEG_CROP)
static unsigned char cropTbl[NUM_CROP_ENTRIES];

static jmp_buf setjmp_buffer;

#pragma warn -rvl
void *ex_alloc( VidStream *vid_stream, unsigned long size )
{
  void *p = (*vid_stream->alloc)(vid_stream, size);
  if (p) return p;
  (*vid_stream->put_message)("\nInsufficient memory");
  longjmp(setjmp_buffer, 4);
}
#pragma warn +rvl

void PrintTimeInfo( VidStream *vid_stream )
{
  long spent = (clock() - vid_stream->realTimeStart) * (1000 / CLK_TCK);

  (*vid_stream->put_message)
    ("\nReal Time Spent (After Initializations): %ld,%03ld secs.\n",
     spent / 1000, spent % 1000);
}

static void ycc_rgb_convert( VidStream *vid_stream )
{
  YCC_RGB_TAB *tab = &ycc_rgb_tab;
  int num_cols = vid_stream->dest_width;
  int num_rows = vid_stream->dest_height;
  unsigned char *ptr0 = vid_stream->red;
  unsigned char *ptr1 = vid_stream->green;
  unsigned char *ptr2 = vid_stream->blue;
  unsigned char *cm = cropTbl + MAX_NEG_CROP;
  long y = 0, cb = 0, cr = 0;

  do {
    int col = num_cols;
    do {
      (unsigned char) y = *ptr0;
      (unsigned char)cb = *ptr1;
      (unsigned char)cr = *ptr2;
      /* Note: if the inputs were computed directly from RGB values,
       * range-limiting would be unnecessary here; but due to possible
       * noise in the DCT/IDCT phase, we do need to apply range limits.
       */
      *ptr2++ = cm[y +   tab->cb_b[cb]];	/* blue */
      *ptr1++ = cm[y + ((tab->cb_g[cb] +	/* green */
			 tab->cr_g[cr]) >> 16)];
      *ptr0++ = cm[y +   tab->cr_r[cr]];	/* red */
    }
    while (--col > 0);
    col -= num_cols; col &= 15; ptr0 += col; ptr1 += col; ptr2 += col;
  }
  while (--num_rows > 0);
}


/*
 *--------------------------------------------------------------
 *
 * InitVidStream --
 *
 *	Initializes a VidStream structure. Takes
 *      as parameter requested size for buffer length.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

static void InitVidStream( VidStream *vid_stream )
{
  int i;
  static unsigned char default_intra_matrix[64] = {
/*
    real scheme:

     8, 16, 19, 22, 26, 27, 29, 34,
    16, 16, 22, 24, 27, 29, 34, 37,
    19, 22, 26, 27, 29, 34, 34, 38,
    22, 22, 26, 27, 29, 34, 37, 40,
    22, 26, 27, 29, 32, 35, 40, 48,
    26, 27, 29, 32, 35, 40, 48, 58,
    26, 27, 29, 34, 38, 46, 56, 69,
    27, 29, 35, 38, 46, 56, 69, 83 };

    zigzag scheme:
*/
     8, 16, 16, 19, 16, 19, 22, 22,
    22, 22, 22, 22, 26, 24, 26, 27,
    27, 27, 26, 26, 26, 26, 27, 27,
    27, 29, 29, 29, 34, 34, 34, 29,
    29, 29, 27, 27, 29, 29, 32, 32,
    34, 34, 37, 38, 37, 35, 35, 34,
    35, 38, 38, 40, 40, 40, 48, 48,
    46, 46, 56, 56, 58, 69, 69, 83 };

  /* Copy default intra matrix and
     initialize non intra quantization matrix. */

  for (i = 0; i < 64; i++) {
    vid_stream->intra_quant_matrix[i] = default_intra_matrix[i];
    vid_stream->non_intra_quant_matrix[i] = 16;
  }

  /* Initialize crop table. */

  for (i = -MAX_NEG_CROP; i < NUM_CROP_ENTRIES - MAX_NEG_CROP; i++)
    if (i <= 0)
      cropTbl[i+MAX_NEG_CROP] = 0;
    else if (i >= 255)
      cropTbl[i+MAX_NEG_CROP] = 255;
    else
      cropTbl[i+MAX_NEG_CROP] = i;

  /* Initialize bitstream i/o fields. */

  vid_stream->bits_left = 0;
  vid_stream->buf_length = 0;

  /* Load data. */

  (*(vid_stream->correct_underflow = vid_correct_underflow))
    (vid_stream);

  /* Check for System Layer Stream, initialize System Layer Junk. */

  if (*vid_stream->buffer == PACK_START_CODE ||
      *vid_stream->buffer == SYSTEM_HEADER_START_CODE) {
    if (vid_stream->vid_buf == 0)
      vid_stream->vid_buf = ex_alloc(vid_stream, BUF_LENGTH);
    vid_stream->sys_buf = (unsigned char *)vid_stream->buffer;
    vid_stream->bytes_left = vid_stream->buf_length << 2;
    vid_stream->buf_length = 0;
    vid_stream->packet_length = 0;
    vid_stream->EOF_flag = 0;
    (*(vid_stream->correct_underflow = sys_correct_underflow))
      (vid_stream);
  }
}


/*
 *--------------------------------------------------------------
 *
 * InitPictImage --
 *
 *	Initializes a PictImage structure.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

static void InitPictImage( VidStream *vid_stream, PictImage *p )
{
  long totpix;

  if (p->luminance) return;

  totpix = ((long)vid_stream->mb_width * vid_stream->mb_height) << 8;

  /* Allocate memory for image spaces. */

  p->luminance = ex_alloc(vid_stream, totpix);

  if (vid_stream->color) {
    if (vid_stream->red == 0) {
      vid_stream->red =
      vid_stream->old_ptr = ex_alloc(vid_stream, totpix * 3);
      vid_stream->green = vid_stream->red + totpix;
      vid_stream->blue = vid_stream->green + totpix;
      vid_stream->c_convert = ycc_rgb_convert;
      ycc_rgb_init();
    }
    totpix >>= 2;
    p->Cr = ex_alloc(vid_stream, totpix * 2);
    p->Cb = p->Cr + totpix;
  }
}


/*
 *--------------------------------------------------------------
 *
 * ReconIMBlock --
 *
 *	Reconstructs intra coded macroblock.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

static void ReconIMBlock( VidStream *vid_stream, int bnum )
{
  long row, col, row_size;
  unsigned char *dest, *cm;
  short *sp;

  /* Calculate macroblock row and column from address. */

  row = vid_stream->mblock.mb_address / vid_stream->mb_width;
  col = vid_stream->mblock.mb_address % vid_stream->mb_width;

  row_size = vid_stream->mb_width;

  /* If block is luminance block... */

  if (bnum < 4) {

    /* Establish row size. */

    row_size <<= 4;

    /* Calculate row and col values for upper left pixel of block. */

    row <<= 4; col <<= 4;
    if (bnum & 2) row += 8;
    if (bnum & 1) col += 8;

    /* Set dest to luminance plane of current pict image. */

    dest = vid_stream->current->luminance;
  }

  /* Otherwise, block is NOT luminance block, ... */

  else {

    /* Establish row size. */

    row_size <<= 3;

    /* Calculate row,col for upper left pixel of block. */

    row <<= 3; col <<= 3;

    if (bnum == 4)

      /* Set dest to Cr plane of current pict image. */

      dest = vid_stream->current->Cr;

    else

      /* Set dest to Cb plane of current pict image. */

      dest = vid_stream->current->Cb;
  }

  /* For each pixel in block, set to cropped reconstructed value
     from inverse dct.
  */

  dest += row * row_size + col;
  sp = vid_stream->block.dct_recon[0];
  cm = cropTbl + (MAX_NEG_CROP + 128);
  row_size -= 8; row = 8;
  do {
    *dest++ = cm[*sp++];
    *dest++ = cm[*sp++];
    *dest++ = cm[*sp++];
    *dest++ = cm[*sp++];
    *dest++ = cm[*sp++];
    *dest++ = cm[*sp++];
    *dest++ = cm[*sp++];
    *dest++ = cm[*sp++];

    dest += row_size;
  }
  while (--row > 0);
}


/*
 *--------------------------------------------------------------
 *
 * ReconPMBlock --
 *
 *	Reconstructs forward predicted macroblocks.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

static void ReconPMBlock( VidStream *vid_stream, int bnum, int zflag )
{
  long row, col, row_size;
  long right_for, down_for;
  unsigned char *dest, *rindex1, *rindex2, *cm;
  short *blockvals;

  /* Calculate macroblock row and column from address. */

  row = vid_stream->mblock.mb_address / vid_stream->mb_width;
  col = vid_stream->mblock.mb_address % vid_stream->mb_width;

  row_size = vid_stream->mb_width;

  right_for = vid_stream->mblock.recon_right_for;
  down_for = vid_stream->mblock.recon_down_for;

  /* If block is luminance block... */

  if (bnum < 4) {

    /* Set dest to luminance plane of current pict image. */

    dest = vid_stream->current->luminance;

    if (vid_stream->picture.code_type == B_TYPE)
      rindex1 = vid_stream->past->luminance;
    else

      /* Set predicitive frame to current future frame. */

      rindex1 = vid_stream->future->luminance;

    /* Establish row size. */

    row_size <<= 4;

    /* Calculate row,col of upper left pixel in block. */

    row <<= 4; col <<= 4;
    if (bnum & 2) row += 8;
    if (bnum & 1) col += 8;
  }

  /* Otherwise, block is NOT luminance block, ... */

  else {

    right_for >>= 1; down_for >>= 1;

    /* Establish row size. */

    row_size <<= 3;

    /* Calculate row,col of upper left pixel in block. */

    row <<= 3; col <<= 3;

    /* If block is Cr block... */

    if (bnum == 4) {

      /* Set dest to Cr plane of current pict image. */

      dest = vid_stream->current->Cr;

      if (vid_stream->picture.code_type == B_TYPE)
        rindex1 = vid_stream->past->Cr;
      else
        rindex1 = vid_stream->future->Cr;
    }

    /* Otherwise, block is Cb block... */

    else {

      /* Set dest to Cb plane of current pict image. */

      dest = vid_stream->current->Cb;

      if (vid_stream->picture.code_type == B_TYPE)
        rindex1 = vid_stream->past->Cb;
      else
        rindex1 = vid_stream->future->Cb;
    }
  }

  /* For each pixel in block... */

  dest += row * row_size + col;
  rindex1 += row * row_size + col;
  rindex1 += right_for >> 1; rindex1 += (down_for >> 1) * row_size;

  rindex2 = rindex1;
  rindex2 += right_for & 1; if (down_for & 1) rindex2 += row_size;

  row_size -= 8; row = 8;

  /*
   * Calculate predictive pixel value based on motion vectors
   * and copy to dest plane.
   */

  if (zflag) {
    if (rindex2 == rindex1)
      do {
	*dest++ = *rindex1++;
	*dest++ = *rindex1++;
	*dest++ = *rindex1++;
	*dest++ = *rindex1++;
	*dest++ = *rindex1++;
	*dest++ = *rindex1++;
	*dest++ = *rindex1++;
	*dest++ = *rindex1++;
	dest += row_size;
	rindex1 += row_size;
      }
      while (--row > 0);
    else
      do {
	*dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
        *dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
	dest += row_size;
	rindex1 += row_size;
	rindex2 += row_size;
      }
      while (--row > 0);
  }
  else {
    blockvals = vid_stream->block.dct_recon[0];
    cm = cropTbl + MAX_NEG_CROP;

    if (rindex2 == rindex1)
      do {
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	dest += row_size;
	rindex1 += row_size;
      }
      while (--row > 0);
    else
      do {
        *dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
        *dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
        *dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
        *dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
        *dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
        *dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
        *dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
        *dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
	dest += row_size;
	rindex1 += row_size;
	rindex2 += row_size;
      }
      while (--row > 0);
  }
}


/*
 *--------------------------------------------------------------
 *
 * ReconBMBlock --
 *
 *	Reconstructs back predicted macroblocks.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

static void ReconBMBlock( VidStream *vid_stream, int bnum, int zflag )
{
  long row, col, row_size;
  long right_back, down_back;
  unsigned char *dest, *rindex1, *rindex2, *cm;
  short *blockvals;

  /* Calculate macroblock row and column from address. */

  row = vid_stream->mblock.mb_address / vid_stream->mb_width;
  col = vid_stream->mblock.mb_address % vid_stream->mb_width;

  row_size = vid_stream->mb_width;

  right_back = vid_stream->mblock.recon_right_back;
  down_back = vid_stream->mblock.recon_down_back;

  /* If block is luminance block... */

  if (bnum < 4) {

    /* Set dest to luminance plane of current pict image. */

    dest = vid_stream->current->luminance;

    /* Set rindex1 to luminance plane of future frame. */

    rindex1 = vid_stream->future->luminance;

    /* Establish row size. */

    row_size <<= 4;

    /* Calculate row,col of upper left pixel in block. */

    row <<= 4; col <<= 4;
    if (bnum & 2) row += 8;
    if (bnum & 1) col += 8;
  }

  /* Otherwise, block is NOT luminance block, ... */

  else {

    right_back >>= 1; down_back >>= 1;

    /* Establish row size. */

    row_size <<= 3;

    /* Calculate row,col of upper left pixel in block. */

    row <<= 3; col <<= 3;

    /* If block is Cr block... */

    if (bnum == 4) {

      /* Set dest to Cr plane of current pict image. */

      dest = vid_stream->current->Cr;

      /* Set rindex1 to Cr plane of future image. */

      rindex1 = vid_stream->future->Cr;
    }

    /* Otherwise, block is Cb block... */

    else {

      /* Set dest to Cb plane of current pict image. */

      dest = vid_stream->current->Cb;

      /* Set rindex1 to Cb plane of future frame. */

      rindex1 = vid_stream->future->Cb;
    }
  }

  /* For each pixel in block do... */

  dest += row * row_size + col;
  rindex1 += row * row_size + col;
  rindex1 += right_back >> 1; rindex1 += (down_back >> 1) * row_size;

  rindex2 = rindex1;
  rindex2 += right_back & 1; if (down_back & 1) rindex2 += row_size;

  row_size -= 8; row = 8;

  if (zflag) {
    if (rindex2 == rindex1)
      do {
	*dest++ = *rindex1++;
	*dest++ = *rindex1++;
	*dest++ = *rindex1++;
	*dest++ = *rindex1++;
	*dest++ = *rindex1++;
	*dest++ = *rindex1++;
	*dest++ = *rindex1++;
	*dest++ = *rindex1++;
	dest += row_size;
	rindex1 += row_size;
      }
      while (--row > 0);
    else
      do {
	*dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*rindex2++) >> 1;
	dest += row_size;
	rindex1 += row_size;
	rindex2 += row_size;
      }
      while (--row > 0);
  }
  else {
    blockvals = vid_stream->block.dct_recon[0];
    cm = cropTbl + MAX_NEG_CROP;

    if (rindex2 == rindex1)
      do {
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	*dest++ = cm[(int)*rindex1++ + *blockvals++];
	dest += row_size;
	rindex1 += row_size;
      }
      while (--row > 0);
    else
      do {
	*dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*rindex2++)>>1) + *blockvals++];
	dest += row_size;
	rindex1 += row_size;
	rindex2 += row_size;
      }
      while (--row > 0);
  }
}


/*
 *--------------------------------------------------------------
 *
 * ReconBiMBlock --
 *
 *	Reconstructs bidirectionally predicted macroblocks.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

static void ReconBiMBlock( VidStream *vid_stream, int bnum, int zflag )
{
  long row, col, row_size;
  long right_for, down_for, right_back, down_back;
  unsigned char *dest, *rindex1, *bindex1, *rindex2, *bindex2, *cm;
  short *blockvals;

  /* Calculate macroblock row and column from address. */

  row = vid_stream->mblock.mb_address / vid_stream->mb_width;
  col = vid_stream->mblock.mb_address % vid_stream->mb_width;

  row_size = vid_stream->mb_width;

  right_for = vid_stream->mblock.recon_right_for;
  down_for = vid_stream->mblock.recon_down_for;
  right_back = vid_stream->mblock.recon_right_back;
  down_back = vid_stream->mblock.recon_down_back;

  /* If block is luminance block... */

  if (bnum < 4) {

    /* Set dest to luminance plane of current pict image. */

    dest = vid_stream->current->luminance;

    /* Set rindex1 to luminance plane of past frame. */

    rindex1 = vid_stream->past->luminance;

    /* Set bindex1 to luminance plane of future frame. */

    bindex1 = vid_stream->future->luminance;

    /* Establish row size. */

    row_size <<= 4;

    /* Calculate row,col of upper left pixel in block. */

    row <<= 4; col <<= 4;
    if (bnum & 2) row += 8;
    if (bnum & 1) col += 8;
  }

  /* Otherwise, block is NOT luminance block, ... */

  else {

    right_for >>= 1; down_for >>= 1;
    right_back >>= 1; down_back >>= 1;

    /* Establish row size. */

    row_size <<= 3;

    /* Calculate row,col of upper left pixel in block. */

    row <<= 3; col <<= 3;

    /* If block is Cr block... */

    if (bnum == 4) {

      /* Set dest to Cr plane of current pict image. */

      dest = vid_stream->current->Cr;

      /* Set rindex1 to Cr plane of past image. */

      rindex1 = vid_stream->past->Cr;

      /* Set bindex1 to Cr plane of future image. */

      bindex1 = vid_stream->future->Cr;
    }
    /* Otherwise, block is Cb block... */

    else {

      /* Set dest to Cb plane of current pict image. */

      dest = vid_stream->current->Cb;

      /* Set rindex1 to Cb plane of past frame. */

      rindex1 = vid_stream->past->Cb;

      /* Set bindex1 to Cb plane of future frame. */

      bindex1 = vid_stream->future->Cb;
    }
  }

  /* For each pixel in block... */

  dest += row * row_size + col;
  rindex1 += row * row_size + col;
  bindex1 += row * row_size + col;
  rindex1 += right_for >> 1; rindex1 += (down_for >> 1) * row_size;
  bindex1 += right_back >> 1; bindex1 += (down_back >> 1) * row_size;

  rindex2 = rindex1;
  bindex2 = bindex1;
  rindex2 += right_for & 1; if (down_for & 1) rindex2 += row_size;
  bindex2 += right_back & 1; if (down_back & 1) bindex2 += row_size;

  blockvals = vid_stream->block.dct_recon[0];
  cm = cropTbl + MAX_NEG_CROP;
  row_size -= 8; row = 8;

  if (rindex2 != rindex1 || bindex2 != bindex1) {
    if (zflag)
      do {
	*dest++ = ((int)*rindex1++ + (int)*bindex1++ +
		   (int)*rindex2++ + (int)*bindex2++) >> 2;
	*dest++ = ((int)*rindex1++ + (int)*bindex1++ +
		   (int)*rindex2++ + (int)*bindex2++) >> 2;
	*dest++ = ((int)*rindex1++ + (int)*bindex1++ +
		   (int)*rindex2++ + (int)*bindex2++) >> 2;
	*dest++ = ((int)*rindex1++ + (int)*bindex1++ +
		   (int)*rindex2++ + (int)*bindex2++) >> 2;
	*dest++ = ((int)*rindex1++ + (int)*bindex1++ +
		   (int)*rindex2++ + (int)*bindex2++) >> 2;
	*dest++ = ((int)*rindex1++ + (int)*bindex1++ +
		   (int)*rindex2++ + (int)*bindex2++) >> 2;
	*dest++ = ((int)*rindex1++ + (int)*bindex1++ +
		   (int)*rindex2++ + (int)*bindex2++) >> 2;
	*dest++ = ((int)*rindex1++ + (int)*bindex1++ +
		   (int)*rindex2++ + (int)*bindex2++) >> 2;
	dest += row_size;
	rindex1 += row_size;
	bindex1 += row_size;
	rindex2 += row_size;
	bindex2 += row_size;
      }
      while (--row > 0);
    else
      do {
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++ +
			(int)*rindex2++ + (int)*bindex2++)>>2) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++ +
			(int)*rindex2++ + (int)*bindex2++)>>2) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++ +
			(int)*rindex2++ + (int)*bindex2++)>>2) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++ +
			(int)*rindex2++ + (int)*bindex2++)>>2) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++ +
			(int)*rindex2++ + (int)*bindex2++)>>2) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++ +
			(int)*rindex2++ + (int)*bindex2++)>>2) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++ +
			(int)*rindex2++ + (int)*bindex2++)>>2) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++ +
			(int)*rindex2++ + (int)*bindex2++)>>2) + *blockvals++];
	dest += row_size;
	rindex1 += row_size;
	bindex1 += row_size;
	rindex2 += row_size;
	bindex2 += row_size;
      }
      while (--row > 0);
  }
  else
    if (zflag)
      do {
	*dest++ = ((int)*rindex1++ + (int)*bindex1++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*bindex1++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*bindex1++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*bindex1++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*bindex1++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*bindex1++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*bindex1++) >> 1;
	*dest++ = ((int)*rindex1++ + (int)*bindex1++) >> 1;
	dest += row_size;
	rindex1 += row_size;
	bindex1 += row_size;
      }
      while (--row > 0);
    else
      do {
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++)>>1) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++)>>1) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++)>>1) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++)>>1) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++)>>1) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++)>>1) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++)>>1) + *blockvals++];
	*dest++ = cm[(((int)*rindex1++ + (int)*bindex1++)>>1) + *blockvals++];
	dest += row_size;
	rindex1 += row_size;
	bindex1 += row_size;
      }
      while (--row > 0);
}


/*
 *--------------------------------------------------------------
 *
 * ProcessSkippedPFrameMBlocks --
 *
 *	Processes skipped macroblocks in P frames.
 *
 * Results:
 *	Calculates pixel values for luminance, Cr, and Cb planes
 *      in current pict image for skipped macroblocks.
 *
 * Side effects:
 *	Pixel values in pict image changed.
 *
 *--------------------------------------------------------------
 */

static void ProcessSkippedPFrameMBlocks( VidStream *vid_stream )
{
  long row_size, row, col, addr, rr;
  unsigned char *dest, *src, *dest1, *src1;

  /* Calculate row sizes for luminance and Cr/Cb macroblock areas. */

  row_size = vid_stream->mb_width << 4;

  /* For each skipped macroblock, do... */

  for (addr = vid_stream->mblock.past_mb_addr;
       ++addr < vid_stream->mblock.mb_address;) {

    /* Calculate macroblock row and col. */

    row = addr / vid_stream->mb_width;
    col = addr % vid_stream->mb_width;

    /* Calculate upper left pixel row,col for luminance plane. */

    row <<= 4; col <<= 4;

    /* For each row in macroblock luminance plane... */
    /* Copy pixel values from last I or P picture. */

    dest = vid_stream->current->luminance + ((row*row_size)+col);
    src = vid_stream->future->luminance + ((row*row_size)+col);

    rr = 16; row_size -= rr;

    do {
      *((long *)dest)++ = *((long *)src)++;
      *((long *)dest)++ = *((long *)src)++;
      *((long *)dest)++ = *((long *)src)++;
      *((long *)dest)++ = *((long *)src)++;

      dest += row_size; src += row_size;
    }
    while (--rr > 0);

    rr = 16; row_size += rr;

    if (vid_stream->color == 0) continue;

    /* Divide row,col to get upper left pixel of macroblock in Cr
       and Cb planes.
       */

    row >>= 1; col >>= 1; row_size >>= 1;

    /* For each row in Cr, and Cb planes... */

    dest = vid_stream->current->Cr + ((row*row_size)+col);
    src = vid_stream->future->Cr + ((row*row_size)+col);
    dest1 = vid_stream->current->Cb + ((row*row_size)+col);
    src1 = vid_stream->future->Cb + ((row*row_size)+col);

    row_size -= 8; rr = 8;

    do {

      /* Copy pixel values from last I or P picture. */

      *((long *)dest)++ = *((long *)src)++;
      *((long *)dest)++ = *((long *)src)++;

      *((long *)dest1)++ = *((long *)src1)++;
      *((long *)dest1)++ = *((long *)src1)++;

      dest += row_size; src += row_size;
      dest1 += row_size; src1 += row_size;
    }
    while (--rr > 0);

    row_size += 8; row_size <<= 1;
  }

  vid_stream->mblock.recon_right_for = 0;
  vid_stream->mblock.recon_down_for = 0;
}


/*
 *--------------------------------------------------------------
 *
 * ProcessSkippedBFrameMBlocks --
 *
 *	Processes skipped macroblocks in B frames.
 *
 * Results:
 *	Calculates pixel values for luminance, Cr, and Cb planes
 *      in current pict image for skipped macroblocks.
 *
 * Side effects:
 *	Pixel values in pict image changed.
 *
 *--------------------------------------------------------------
 */

static void ProcessSkippedBFrameMBlocks( VidStream *vid_stream )
{
  long row_size, row, col, addr, rr;
  long right_half_for, down_half_for;
  long right_half_back, down_half_back;
  long right_for, down_for, right_back, down_back;
  unsigned char *dest, *src1, *src2, *src1a, *src2a;

  /* Calculate row sizes for luminance and Cr/Cb macroblock areas. */

  row_size = vid_stream->mb_width << 4;

  /* Construct motion vectors. */

  if (vid_stream->mblock.bpict_past_forw) {
    right_for = vid_stream->mblock.recon_right_for;
    down_for = vid_stream->mblock.recon_down_for;
    right_half_for = right_for & 1;
    down_half_for = down_for & 1;
    right_for >>= 1;
    down_for >>= 1;
  }
  if (vid_stream->mblock.bpict_past_back) {
    right_back = vid_stream->mblock.recon_right_back;
    down_back = vid_stream->mblock.recon_down_back;
    right_half_back = right_back & 1;
    down_half_back = down_back & 1;
    right_back >>= 1;
    down_back >>= 1;
  }

  /* For each skipped macroblock, do... */

  for (addr = vid_stream->mblock.past_mb_addr;
       ++addr < vid_stream->mblock.mb_address;) {

    /* Calculate macroblock row and col. */

    row = addr / vid_stream->mb_width;
    col = addr % vid_stream->mb_width;

    /* Calculate upper left pixel row,col for luminance plane. */

    row <<= 4; col <<= 4;

    if (vid_stream->mblock.bpict_past_forw &&
	vid_stream->mblock.bpict_past_back == 0) {

      dest = vid_stream->current->luminance + ((row*row_size)+col);
      src1 = vid_stream->past->luminance + ((row*row_size)+col);
      src1 += right_for; src1 += down_for*row_size;

      src2 = src1; src2 += right_half_for;
      if (down_half_for) src2 += row_size;
      rr = 16; row_size -= rr;

      if (src2 == src1)
	do {
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;

	  dest += row_size;
	  src1 += row_size;
	}
	while (--rr > 0);
      else
	do {
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;

	  dest += row_size;
	  src1 += row_size;
	  src2 += row_size;
	}
	while (--rr > 0);
      rr = 16; row_size += rr;

      if (vid_stream->color) {

	row >>= 1; col >>= 1; row_size >>= 1;

	dest = vid_stream->current->Cr + ((row*row_size)+col);
	src1 = vid_stream->past->Cr + ((row*row_size)+col);
	src1 += right_for >> 1; src1 += (down_for >> 1)*row_size;

	src2 = src1; src2 += right_for & 1;
	if (down_for & 1) src2 += row_size;
	row_size -= 8; rr = 8;

	if (src2 == src1)
	  do {
	    *dest++ = *src1++;
	    *dest++ = *src1++;
	    *dest++ = *src1++;
	    *dest++ = *src1++;
	    *dest++ = *src1++;
	    *dest++ = *src1++;
	    *dest++ = *src1++;
	    *dest++ = *src1++;

	    dest += row_size;
	    src1 += row_size;
	  }
	  while (--rr > 0);
	else
	  do {
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;

	    dest += row_size;
	    src1 += row_size;
	    src2 += row_size;
	  }
	  while (--rr > 0);
	row_size += 8;

	dest = vid_stream->current->Cb + ((row*row_size)+col);
	src1 = vid_stream->past->Cb + ((row*row_size)+col);
	src1 += right_for >> 1; src1 += (down_for >> 1)*row_size;

	src2 = src1; src2 += right_for & 1;
	if (down_for & 1) src2 += row_size;

	goto tail1;
      }
    }

    else if (vid_stream->mblock.bpict_past_back &&
	     vid_stream->mblock.bpict_past_forw == 0) {

      dest = vid_stream->current->luminance + ((row*row_size)+col);
      src1 = vid_stream->future->luminance + ((row*row_size)+col);
      src1 += right_back; src1 += down_back*row_size;

      src2 = src1; src2 += right_half_back;
      if (down_half_back) src2 += row_size;
      rr = 16; row_size -= rr;

      if (src2 == src1)
	do {
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;

	  dest += row_size;
	  src1 += row_size;
	}
	while (--rr > 0);
      else
	do {
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;

	  dest += row_size;
	  src1 += row_size;
	  src2 += row_size;
	}
	while (--rr > 0);
      rr = 16; row_size += rr;

      if (vid_stream->color) {

	row >>= 1; col >>= 1; row_size >>= 1;

	dest = vid_stream->current->Cr + ((row*row_size)+col);
	src1 = vid_stream->future->Cr + ((row*row_size)+col);
	src1 += right_back >> 1; src1 += (down_back >> 1)*row_size;

	src2 = src1; src2 += right_back & 1;
	if (down_back & 1) src2 += row_size;
	row_size -= 8; rr = 8;

	if (src2 == src1)
	  do {
	    *dest++ = *src1++;
	    *dest++ = *src1++;
	    *dest++ = *src1++;
	    *dest++ = *src1++;
	    *dest++ = *src1++;
	    *dest++ = *src1++;
	    *dest++ = *src1++;
	    *dest++ = *src1++;

	    dest += row_size;
	    src1 += row_size;
	  }
	  while (--rr > 0);
	else
	  do {
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;

	    dest += row_size;
	    src1 += row_size;
	    src2 += row_size;
	  }
	  while (--rr > 0);
	row_size += 8;

	dest = vid_stream->current->Cb + ((row*row_size)+col);
	src1 = vid_stream->future->Cb + ((row*row_size)+col);
	src1 += right_back >> 1; src1 += (down_back >> 1)*row_size;

	src2 = src1; src2 += right_back & 1;
	if (down_back & 1) src2 += row_size;
	tail1: row_size -= 8; rr = 8;

	if (src2 != src1) goto tail2;
	do {
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;
	  *dest++ = *src1++;

	  dest += row_size;
	  src1 += row_size;
	}
	while (--rr > 0);
	row_size += 8; row_size <<= 1;
      }
    }
    else {

      dest = vid_stream->current->luminance + ((row*row_size)+col);
      src1 = vid_stream->past->luminance + ((row*row_size)+col);
      src2 = vid_stream->future->luminance + ((row*row_size)+col);
      src1 += right_for; src1 += down_for*row_size;
      src2 += right_back; src2 += down_back*row_size;

      src1a = src1;
      src2a = src2;
      src1a += right_half_for; if (down_half_for) src1a += row_size;
      src2a += right_half_back; if (down_half_back) src2a += row_size;
      rr = 16; row_size -= rr;

      if (src1a != src1 || src2a != src2)
	do {
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;
	  *dest++ = ((int)*src1++ + (int)*src1a++ +
		     (int)*src2++ + (int)*src2a++) >> 2;

	  dest += row_size;
	  src1 += row_size;
	  src1a += row_size;
	  src2 += row_size;
	  src2a += row_size;
	}
	while (--rr > 0);
      else
	do {
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	  *dest++ = ((int)*src1++ + (int)*src2++) >> 1;

	  dest += row_size;
	  src1 += row_size;
	  src2 += row_size;
	}
	while (--rr > 0);
      rr = 16; row_size += rr;

      if (vid_stream->color) {

	row >>= 1; col >>= 1; row_size >>= 1;

	dest = vid_stream->current->Cr + ((row*row_size)+col);
	src1 = vid_stream->past->Cr + ((row*row_size)+col);
	src2 = vid_stream->future->Cr + ((row*row_size)+col);
	src1 += right_for >> 1; src1 += (down_for >> 1)*row_size;
	src2 += right_back >> 1; src2 += (down_back >> 1)*row_size;

	src1a = src1;
	src2a = src2;
	src1a += right_for & 1; if (down_for & 1) src1a += row_size;
	src2a += right_back & 1; if (down_back & 1) src2a += row_size;
	row_size -= 8; rr = 8;

	if (src1a != src1 || src2a != src2)
	  do {
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;

	    dest += row_size;
	    src1 += row_size;
	    src1a += row_size;
	    src2 += row_size;
	    src2a += row_size;
	  }
	  while (--rr > 0);
	else
	  do {
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;

	    dest += row_size;
	    src1 += row_size;
	    src2 += row_size;
	  }
	  while (--rr > 0);
	row_size += 8;

	dest = vid_stream->current->Cb + ((row*row_size)+col);
	src1 = vid_stream->past->Cb + ((row*row_size)+col);
	src2 = vid_stream->future->Cb + ((row*row_size)+col);
	src1 += right_for >> 1; src1 += (down_for >> 1)*row_size;
	src2 += right_back >> 1; src2 += (down_back >> 1)*row_size;

	src1a = src1;
	src2a = src2;
	src1a += right_for & 1; if (down_for & 1) src1a += row_size;
	src2a += right_back & 1; if (down_back & 1) src2a += row_size;
	row_size -= 8; rr = 8;

	if (src1a != src1 || src2a != src2)
	  do {
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;
	    *dest++ = ((int)*src1++ + (int)*src1a++ +
		       (int)*src2++ + (int)*src2a++) >> 2;

	    dest += row_size;
	    src1 += row_size;
	    src1a += row_size;
	    src2 += row_size;
	    src2a += row_size;
	  }
	  while (--rr > 0);
	else {
	  tail2:
	  do {
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;
	    *dest++ = ((int)*src1++ + (int)*src2++) >> 1;

	    dest += row_size;
	    src1 += row_size;
	    src2 += row_size;
	  }
	  while (--rr > 0);
	}
	row_size += 8; row_size <<= 1;
      }
    }
  }
}


/*
 *--------------------------------------------------------------
 *
 * ParseMacroBlocks --
 *
 *      Parseoff macroblocks. Reconstructs DCT values. Applies
 *      inverse DCT, reconstructs motion vectors, calculates and
 *      set pixel values for macroblock in current pict image
 *      structure.
 *
 * Results:
 *      Here's where everything really happens. Welcome to the
 *      heart of darkness.
 *
 * Side effects:
 *      Bit stream irreversibly parsed off.
 *
 *--------------------------------------------------------------
 */

static void ParseMacroBlocks( VidStream *vid_stream )
{
  unsigned long data;
  long motion_code;
  int i, zero_block_flag;
  BOOLEAN mb_quant, mb_motion_forw, mb_motion_back, mb_pattern;

  start:

  show_bits(23,data)
  if (data == 0) return;

  /* Parse off macroblock address increment and add to macroblock
     address.
  */

  do {
    DecodeMBAddrInc(i, data)
    if (i == MB_ESCAPE) {
      vid_stream->mblock.mb_address += 33;
      i = MB_STUFFING;
    }
  } while (i == MB_STUFFING);
  vid_stream->mblock.mb_address += i;

  i = 0;
  if ((long)vid_stream->mb_height * (long)vid_stream->mb_width
      <= vid_stream->mblock.mb_address) {
    vid_stream->mblock.mb_address =
      (long)vid_stream->mb_height * (long)vid_stream->mb_width;
    i = 1;
  }

  /* If macroblocks have been skipped, process skipped macroblocks. */

  if (vid_stream->mblock.mb_address -
      vid_stream->mblock.past_mb_addr - 1 > 0) {
    switch (vid_stream->picture.code_type) {
    case P_TYPE:
      ProcessSkippedPFrameMBlocks(vid_stream); break;
    case B_TYPE:
      ProcessSkippedBFrameMBlocks(vid_stream); break;
    }

    /* Reset past dct dc y, cr, and cb values. */

    vid_stream->block.dct_dc_y_past = 0;
    vid_stream->block.dct_dc_cr_past = 0;
    vid_stream->block.dct_dc_cb_past = 0;
  }

  if (i) return;

  /* Set past macroblock address to current macroblock address. */

  vid_stream->mblock.past_mb_addr = vid_stream->mblock.mb_address;

  /* Based on picture type decode macroblock type. */

  switch (vid_stream->picture.code_type) {
  case I_TYPE:
    DecodeMBTypeI(data, mb_quant, mb_motion_forw, mb_motion_back,
		  mb_pattern, vid_stream->mblock.mb_intra)
    break;

  case P_TYPE:
    DecodeMBTypeP(data, mb_quant, mb_motion_forw, mb_motion_back,
		  mb_pattern, vid_stream->mblock.mb_intra)
    break;

  case B_TYPE:
    DecodeMBTypeB(data, mb_quant, mb_motion_forw, mb_motion_back,
		  mb_pattern, vid_stream->mblock.mb_intra)
    break;
  }

  /* If quantization flag set, parse off new quantization scale. */

  if (mb_quant == TRUE) {
    get_bits(5,data)
    vid_stream->slice.quant_scale = (int) data;
  }

  /* If forward motion vectors exist... */

  if (mb_motion_forw == TRUE) {

    /* Parse off and decode horizontal forward motion vector. */
    DecodeMotionVectors((int)motion_code, data)

    if ((motion_code = (short) motion_code) != 0) {

      /* If horiz. forward r data exists, parse off. */
      data = 0;
      if (vid_stream->picture.forw_r_size)
	get_bits(vid_stream->picture.forw_r_size, data)

      ComputeVector(&vid_stream->mblock.recon_right_for,
		    vid_stream->picture.forw_r_size,
		    vid_stream->picture.full_pel_forw_vector,
		    motion_code, data);
    }
    /* Parse off and decode vertical forward motion vector. */
    DecodeMotionVectors((int)motion_code, data)

    if ((motion_code = (short) motion_code) != 0) {

      /* If vert. forw. r data exists, parse off. */
      data = 0;
      if (vid_stream->picture.forw_r_size)
	get_bits(vid_stream->picture.forw_r_size, data)

      ComputeVector(&vid_stream->mblock.recon_down_for,
		    vid_stream->picture.forw_r_size,
		    vid_stream->picture.full_pel_forw_vector,
		    motion_code, data);
    }
  }

  /* If back motion vectors exist... */

  if (mb_motion_back == TRUE) {

    /* Parse off and decode horiz. back motion vector. */
    DecodeMotionVectors((int)motion_code, data)

    if ((motion_code = (short) motion_code) != 0) {

      /* If horiz. back r data exists, parse off. */
      data = 0;
      if (vid_stream->picture.back_r_size)
	get_bits(vid_stream->picture.back_r_size, data)

      ComputeVector(&vid_stream->mblock.recon_right_back,
		    vid_stream->picture.back_r_size,
		    vid_stream->picture.full_pel_back_vector,
		    motion_code, data);
    }
    /* Parse off and decode vert. back motion vector. */
    DecodeMotionVectors((int)motion_code, data)

    if ((motion_code = (short) motion_code) != 0) {

      /* If vert. back r data exists, parse off. */
      data = 0;
      if (vid_stream->picture.back_r_size)
	get_bits(vid_stream->picture.back_r_size, data)

      ComputeVector(&vid_stream->mblock.recon_down_back,
		    vid_stream->picture.back_r_size,
		    vid_stream->picture.full_pel_back_vector,
		    motion_code, data);
    }
  }

  /* If mblock pattern flag set, parse and decode CBP (code block pattern). */
  /* Otherwise, set CBP to zero. */

  i = 0; if (mb_pattern == TRUE) DecodeCBP(i, data)

  vid_stream->mblock.cbp = i;

  /* Reconstruct motion vectors depending on picture type. */

  switch (vid_stream->picture.code_type) {
  case P_TYPE:

    /* If no forw motion vectors, reset vectors to 0. */

    if (mb_motion_forw == 0) {
      vid_stream->mblock.recon_right_for = 0;
      vid_stream->mblock.recon_down_for = 0;
    }
    break;

  case B_TYPE:

    /* Reset vectors to zero if mblock is intracoded. */

    if (vid_stream->mblock.mb_intra) {
      vid_stream->mblock.recon_right_for = 0;
      vid_stream->mblock.recon_down_for = 0;
      vid_stream->mblock.recon_right_back = 0;
      vid_stream->mblock.recon_down_back = 0;
    }
    else {

      /* Store vector existance flags in structure for possible
         skipped macroblocks to follow.
      */

      vid_stream->mblock.bpict_past_forw = mb_motion_forw;
      vid_stream->mblock.bpict_past_back = mb_motion_back;
    }
    break;
  }

  /* For each possible block in macroblock. */

  for (i = 0; i < 6; i++) {

    /* If block exists... */

    if (vid_stream->mblock.mb_intra ||
        (vid_stream->mblock.cbp & (32 >> i))) {

      /* Unset Zero Block Flag. */

      zero_block_flag = 0;

      if (vid_stream->color || i < 4)

	/* Parse and reconstruct block. */

	ParseReconBlock(vid_stream, i);

	/* Parse and throw away block. */

      else ParseAwayBlock(vid_stream, i);
    }

    /* Otherwise, set zero block flag. */

    else zero_block_flag = 1;

    if (vid_stream->color || i < 4) {

      /* If macroblock is intra coded... */
      if (vid_stream->mblock.mb_intra) ReconIMBlock(vid_stream, i);
      else if (mb_motion_forw && mb_motion_back)
	ReconBiMBlock(vid_stream, i, zero_block_flag);
      else if (mb_motion_forw || (vid_stream->picture.code_type == P_TYPE))
	ReconPMBlock(vid_stream, i, zero_block_flag);
      else if (mb_motion_back)
	ReconBMBlock(vid_stream, i, zero_block_flag);
    }
  }

  /* If D Type picture, flush marker bit. */

  if (vid_stream->picture.code_type == 4) flush_bits(1)

  if (vid_stream->mblock.mb_intra == 0) {

    /* Reset past dct dc y, cr, and cb values. */

    vid_stream->block.dct_dc_y_past = 0;
    vid_stream->block.dct_dc_cr_past = 0;
    vid_stream->block.dct_dc_cb_past = 0;
  }

  goto start;
}


/*
 *--------------------------------------------------------------
 *
 * ParseSlice --
 *
 *      Parses off slice header.
 *
 * Results:
 *      Values found in slice header put into video stream structure.
 *
 * Side effects:
 *      Bit stream irreversibly parsed.
 *
 *--------------------------------------------------------------
 */

static void ParseSlice( VidStream *vid_stream, int slice )
{
  unsigned long data;

  /* Reset macroblock address. */

  vid_stream->mblock.mb_address =
    (long)slice * (long)vid_stream->mb_width - 1;

  /* Parse off quantization scale. */

  get_bits(5,data)
  vid_stream->slice.quant_scale = (int) data;

  /* Parse off extra bit slice info. */

  get_extra_bit_info(vid_stream, &vid_stream->slice.extra_info);

  /* Reset previous recon motion vectors. */

  vid_stream->mblock.recon_right_for = 0;
  vid_stream->mblock.recon_down_for = 0;
  vid_stream->mblock.recon_right_back = 0;
  vid_stream->mblock.recon_down_back = 0;

  /* Reset past dct dc y, cr, and cb values. */

  vid_stream->block.dct_dc_y_past = 0;
  vid_stream->block.dct_dc_cr_past = 0;
  vid_stream->block.dct_dc_cb_past = 0;
}


/*
 *--------------------------------------------------------------
 *
 * ParsePicture --
 *
 *      Parses picture header. Marks picture to be presented
 *      at particular time given a time stamp.
 *
 * Results:
 *      Values from picture header put into video stream structure.
 *
 * Side effects:
 *      Bit stream irreversibly parsed.
 *
 *--------------------------------------------------------------
 */

static void ParsePicture( VidStream *vid_stream, TimeStamp time_stamp )
{
  unsigned long data;
  int i;

  /* Parse off temporal reference. */
  get_bits(10,data)
  vid_stream->picture.temp_ref = (unsigned) data;

  /* Parse of picture type. */
  get_bits(3,data)
  vid_stream->picture.code_type = (unsigned) data;

  if (vid_stream->picture.code_type == B_TYPE &&
      (vid_stream->No_B_Flag ||
       vid_stream->past == 0 || vid_stream->future == 0) ||
      vid_stream->picture.code_type == P_TYPE &&
      (vid_stream->No_P_Flag || vid_stream->future == 0)) {
    (*vid_stream->put_message)("\nSkipping picture...");
    for (;;) {
      data = next_start_code(vid_stream);
      if ((int) data == SEQ_START_CODE) break;
      if ((int) data == GOP_START_CODE) break;
      if ((int) data == PICTURE_START_CODE) break;
      if ((int) data == SEQ_END_CODE) break;
      flush_bits32
    }
    (*vid_stream->put_message)("Done.\n"); return;
  }

  /* Parse off vbv buffer delay value. */
  get_bits(16,data)
  vid_stream->picture.vbv_delay = (unsigned) data;

  /* If P or B type frame... */

  switch (vid_stream->picture.code_type) {
  case P_TYPE:
  case B_TYPE:

    /* Parse off forward vector full pixel flag. */
    get_bits1(data)
    vid_stream->picture.full_pel_forw_vector = (BOOLEAN) data;

    /* Parse of forw_r_code. */
    get_bits(3,data)

    /* Decode forw_r_code into forw_r_size. */

    vid_stream->picture.forw_r_size = (int) data - 1;

    /* If B type frame... */

    if (vid_stream->picture.code_type == B_TYPE) {

      /* Parse off back vector full pixel flag. */
      get_bits1(data)
      vid_stream->picture.full_pel_back_vector = (BOOLEAN) data;

      /* Parse off back_r_code. */
      get_bits(3,data)

      /* Decode back_r_code into back_r_size. */

      vid_stream->picture.back_r_size = (int) data - 1;
    }
  }

  /* Find a pict image structure in ring buffer not currently locked. */

  i = 0;

  while (vid_stream->ring[i].locked)
    if (++i >= RING_BUF_SIZE) {
      (*vid_stream->put_message)("Fatal error. Ring buffer full.");
      longjmp(setjmp_buffer, 3);
    }

  /* Set current pict image structure to the one just found in ring. */

  vid_stream->current = vid_stream->ring + i;

  /* Set time stamp. */

  vid_stream->current->show_time = time_stamp;

  /* Reset past macroblock address field. */

  vid_stream->mblock.past_mb_addr = -1;

  /* Get extra bit picture info. */

  get_extra_bit_info(vid_stream, &vid_stream->picture.extra_info);
}


/*
 *--------------------------------------------------------------
 *
 * ParseGOP --
 *
 *      Parses of group of pictures header from bit stream
 *      associated with vid_stream.
 *
 * Results:
 *      Values in gop header placed into video stream structure.
 *
 * Side effects:
 *      Bit stream irreversibly parsed.
 *
 *--------------------------------------------------------------
 */

static void ParseGOP( VidStream *vid_stream )
{
  unsigned long data;

  /* Parse off drop frame flag. */

  get_bits1(data)
  vid_stream->group.drop_flag = (BOOLEAN) data;

  /* Parse off hour component of time code. */

  get_bits(5,data)
  vid_stream->group.tc_hours = (int) data;

  /* Parse off minute component of time code. */

  get_bits(6,data)
  vid_stream->group.tc_minutes = (int) data;

  /* Flush marker bit. */

  flush_bits(1)

  /* Parse off second component of time code. */

  get_bits(6,data)
  vid_stream->group.tc_seconds = (int) data;

  /* Parse off picture count component of time code. */

  get_bits(6,data)
  vid_stream->group.tc_pictures = (int) data;

  /* Parse off closed gop and broken link flags. */

  get_bits1(data)
  vid_stream->group.closed_gop = (BOOLEAN) data;
  get_bits1(data)
  vid_stream->group.broken_link = (BOOLEAN) data;
}


/*
 *--------------------------------------------------------------
 *
 * ParseSeqHead --
 *
 *      Assumes bit stream is at the begining of the sequence
 *      header start code. Parses off the sequence header.
 *
 * Results:
 *      Fills the vid_stream structure with values derived and
 *      decoded from the sequence header. Allocates the pict image
 *      structures based on the dimensions of the image space
 *      found in the sequence header.
 *
 * Side effects:
 *      Bit stream irreversibly parsed off.
 *
 *--------------------------------------------------------------
 */

static void ParseSeqHead( VidStream *vid_stream )
{
  unsigned long data;
  int i;

  /* Get horizontal size of image space. */

  get_bits(12,data)
  vid_stream->h_size = (int) data;

  /* Get vertical size of image space. */

  get_bits(12,data)
  vid_stream->v_size = (int) data;

  /* Calculate macroblock width and height of image space. */

  vid_stream->mb_width = (vid_stream->h_size + 15) >> 4;
  vid_stream->mb_height = (vid_stream->v_size + 15) >> 4;

  /* Initialize ring buffer of pict images now
     that dimensions of image space are known.
  */

  for (i = 0; i < RING_BUF_SIZE; i++)
    InitPictImage(vid_stream, vid_stream->ring + i);

  if (vid_stream->y_scale == 0) mpegNewScale(vid_stream, 0);

  /* Parse of aspect ratio code. */

  get_bits(4,data)
  vid_stream->aspect_ratio = (unsigned char) data;

  /* Parse off picture rate code. */

  get_bits(4,data)
  vid_stream->picture_rate = (unsigned char) data;

  /* Parse off bit rate. */

  get_bits(18,data)
  vid_stream->bit_rate = data;

  /* Flush marker bit. */

  flush_bits(1)

  /* Parse off vbv buffer size. */

  get_bits(10,data)
  vid_stream->vbv_buffer_size = data;

  /* Parse off contrained parameter flag. */

  get_bits1(data)
  vid_stream->const_param_flag = (BOOLEAN) data;

  /* If intra_quant_matrix_flag set, parse off intra quant matrix
     values.
  */

  get_bits1(data)
  if (data)
    for (i = 0; i < 64; i++) {
      get_bits(8,data)
      vid_stream->intra_quant_matrix[i] = (unsigned char) data;
    }

  /* If non intra quant matrix flag set, parse off non intra quant matrix
     values.
  */

  get_bits1(data)
  if (data)
    for (i = 0; i < 64; i++) {
      get_bits(8,data)
      vid_stream->non_intra_quant_matrix[i] = (unsigned char) data;
    }
}


/*
 *--------------------------------------------------------------
 *
 * mpegVidRsrc --
 *
 *      Parses bit stream until current slice or picture ends.
 *	If the start of a frame is encountered, the frame is time
 *	stamped with the value passed in time_stamp. If the value
 *      passed in buffer is not null, the video stream buffer
 *      is set to buffer and the length of the buffer is
 *      expected in value passed in through length.
 *
 * Results:
 *      Status.
 *
 * Side effects:
 *      Bit stream is irreversibly parsed. If a picture is completed,
 *      a function is called to display the frame at the correct time.
 *
 *--------------------------------------------------------------
 */

int mpegVidRsrc( VidStream *vid_stream, TimeStamp time_stamp )
{
  int data;

  if ((data = setjmp( setjmp_buffer )) != 0) return data;

  if (vid_stream->init_flag == 0) {
    vid_stream->init_flag++;
    init_tables(); InitVidStream(vid_stream);

    /* If called for the first time, find start code,
       make sure it is a sequence start code.
    */

    if ((int) next_start_code(vid_stream) != SEQ_START_CODE) {
      (*vid_stream->put_message)("\nThis is not an MPEG stream.\n");
      return 2;
    }
    vid_stream->realTimeStart = clock();
  }

  for (;;) {
    /* Get next 32 bits (size of start codes). */

    data = (int) next_start_code(vid_stream);
    flush_bits32

    /* Process according to start code */

    if (data == SEQ_START_CODE) {

      /* Sequence start code. Parse sequence header. */

      ParseSeqHead(vid_stream);
    }
    else if (data == GOP_START_CODE) {

      /* Group of Pictures start code. Parse gop header. */

      ParseGOP(vid_stream);
    }
    else if (data == PICTURE_START_CODE) {

      /* Picture start code. Parse picture header. */

      ParsePicture(vid_stream, time_stamp);
    }
    else if (data == SEQ_END_CODE) {

      /* Sequence done. Do the right thing. For right now, exit. */

      (*vid_stream->put_message)("\nDone!\n");
      PrintTimeInfo(vid_stream); return 1;
    }
    else if (data >= SLICE_MIN_START_CODE &&
	     data <= SLICE_MAX_START_CODE) break;
  }

  for (;;) {
    /* Slice start code. Parse slice header. */

    ParseSlice(vid_stream, data - SLICE_MIN_START_CODE);

    /* Parse macroblocks. */

    ParseMacroBlocks(vid_stream);

    data = (int) next_start_code(vid_stream);
    if (data < SLICE_MIN_START_CODE ||
	data > SLICE_MAX_START_CODE) break;
    flush_bits32
  }

  /* If start code is outside range of slice start codes,
     frame is complete, display frame.
  */

  /* Update past and future references if needed. */

  switch (vid_stream->picture.code_type) {
  case I_TYPE:
  case P_TYPE:
    if (vid_stream->future == 0) {
      vid_stream->future = vid_stream->current;
      vid_stream->future->locked |= FUTURE_LOCK;
    } else {
      if (vid_stream->past)
	vid_stream->past->locked &= ~PAST_LOCK;
      vid_stream->past = vid_stream->future;
      vid_stream->past->locked &= ~FUTURE_LOCK;
      vid_stream->past->locked |= PAST_LOCK;
      vid_stream->future = vid_stream->current;
      vid_stream->future->locked |= FUTURE_LOCK;
      vid_stream->current = vid_stream->past;
    }
  }

  return 0;
}
