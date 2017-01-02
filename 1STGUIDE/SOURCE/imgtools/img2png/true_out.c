/* File: TRUE_OUT.C, preliminary version of Nov 16 1995
 * Developed by Guido Vollbeding <guivol@jpegclub.org>
 * Slightly modified on 21 Aug 2007 to avoid compiler warning
 */

#include "imgcodec.h"

/* Local extension of public image buffer struct for TrueColor output. */

typedef struct ibuf
{
  IBUFPUB pub;
  char *line_adr;
  long length;
  long byte_width;
  short num_cols;
  short lines_left;
  short des_comps;		/* before color conversion */
  short out_comps;		/* after color conversion */
  short bits[4];
  unsigned char *src_end[4];
  unsigned char *des_end;	/* before color conversion */
  unsigned char *out_end;	/* after color conversion */
  TPUTPUB *true_put;
  void (*color_convert)(struct ibuf *ip);
  void (*user_exit)(void);
  long r_y_tab[256];		/* RGB => Y conversion tables */
  long g_y_tab[256];
  long b_y_tab[256];
  char data[0];
}
IBUF;

/* Forward declarations of local functions. */

static void put_true(IBUF *ip);

static void colorout_init(IBUF *ip);

/* Initialization function to be used for decoding timg input data
 * into conventional TrueColor representation, which then can be
 * further processed by standard dithering/rendering algorithms.
 *
 * Returns a zero pointer if user_malloc failes.
 * Otherwise the returned pointer has to be freed after
 * processing by the 'complement' of user_malloc.
 *
 * This routine must be called *after* checking the timg header,
 * with valid entries in it that satisfy the sanity constraints
 * of the timg header definition, i.e.:
 *   th->magic = 'TIMG'
 *   th->comps = 1 or 3
 *   th->red >= 0, th->green >= 0, th->blue >= 0
 *   th->red + th->green + th->blue <= th->ih.planes
 * For easier handling, the th->green and th->blue entries must
 * be set to 0 for th->comps = 1, thus the latter conditions can
 * be used in this case, too.
 *
 * By passing the argument 'out_comps', the application can choose
 * an appropriate TrueColor output format. Possible values are 1,
 * 2, 3, or 4 (see TPUTPUB definition). The output format can be
 * choosen *independently* from the input format, this module will
 * perform appropriate color conversions if necessary!
 *
 * Thus, if you want, for instance, display the image in monochrome
 * or grayscale, you simply need to choose the desired output format
 * (1 or 2), and the module will convert RGB input to Y (Gray)
 * automatically. Otherwise, if the input is Gray and you choose RGB
 * output, the data is expanded appropriately (R = G = B = Y).
 *
 * Similar, the routines will automatically add or discard Alpha data
 * if requested for output. The application does not need to take care
 * of the input format! (If the input does not provide Alpha, the Alpha
 * output values will be 0.)
 *
 * It should be noted, that the provided implementation handles *all*
 * input bit combinations within the given constraints, which are far
 * more general than in any other image format I know!
 * Component precisions greater than 8 bit are truncated to 8 bit
 * precision (by discarding the lower bitplanes). Precisions less than
 * 8 bit are upscaled to the 0-255 output range with reliable formulas.
 *
 * Thus, for instance, nothing would prevent you from storing image
 * data as TIMG with 53 bit red, 87 bit green, and 39 bit blue
 * precision - using this implementation you will get an approximated
 * standard output with 8 bit precision per component.
 * Somewhat more useful at the moment, however, is to store HighColor
 * data (screen snapshots) with reduced precision exactly with device-
 * specific parameters (5-5-5, 5-6-5 or whatever, with or without mask
 * bits).
 *
 * All TIMG-compliant decoders are required to handle *all* valid bit
 * combinations, otherwise they must not proclaim to be TIMG-compliant!
 * This implementation should help programmers to satisfy this demand
 * with minimal effort.
 *
 * Note further: A bit count may also be zero, forcing the output of
 * 0 values for this component. This would allow to store separated
 * images of primary colors without wasting space for unused comps
 * (this is, by the way, similar to the alpha handling). 
 */

IBUFPUB *true_out_init(TIMG_HEADER *th, TPUTPUB *true_put,
		       void *(*user_malloc)(long size),
		       void (*user_exit)(void),
		       short out_comps)
{
  long byte_width, line_len, buf_len;
  short des_comps;
  IBUF *image;

  byte_width = (th->ih.sl_width + 7) >> 3;
  line_len = byte_width * th->ih.planes;
  des_comps = th->comps;
  buf_len = th->ih.sl_width;
  switch (out_comps)
  {
    case 2: des_comps++; /* add alpha, fall through! */
    case 1: buf_len *= des_comps; break;
    case 4: des_comps++; /* add alpha, fall through! */
    case 3: buf_len *= out_comps; break;
  }
  buf_len += line_len;
  image = (*user_malloc)(sizeof(IBUF) + buf_len + th->ih.pat_run);
  if (image)
  {
    image->line_adr = image->pub.pbuf = image->data;
    image->pub.pat_buf = image->data + buf_len;
    image->pub.bytes_left = line_len;
    image->length = line_len;
    image->byte_width = byte_width;
    image->pub.pat_run = th->ih.pat_run;
    image->num_cols = th->ih.sl_width;
    image->lines_left = th->ih.sl_height;
    image->des_comps = des_comps;
    image->out_comps = out_comps;
    image->bits[0] = th->red < 8 ? th->red : 8;
    image->bits[1] = th->green < 8 ? th->green : 8;
    image->bits[2] = th->blue < 8 ? th->blue : 8;
    image->bits[3] = th->ih.planes - th->red - th->green - th->blue;
    if (image->bits[3] > 8) image->bits[3] = 8;
    image->src_end[0] = (unsigned char *)image->pub.pbuf
					  + byte_width * th->red;
    image->src_end[1] = image->src_end[0] + byte_width * th->green;
    image->src_end[2] = image->src_end[1] + byte_width * th->blue;
    image->src_end[3] = (unsigned char *)image->pub.pbuf + line_len;
    image->des_end = image->src_end[3] +
		     (long)des_comps * th->ih.sl_width;
    image->out_end = image->src_end[3] +
		     (long)out_comps * th->ih.sl_width;
    if (des_comps != out_comps) colorout_init(image);
    image->pub.vrc = 1;
    image->true_put = true_put;
    image->user_exit = user_exit;
    image->pub.put_line = (void (*)(IBUFPUB *))put_true;
  }
  return &image->pub;
}

static void put_true(IBUF *ip)
{
  unsigned char *src, *des, *sp;
  short comp, num_bits, bits;
  unsigned char mask, val;
  long byte_width;

  byte_width = ip->byte_width;
  do
  { comp = 0;
    do
    { des = ip->src_end[3] + comp;
      src = ip->src_end[comp];
      num_bits = ip->bits[comp];
      mask = 0x80;
      do
      { val = 0;
	if ((bits = num_bits) != 0)
	{
	  sp = src;
	  do
	  { sp -= byte_width; val <<= 1;
	    if (*sp & mask) val++;
	  }
	  while (--bits);
	  switch (num_bits) /* Upscaling */
	  {
	    case 1: val |= val << 1; /* fall through! */
	    case 2: val |= val << 2; /* fall through! */
	    case 4: val |= val << 4; break;
	    case 5: val <<= 3; val |= val >> 5; break;
	    case 3: val |= val << 3; /* fall through! */
	    case 6: val <<= 2; val |= val >> 6; break;
	    case 7: val <<= 1; val |= val >> 7; break;
	} }
	*des = val; des += ip->des_comps;
	if ((mask >>= 1) == 0)
	{
	  mask = 0x80; src++;
      } }
      while (des < ip->des_end);
    }
    while (++comp < ip->des_comps);

    if (comp != ip->out_comps) (*ip->color_convert)(ip);

    (*ip->true_put->put_true_pix_row)(ip->true_put,
      ip->src_end[3], ip->num_cols, ip->out_comps);

    if (--ip->lines_left <= 0) (*ip->user_exit)();
  }
  while (--ip->pub.vrc);
  ip->pub.vrc++;
  ip->pub.pbuf = ip->line_adr; ip->pub.bytes_left = ip->length;
}

/* The following code is based in part on the work of the
 * Independent JPEG Group, Copyright (C) 1991-1995, Thomas G. Lane.
 * (File jccolor.c)
 */

/**************** RGB -> Y(CbCr) conversion **************/

/*
 * YCbCr is defined per CCIR 601-1, except that Cb and Cr are
 * normalized to the range 0..MAXJSAMPLE rather than -0.5 .. 0.5.
 * The conversion equations to be implemented are therefore
 *	Y  =  0.29900 * R + 0.58700 * G + 0.11400 * B
 *	Cb = -0.16874 * R - 0.33126 * G + 0.50000 * B  + MAXJSAMPLE/2
 *	Cr =  0.50000 * R - 0.41869 * G - 0.08131 * B  + MAXJSAMPLE/2
 * (These numbers are derived from TIFF 6.0 section 21, dated 3-June-92.)
 *
 * To avoid floating-point arithmetic, we represent the fractional
 * constants as integers scaled up by 2^16 (about 4 digits precision);
 * we have to divide the products by 2^16, with appropriate rounding,
 * to get the correct answer.
 *
 * For even more speed, we avoid doing any multiplications in the
 * inner loop by precalculating the constants times R,G,B for all
 * possible values.
 * The rounding fudge-factor of 0.5 is included in the tables to
 * save adding them separately in the inner loop.
 */

#define SCALEBITS	16
#define ONE_HALF	(1L << (SCALEBITS-1))
#define FIX(x)		((long)((x) * (1L << SCALEBITS) + 0.5))

static void rgb_y_convert(IBUF *ip)
{
  long y, *r_y_tab, *g_y_tab, *b_y_tab;
  unsigned char *src, *des;
  short i;

  i = ip->num_cols;
  r_y_tab = ip->r_y_tab;
  g_y_tab = ip->g_y_tab;
  b_y_tab = ip->b_y_tab;
  des = src = ip->src_end[3];
  do
  { y  = r_y_tab[*src++]; /* R -> Y */
    y += g_y_tab[*src++]; /* G -> Y */
    y += b_y_tab[*src++]; /* B -> Y */
    y >>= SCALEBITS;
    *des++ = (unsigned char)y;
  }
  while (--i);
}

static void rgba_ya_convert(IBUF *ip)
{
  long y, *r_y_tab, *g_y_tab, *b_y_tab;
  unsigned char *src, *des;
  short i;

  i = ip->num_cols;
  r_y_tab = ip->r_y_tab;
  g_y_tab = ip->g_y_tab;
  b_y_tab = ip->b_y_tab;
  des = src = ip->src_end[3];
  do
  { y  = r_y_tab[*src++]; /* R -> Y */
    y += g_y_tab[*src++]; /* G -> Y */
    y += b_y_tab[*src++]; /* B -> Y */
    y >>= SCALEBITS;
    *des++ = (unsigned char)y;
    *des++ = *src++;      /* A -> A */
  }
  while (--i);
}

static void y_rgb_convert(IBUF *ip)
{
  unsigned char *src, *des;

  src = ip->des_end; des = ip->out_end;
  do
  { *--des = *--src; /* Y -> B */
    *--des = *src;   /* Y -> G */
    *--des = *src;   /* Y -> R */
  }
  while (src != des);
}

static void ya_rgba_convert(IBUF *ip)
{
  unsigned char *src, *des;

  src = ip->des_end; des = ip->out_end;
  do
  { *--des = *--src; /* A -> A */
    *--des = *--src; /* Y -> B */
    *--des = *src;   /* Y -> G */
    *--des = *src;   /* Y -> R */
  }
  while (src != des);
}

static void colorout_init(IBUF *ip)
{
  long *r_y_tab, *g_y_tab, *b_y_tab;
  long y1, y2, y3;
  int i;

  /* Initialize color conversion function pointer. */

  ip->color_convert = ip->des_comps == 1 ? y_rgb_convert :
		      ip->des_comps == 2 ? ya_rgba_convert :
		      ip->des_comps == 3 ? rgb_y_convert :
					   rgba_ya_convert;
  if (ip->des_comps < 3) return;

  /* Initialize RGB => Y conversion tables. */

  y1 = y2 = 0; y3 = ONE_HALF; i = 256;
  r_y_tab = ip->r_y_tab;
  g_y_tab = ip->g_y_tab;
  b_y_tab = ip->b_y_tab;
  do
  { *r_y_tab++ = y1; y1 += FIX(0.29900);
    *g_y_tab++ = y2; y2 += FIX(0.58700);
    *b_y_tab++ = y3; y3 += FIX(0.11400);
  }
  while (--i);
}
