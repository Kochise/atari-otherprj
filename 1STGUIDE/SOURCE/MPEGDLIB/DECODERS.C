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
/*
 * decoders.c
 *
 * This file contains all the routines for Huffman decoding required in
 * MPEG
 *
 */

#include "video.h"
#include "decoders.h"

/* Decoding table for macroblock_address_increment */
mb_addr_inc_entry mb_addr_inc[2048];

/* Decoding table for macroblock_type in predictive-coded pictures */
mb_type_entry mb_type_P[64];

/* Decoding table for macroblock_type in bidirectionally-coded pictures */
mb_type_entry mb_type_B[64];

/* Decoding table for motion vectors */
motion_vectors_entry motion_vectors[2048];

/* Decoding table for coded_block_pattern */

const int coded_block_pattern[512] =
{   0xFFF0, 0xFFF0, 0x0279, 0x01B9, 0x03B9, 0x0379, 0x02F9, 0x01F9,
    0x03A8, 0x03A8, 0x0368, 0x0368, 0x02E8, 0x02E8, 0x01E8, 0x01E8,
    0x0398, 0x0398, 0x0358, 0x0358, 0x02D8, 0x02D8, 0x01D8, 0x01D8,
    0x0268, 0x0268, 0x01A8, 0x01A8, 0x0258, 0x0258, 0x0198, 0x0198,
    0x02B8, 0x02B8, 0x0178, 0x0178, 0x0338, 0x0338, 0x00F8, 0x00F8,
    0x02A8, 0x02A8, 0x0168, 0x0168, 0x0328, 0x0328, 0x00E8, 0x00E8,
    0x0298, 0x0298, 0x0158, 0x0158, 0x0318, 0x0318, 0x00D8, 0x00D8,
    0x0238, 0x0238, 0x0138, 0x0138, 0x00B8, 0x00B8, 0x0078, 0x0078,
    0x0227, 0x0227, 0x0227, 0x0227, 0x0127, 0x0127, 0x0127, 0x0127,
    0x00A7, 0x00A7, 0x00A7, 0x00A7, 0x0067, 0x0067, 0x0067, 0x0067,
    0x0217, 0x0217, 0x0217, 0x0217, 0x0117, 0x0117, 0x0117, 0x0117,
    0x0097, 0x0097, 0x0097, 0x0097, 0x0057, 0x0057, 0x0057, 0x0057,
    0x03F6, 0x03F6, 0x03F6, 0x03F6, 0x03F6, 0x03F6, 0x03F6, 0x03F6,
    0x0036, 0x0036, 0x0036, 0x0036, 0x0036, 0x0036, 0x0036, 0x0036,
    0x0246, 0x0246, 0x0246, 0x0246, 0x0246, 0x0246, 0x0246, 0x0246,
    0x0186, 0x0186, 0x0186, 0x0186, 0x0186, 0x0186, 0x0186, 0x0186,
    0x03E5, 0x03E5, 0x03E5, 0x03E5, 0x03E5, 0x03E5, 0x03E5, 0x03E5,
    0x03E5, 0x03E5, 0x03E5, 0x03E5, 0x03E5, 0x03E5, 0x03E5, 0x03E5,
    0x0025, 0x0025, 0x0025, 0x0025, 0x0025, 0x0025, 0x0025, 0x0025,
    0x0025, 0x0025, 0x0025, 0x0025, 0x0025, 0x0025, 0x0025, 0x0025,
    0x03D5, 0x03D5, 0x03D5, 0x03D5, 0x03D5, 0x03D5, 0x03D5, 0x03D5,
    0x03D5, 0x03D5, 0x03D5, 0x03D5, 0x03D5, 0x03D5, 0x03D5, 0x03D5,
    0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015,
    0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015,
    0x0385, 0x0385, 0x0385, 0x0385, 0x0385, 0x0385, 0x0385, 0x0385,
    0x0385, 0x0385, 0x0385, 0x0385, 0x0385, 0x0385, 0x0385, 0x0385,
    0x0345, 0x0345, 0x0345, 0x0345, 0x0345, 0x0345, 0x0345, 0x0345,
    0x0345, 0x0345, 0x0345, 0x0345, 0x0345, 0x0345, 0x0345, 0x0345,
    0x02C5, 0x02C5, 0x02C5, 0x02C5, 0x02C5, 0x02C5, 0x02C5, 0x02C5,
    0x02C5, 0x02C5, 0x02C5, 0x02C5, 0x02C5, 0x02C5, 0x02C5, 0x02C5,
    0x01C5, 0x01C5, 0x01C5, 0x01C5, 0x01C5, 0x01C5, 0x01C5, 0x01C5,
    0x01C5, 0x01C5, 0x01C5, 0x01C5, 0x01C5, 0x01C5, 0x01C5, 0x01C5,
    0x0285, 0x0285, 0x0285, 0x0285, 0x0285, 0x0285, 0x0285, 0x0285,
    0x0285, 0x0285, 0x0285, 0x0285, 0x0285, 0x0285, 0x0285, 0x0285,
    0x0145, 0x0145, 0x0145, 0x0145, 0x0145, 0x0145, 0x0145, 0x0145,
    0x0145, 0x0145, 0x0145, 0x0145, 0x0145, 0x0145, 0x0145, 0x0145,
    0x0305, 0x0305, 0x0305, 0x0305, 0x0305, 0x0305, 0x0305, 0x0305,
    0x0305, 0x0305, 0x0305, 0x0305, 0x0305, 0x0305, 0x0305, 0x0305,
    0x00C5, 0x00C5, 0x00C5, 0x00C5, 0x00C5, 0x00C5, 0x00C5, 0x00C5,
    0x00C5, 0x00C5, 0x00C5, 0x00C5, 0x00C5, 0x00C5, 0x00C5, 0x00C5,
    0x0204, 0x0204, 0x0204, 0x0204, 0x0204, 0x0204, 0x0204, 0x0204,
    0x0204, 0x0204, 0x0204, 0x0204, 0x0204, 0x0204, 0x0204, 0x0204,
    0x0204, 0x0204, 0x0204, 0x0204, 0x0204, 0x0204, 0x0204, 0x0204,
    0x0204, 0x0204, 0x0204, 0x0204, 0x0204, 0x0204, 0x0204, 0x0204,
    0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104,
    0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104,
    0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104,
    0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104,
    0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084,
    0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084,
    0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084,
    0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084,
    0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044,
    0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044,
    0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044,
    0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044,
    0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3,
    0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3,
    0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3,
    0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3,
    0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3,
    0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3,
    0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3,
    0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3, 0x03C3
};

/* Macro for filling up the decoding table for mb_addr_inc */
#define ASSIGN1(end, step, num) \
  for (; i < end; val--) { \
    for (j = i + step; i < j; i++) { \
      mb_addr_inc[i].value = val; \
      mb_addr_inc[i].num_bits = num; \
    } \
  }



/*
 *--------------------------------------------------------------
 *
 * init_mb_addr_inc --
 *
 *	Initialize the VLC decoding table for macro_block_address_increment
 *
 * Results:
 *	The decoding table for macro_block_address_increment will
 *      be filled; illegal values will be filled as ERROR.
 *
 * Side effects:
 *	The global array mb_addr_inc will be filled.
 *
 *--------------------------------------------------------------
 */
static void init_mb_addr_inc( void )
{
  int i, j, val = ERROR;

  for (i = 0; i < 8; i++) {
    mb_addr_inc[i].value = val;
    mb_addr_inc[i].num_bits = 0;
  }

  mb_addr_inc[8].value = MACRO_BLOCK_ESCAPE;
  mb_addr_inc[8].num_bits = 11;

  for (i = 9; i < 15; i++) {
    mb_addr_inc[i].value = val;
    mb_addr_inc[i].num_bits = 0;
  }

  mb_addr_inc[15].value = MACRO_BLOCK_STUFFING;
  mb_addr_inc[15].num_bits = 11;

  for (i = 16; i < 24; i++) {
    mb_addr_inc[i].value = val;
    mb_addr_inc[i].num_bits = 0;
  }

  val = 33;

  ASSIGN1(36, 1, 11);
  ASSIGN1(48, 2, 10);
  ASSIGN1(96, 8, 8);
  ASSIGN1(128, 16, 7);
  ASSIGN1(256, 64, 5);
  ASSIGN1(512, 128, 4);
  ASSIGN1(1024, 256, 3);
  ASSIGN1(2048, 1024, 1);
}


/* Macro for filling up the decoding table for mb_type */
#define ASSIGN2(end, quant, motion_forward, motion_backward, pattern, intra, num, mb_type) \
  for (; i < end; i++) { \
    mb_type[i].mb_quant = quant; \
    mb_type[i].mb_motion_forward = motion_forward; \
    mb_type[i].mb_motion_backward = motion_backward; \
    mb_type[i].mb_pattern = pattern; \
    mb_type[i].mb_intra = intra; \
    mb_type[i].num_bits = num; \
  }
	


/*
 *--------------------------------------------------------------
 *
 * init_mb_type_P --
 *
 *	Initialize the VLC decoding table for macro_block_type in
 *      predictive-coded pictures.
 *
 * Results:
 *	The decoding table for macro_block_type in predictive-coded
 *      pictures will be filled; illegal values will be filled as ERROR.
 *
 * Side effects:
 *	The global array mb_type_P will be filled.
 *
 *--------------------------------------------------------------
 */
static void init_mb_type_P( void )
{
  int i, l;

  mb_type_P[0].mb_quant = mb_type_P[0].mb_motion_forward
    = mb_type_P[0].mb_motion_backward = mb_type_P[0].mb_pattern
      = mb_type_P[0].mb_intra = ERROR;
  mb_type_P[0].num_bits = 0;

  i = l = 1;

  ASSIGN2(2, l, 0, 0, 0, l, 6, mb_type_P)
  ASSIGN2(4, l, 0, 0, l, 0, 5, mb_type_P)
  ASSIGN2(6, l, l, 0, l, 0, 5, mb_type_P);
  ASSIGN2(8, 0, 0, 0, 0, l, 5, mb_type_P);
  ASSIGN2(16, 0, l, 0, 0, 0, 3, mb_type_P);
  ASSIGN2(32, 0, 0, 0, l, 0, 2, mb_type_P);
  ASSIGN2(64, 0, l, 0, l, 0, 1, mb_type_P);
}


/*
 *--------------------------------------------------------------
 *
 * init_mb_type_B --
 *
 *	Initialize the VLC decoding table for macro_block_type in
 *      bidirectionally-coded pictures.
 *
 * Results:
 *	The decoding table for macro_block_type in bidirectionally-coded
 *      pictures will be filled; illegal values will be filled as ERROR.
 *
 * Side effects:
 *	The global array mb_type_B will be filled.
 *
 *--------------------------------------------------------------
 */
static void init_mb_type_B( void )
{
  int i, l;

  mb_type_B[0].mb_quant = mb_type_B[0].mb_motion_forward
    = mb_type_B[0].mb_motion_backward = mb_type_B[0].mb_pattern
      = mb_type_B[0].mb_intra = ERROR;
  mb_type_B[0].num_bits = 0;

  i = l = 1;

  ASSIGN2(2, l, 0, 0, 0, l, 6, mb_type_B);
  ASSIGN2(3, l, 0, l, l, 0, 6, mb_type_B);
  ASSIGN2(4, l, l, 0, l, 0, 6, mb_type_B);
  ASSIGN2(6, l, l, l, l, 0, 5, mb_type_B);
  ASSIGN2(8, 0, 0, 0, 0, l, 5, mb_type_B);
  ASSIGN2(12, 0, l, 0, 0, 0, 4, mb_type_B);
  ASSIGN2(16, 0, l, 0, l, 0, 4, mb_type_B);
  ASSIGN2(24, 0, 0, l, 0, 0, 3, mb_type_B);
  ASSIGN2(32, 0, 0, l, l, 0, 3, mb_type_B);
  ASSIGN2(48, 0, l, l, 0, 0, 2, mb_type_B);
  ASSIGN2(64, 0, l, l, l, 0, 2, mb_type_B);
}


/* Macro for filling up the decoding tables for motion_vectors */
#define ASSIGN3(end, step, num) \
  for (; i < end; val--) { \
    for (j = i + (step >> 1); i < j; i++) { \
      motion_vectors[i].code = val; \
      motion_vectors[i].num_bits = num; \
    } \
    val = -val; \
    for (j = i + (step >> 1); i < j; i++) { \
      motion_vectors[i].code = val; \
      motion_vectors[i].num_bits = num; \
    } \
    val = -val; \
  }



/*
 *--------------------------------------------------------------
 *
 * init_motion_vectors --
 *
 *	Initialize the VLC decoding table for the various motion
 *      vectors, including motion_horizontal_forward_code,
 *      motion_vertical_forward_code, motion_horizontal_backward_code,
 *      and motion_vertical_backward_code.
 *
 * Results:
 *	The decoding table for the motion vectors will be filled;
 *      illegal values will be filled as ERROR.
 *
 * Side effects:
 *	The global array motion_vector will be filled.
 *
 *--------------------------------------------------------------
 */
static void init_motion_vectors( void )
{
  int i, j, val = ERROR;

  for (i = 0; i < 24; i++) {
    motion_vectors[i].code = val;
    motion_vectors[i].num_bits = 0;
  }

  val = 16;

  ASSIGN3(36, 2, 11);
  ASSIGN3(48, 4, 10);
  ASSIGN3(96, 16, 8);
  ASSIGN3(128, 32, 7);
  ASSIGN3(256, 128, 5);
  ASSIGN3(512, 256, 4);
  ASSIGN3(1024, 512, 3);
  ASSIGN3(2048, 1024, 1);
}


/*
 *--------------------------------------------------------------
 *
 * init_tables --
 *
 *	Initialize all the tables for VLC decoding; this must be
 *      called when the system is set up before any decoding can
 *      take place.
 *
 * Results:
 *	All the decoding tables will be filled accordingly.
 *
 * Side effects:
 *	The corresponding global array for each decoding table
 *      will be filled.
 *
 *--------------------------------------------------------------
 */
void init_tables( void )
{
  static int init_flag = 0;

  if (init_flag) return; init_flag++;
  init_mb_addr_inc();
  init_mb_type_P();
  init_mb_type_B();
  init_motion_vectors();
}
