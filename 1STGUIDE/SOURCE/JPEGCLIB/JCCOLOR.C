/*
 * jccolor.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains input colorspace conversion routines.
 * These routines are invoked via the methods color_convert
 * and colorin_init/term.
 */

#include "jinclude.h"


/**************** RGB -> YCbCr conversion: most common case **************/

/*
 * YCbCr is defined per CCIR 601-1, except that Cb and Cr are
 * normalized to the range 0..MAXJSAMPLE rather than -0.5 .. 0.5.
 * The conversion equations to be implemented are therefore
 *	Y  =  0.29900 * R + 0.58700 * G + 0.11400 * B
 *	Cb = -0.16874 * R - 0.33126 * G + 0.50000 * B  + MAXJSAMPLE/2
 *	Cr =  0.50000 * R - 0.41869 * G - 0.08131 * B  + MAXJSAMPLE/2
 * (These numbers are derived from TIFF 6.0 section 21, dated 3-June-92.)
 *
 * To avoid floating-point arithmetic, we represent the fractional constants
 * as integers scaled up by 2^16 (about 4 digits precision); we have to divide
 * the products by 2^16, with appropriate rounding, to get the correct answer.
 *
 * For even more speed, we avoid doing any multiplications in the inner loop
 * by precalculating the constants times R,G,B for all possible values.
 * For 8-bit JSAMPLEs this is very reasonable (only 256 entries per table);
 * for 12-bit samples it is still acceptable.  It's not very reasonable for
 * 16-bit samples, but if you want lossless storage you shouldn't be changing
 * colorspace anyway.
 * The MAXJSAMPLE/2 offsets and the rounding fudge-factor of 0.5 are included
 * in the tables to save adding them separately in the inner loop.
 */

#ifdef SIXTEEN_BIT_SAMPLES
#define SCALEBITS	14	/* avoid overflow */
#else
#define SCALEBITS	16	/* speedier right-shift on some machines */
#endif
#define ONE_HALF	((INT32) 1 << (SCALEBITS-1))
#define FIX(x)		((INT32) ((x) * (1L<<SCALEBITS) + 0.5))

typedef struct
{
  INT32 r_y[MAXJSAMPLE+1],	/* R => Y section */
	g_y[MAXJSAMPLE+1],	/* G => Y section */
	b_y[MAXJSAMPLE+1],	/* etc. */
	r_cb[MAXJSAMPLE+1],
	g_cb[MAXJSAMPLE+1],
	b_cb[MAXJSAMPLE+1],
/*	r_cr[MAXJSAMPLE+1],	   B=>Cb, R=>Cr are the same */
	g_cr[MAXJSAMPLE+1],
	b_cr[MAXJSAMPLE+1];
}
RGB_YCC_TAB;

#define r_cr b_cb

static void *Tab;


/*
 * Initialize for colorspace conversion.
 */

METHODDEF void
rgb_ycc_init (compress_info_ptr cinfo)
{
  INT32 i;
  RGB_YCC_TAB *tab;

  tab = (RGB_YCC_TAB *)
  (Tab = (*cinfo->emethods->alloc_small)(sizeof(RGB_YCC_TAB)));
  i = 0;
  do
  { *tab->r_cb = (-FIX(0.16874)) * i;
    *tab->g_cb = (-FIX(0.33126)) * i;
    *tab->b_cb = FIX(0.50000) * i +
				((INT32) CENTERJSAMPLE << SCALEBITS);
/*  B=>Cb and R=>Cr tables are the same
    *tab->r_cr = FIX(0.50000) * i +
				((INT32) CENTERJSAMPLE << SCALEBITS);
*/
    *tab->g_cr = (-FIX(0.41869)) * i;
    *tab->b_cr = (-FIX(0.08131)) * i;
    *tab->b_y = FIX(0.11400) * i + ONE_HALF;
    *tab->g_y = FIX(0.58700) * i;
    *((INT32 *)tab)++ = FIX(0.29900) * i;
  }
  while (++(int)i <= MAXJSAMPLE);
}


/*
 * Convert some rows of pixels to the JPEG colorspace.
 */

METHODDEF void
rgb_ycc_convert (compress_info_ptr cinfo,
		 int num_rows, JSAMPIMAGE image_data)
{
  long num_cols = cinfo->image_width;
  RGB_YCC_TAB *tab = (RGB_YCC_TAB *)Tab;
  INT32 i = 0, y, cb, cr;

  while (--num_rows >= 0)
  {
    JSAMPROW ptr0 = image_data[0][num_rows],
	     ptr1 = image_data[1][num_rows],
	     ptr2 = image_data[2][num_rows];
    long col = num_cols;
    do
    { /* If the inputs are 0..MAXJSAMPLE, the outputs of these equations
       * must be too; we do not need an explicit range-limiting operation.
       * Hence the value being shifted is never negative, and we don't
       * need the general RIGHT_SHIFT macro.
       */
      /* Red */
      (JSAMPLE)i = *ptr0;
      cr  = tab->r_cr[i]; cb  = tab->r_cb[i]; y  = tab->r_y[i];
      /* Green */
      (JSAMPLE)i = *ptr1;
      cr += tab->g_cr[i]; cb += tab->g_cb[i]; y += tab->g_y[i];
      /* Blue */
      (JSAMPLE)i = *ptr2;
      cr += tab->b_cr[i]; cb += tab->b_cb[i]; y += tab->b_y[i];
      cr >>= SCALEBITS; *ptr2++ = (JSAMPLE)cr;
      cb >>= SCALEBITS; *ptr1++ = (JSAMPLE)cb;
      y  >>= SCALEBITS; *ptr0++ = (JSAMPLE)y;
    }
    while (--col > 0);
  }
}


/**************** Cases other than RGB -> YCbCr **************/


/*
 * Convert some rows of pixels to the JPEG colorspace.
 * This version handles RGB->grayscale conversion, which is the same
 * as the RGB->Y portion of RGB->YCbCr.
 * We assume rgb_ycc_init has been called (we only use the Y tables).
 */

METHODDEF void
rgb_gray_convert (compress_info_ptr cinfo,
		  int num_rows, JSAMPIMAGE image_data)
{
  long num_cols = cinfo->image_width;
  RGB_YCC_TAB *tab = (RGB_YCC_TAB *)Tab;

  while (--num_rows >= 0)
  {
    JSAMPROW ptr0 = image_data[0][num_rows],
	     ptr1 = image_data[1][num_rows],
	     ptr2 = image_data[2][num_rows];
    long col = num_cols;
    do
    { /* If the inputs are 0..MAXJSAMPLE, the outputs of these equations
       * must be too; we do not need an explicit range-limiting operation.
       * Hence the value being shifted is never negative, and we don't
       * need the general RIGHT_SHIFT macro.
       */
      INT32 y = (tab->r_y[*ptr0  ]
	       + tab->g_y[*ptr1++]
	       + tab->b_y[*ptr2++]) >> SCALEBITS;
      *ptr0++ = (JSAMPLE)y;
    }
    while (--col > 0);
  }
}


METHODDEF void
dummy_convert () { }


/*
 * The method selection routine for input colorspace conversion.
 */

GLOBAL void
jselccolor (compress_info_ptr cinfo)
{
  /* Make sure input_components agrees with in_color_space */
  switch (cinfo->in_color_space) {
  case CS_GRAYSCALE:
    if (cinfo->input_components != 1)
      ERREXIT(cinfo->emethods, "Bogus input colorspace");
    break;

  case CS_RGB:
  case CS_YCbCr:
  case CS_YIQ:
    if (cinfo->input_components != 3)
      ERREXIT(cinfo->emethods, "Bogus input colorspace");
    break;

  case CS_CMYK:
    if (cinfo->input_components != 4)
      ERREXIT(cinfo->emethods, "Bogus input colorspace");
    break;

  default:
    ERREXIT(cinfo->emethods, "Unsupported input colorspace");
    break;
  }

  /* Standard init/term methods (may override below) */
  cinfo->methods->color_convert = dummy_convert;
  cinfo->methods->colorin_init =
  cinfo->methods->colorin_term = dummy_convert;

  /* Check num_components, set conversion method based on requested space */
  switch (cinfo->jpeg_color_space) {
  case CS_GRAYSCALE:
    if (cinfo->num_components != 1)
      ERREXIT(cinfo->emethods, "Bogus JPEG colorspace");
    if (cinfo->in_color_space == CS_RGB) {
      cinfo->methods->colorin_init = rgb_ycc_init;
      cinfo->methods->color_convert = rgb_gray_convert;
    } else if (cinfo->in_color_space != CS_GRAYSCALE &&
	       cinfo->in_color_space != CS_YCbCr)
      ERREXIT(cinfo->emethods, "Unsupported color conversion request");
    break;

  case CS_YCbCr:
    if (cinfo->num_components != 3)
      ERREXIT(cinfo->emethods, "Bogus JPEG colorspace");
    if (cinfo->in_color_space == CS_RGB) {
      cinfo->methods->colorin_init = rgb_ycc_init;
      cinfo->methods->color_convert = rgb_ycc_convert;
    } else if (cinfo->in_color_space != CS_YCbCr)
      ERREXIT(cinfo->emethods, "Unsupported color conversion request");
    break;

  case CS_CMYK:
    if (cinfo->num_components != 4)
      ERREXIT(cinfo->emethods, "Bogus JPEG colorspace");
    if (cinfo->in_color_space != CS_CMYK)
      ERREXIT(cinfo->emethods, "Unsupported color conversion request");
    break;

  default:
    ERREXIT(cinfo->emethods, "Unsupported JPEG colorspace");
  }
}
