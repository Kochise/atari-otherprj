#include "video.h"

static void copy_1_16( unsigned char *src, unsigned char *des,
		       int src_cols, int src_rows, int des_cols )
{
  do {
    unsigned char *srcp = src;
    unsigned char *old = des;
    int col = des_cols;
    do {
      unsigned char c = *srcp++;
      *des++ = c; *des++ = c;
      *des++ = c; *des++ = c;
      *des++ = c; *des++ = c;
      *des++ = c; *des++ = c;
      *des++ = c; *des++ = c;
      *des++ = c; *des++ = c;
      *des++ = c; *des++ = c;
      *des++ = c; *des++ = c;
    }
    while ((col -= 16) > 0);
    src += src_cols;
    col = des_cols;
    do {
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
    }
    while ((col -= 4) > 0);
  }
  while (--src_rows > 0);
}

static void copy_1_8( unsigned char *src, unsigned char *des,
		      int src_cols, int src_rows, int des_cols )
{
  do {
    unsigned char *srcp = src;
    unsigned char *old = des;
    int col = des_cols;
    do {
      unsigned char c = *srcp++;
      *des++ = c; *des++ = c;
      *des++ = c; *des++ = c;
      *des++ = c; *des++ = c;
      *des++ = c; *des++ = c;
    }
    while ((col -= 8) > 0);
    src += src_cols;
    col = des_cols;
    do {
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
    }
    while ((col -= 4) > 0);
  }
  while (--src_rows > 0);
}

static void copy_1_4( unsigned char *src, unsigned char *des,
		      int src_cols, int src_rows, int des_cols )
{
  do {
    unsigned char *srcp = src;
    unsigned char *old = des;
    int col = des_cols;
    do {
      unsigned char c = *srcp++;
      *des++ = c; *des++ = c;
      *des++ = c; *des++ = c;
    }
    while ((col -= 4) > 0);
    src += src_cols;
    col = des_cols;
    do {
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
      *((long *)des)++ = *((long *)old)++;
    }
    while ((col -= 4) > 0);
  }
  while (--src_rows > 0);
}

static void copy_1_2( unsigned char *src, unsigned char *des1,
		      int src_cols, int src_rows, int des_cols )
{
  do {
    unsigned char *des2 = des1 + des_cols;
    unsigned char *srcp = src;
    int col = des_cols;
    do {
      unsigned char c = *srcp++;
      *des1++ = c; *des1++ = c;
      *des2++ = c; *des2++ = c;
    }
    while ((col -= 2) > 0);
    src += src_cols;
    des1 = des2;
  }
  while (--src_rows > 0);
}

#pragma warn -par
static void copy_1_1( unsigned char *src, unsigned char *des,
		      int src_cols, int src_rows, int des_cols )
{
  do {
    int col = src_cols;
    do *((long *)des)++ = *((long *)src)++;
    while ((col -= 4) > 0);
    col -= src_cols; col &= 15; des += col;
  }
  while (--src_rows > 0);
}

static void copy_2_1( unsigned char *src1, unsigned char *des,
		      int src_cols, int src_rows, int des_cols )
{
  int i = 0;
  do {
    int col = src_cols;
    unsigned char *src2 = src1 + src_cols;
    do {
      int val = 2;
      (unsigned char)i = *src1++; val += i;
      (unsigned char)i = *src1++; val += i;
      (unsigned char)i = *src2++; val += i;
      (unsigned char)i = *src2++; val += i;
      val >>= 2; *des++ = val;
    }
    while ((col -= 2) > 0);
    col -= src_cols; col >>= 1; col &= 15; des += col;
    src1 = src2;
  }
  while ((src_rows -= 2) > 0);
}

static void copy_4_1( unsigned char *src, unsigned char *des,
		      int src_cols, int src_rows, int des_cols )
{
  int i = 0;
  do {
    int col = src_cols;
    for (;;) {
      int count = 4, val = 8;
      for (;;) {
	(unsigned char)i = *src++; val += i;
	(unsigned char)i = *src++; val += i;
	(unsigned char)i = *src++; val += i;
	(unsigned char)i = *src++; val += i;
	if (--count <= 0) break;
	src -= 4; src += src_cols;
      }
      val >>= 4; *des++ = val;
      if ((col -= 4) <= 0) break;
      src -= src_cols; src -= src_cols;
      src -= src_cols;
    }
    col -= src_cols; col >>= 2; col &= 15; des += col;
  }
  while ((src_rows -= 4) > 0);
}

static void copy_8_1( unsigned char *src, unsigned char *des,
		      int src_cols, int src_rows, int des_cols )
{
  int i = 0;
  do {
    int col = src_cols;
    for (;;) {
      int count = 8, val = 32;
      for (;;) {
	(unsigned char)i = *src++; val += i;
	(unsigned char)i = *src++; val += i;
	(unsigned char)i = *src++; val += i;
	(unsigned char)i = *src++; val += i;
	(unsigned char)i = *src++; val += i;
	(unsigned char)i = *src++; val += i;
	(unsigned char)i = *src++; val += i;
	(unsigned char)i = *src++; val += i;
	if (--count <= 0) break;
	src -= 8; src += src_cols;
      }
      val >>= 6; *des++ = val;
      if ((col -= 8) <= 0) break;
      src -= src_cols; src -= src_cols;
      src -= src_cols; src -= src_cols;
      src -= src_cols; src -= src_cols;
      src -= src_cols;
    }
    col -= src_cols; col >>= 3; col &= 15; des += col;
  }
  while ((src_rows -= 8) > 0);
}
#pragma warn +par

void mpegNewScale( VidStream *vid_stream, int new_scale )
{
  int dx, dy;
  long totpix;
  void (*y_scale)( unsigned char *src, unsigned char *des,
		   int src_cols, int src_rows, int des_cols );
  void (*c_scale)( unsigned char *src, unsigned char *des,
		   int src_cols, int src_rows, int des_cols );

  start:
  if (vid_stream->color == 0) {
    if (new_scale)
      totpix = ((long)vid_stream->mb_width *
		(long)vid_stream->mb_height) << 8;
    if (vid_stream->red) {
      (*vid_stream->free)(vid_stream, vid_stream->red);
      vid_stream->red = 0;
    }
  }
  else if (vid_stream->scale > 0 || new_scale > 0) {
    totpix = ((long)vid_stream->mb_width *
	      (long)vid_stream->mb_height) << 8;
    if (vid_stream->red != vid_stream->old_ptr) {
      (*vid_stream->free)(vid_stream, vid_stream->red);
      vid_stream->red = vid_stream->old_ptr;
      vid_stream->green = vid_stream->red + totpix;
      vid_stream->blue = vid_stream->green + totpix;
    }
  }
  dx = vid_stream->h_size; dy = vid_stream->v_size;
  switch (new_scale) {
  case -3:
    dx += 7; dx >>= 3; dy += 7; dy >>= 3;
    y_scale = copy_8_1;
    c_scale = copy_4_1; break;
  case -2:
    dx += 3; dx >>= 2; dy += 3; dy >>= 2;
    y_scale = copy_4_1;
    c_scale = copy_2_1; break;
  case -1:
    dx++; dx >>= 1; dy++; dy >>= 1;
    y_scale = copy_2_1;
    c_scale = copy_1_1; break;
  case  0:
    y_scale = copy_1_1;
    c_scale = copy_1_2; break;
  case  1:
    totpix <<= 2; dx <<= 1; dy <<= 1;
    y_scale = copy_1_2;
    c_scale = copy_1_4; break;
  case  2:
    totpix <<= 4; dx <<= 2; dy <<= 2;
    y_scale = copy_1_4;
    c_scale = copy_1_8; break;
  case  3:
    totpix <<= 6; dx <<= 3; dy <<= 3;
    y_scale = copy_1_8;
    c_scale = copy_1_16; break;
  default: goto skip;
  }
  vid_stream->y_scale = y_scale;
  vid_stream->c_scale = c_scale;
  skip:
  if (vid_stream->color == 0) {
    if (new_scale)
      if ((vid_stream->red =
	   (*vid_stream->alloc)(vid_stream, totpix)) == 0) {
	if (--new_scale < 0) new_scale = 0;
	goto start;
      }
  }
  else if (new_scale > 0) {
    unsigned char *p;
    p = (*vid_stream->alloc)(vid_stream, totpix * 3);
    if (p == 0) { new_scale--; goto start; }
    vid_stream->red = p;
    vid_stream->green = vid_stream->red + totpix;
    vid_stream->blue = vid_stream->green + totpix;
  }
  vid_stream->dest_width = dx;
  vid_stream->dest_height = dy;
  vid_stream->scale = new_scale;
}
