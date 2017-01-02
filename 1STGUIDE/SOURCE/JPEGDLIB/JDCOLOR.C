/*
 * jdcolor.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains output colorspace conversion routines.
 * These routines are invoked via the methods color_convert
 * and colorout_init/term.
 */

#include "jinclude.h"


#ifdef SIXTEEN_BIT_SAMPLES
#define SCALEBITS	14	/* avoid overflow */
#else
#define SCALEBITS	16	/* speedier right-shift on some machines */
#endif
#define ONE_HALF	((INT32) 1 << (SCALEBITS-1))
#define FIX(x)		((INT32) ((x) * (1L<<SCALEBITS) + 0.5))

typedef struct
{
  INT32	cr_r[MAXJSAMPLE+1],	/* => table for Cr to R conversion */
	cb_b[MAXJSAMPLE+1],	/* => table for Cb to B conversion */
	cr_g[MAXJSAMPLE+1],	/* => table for Cr to G conversion */
	cb_g[MAXJSAMPLE+1];	/* => table for Cb to G conversion */
}
YCC_RGB_TAB;

typedef struct
{
  INT32	r_y[MAXJSAMPLE+1],	/* R => Y section */
	g_y[MAXJSAMPLE+1],	/* G => Y section */
	b_y[MAXJSAMPLE+1];	/* B => Y section */
}
RGB_Y_TAB;


YCC_RGB_TAB ycc_rgb_tab;
RGB_Y_TAB rgb_y_tab;


/**************** YCbCr -> RGB conversion: most common case **************/

/*
 * YCbCr is defined per CCIR 601-1, except that Cb and Cr are
 * normalized to the range 0..MAXJSAMPLE rather than -0.5 .. 0.5.
 * The conversion equations to be implemented are therefore
 *	R = Y                + 1.40200 * Cr
 *	G = Y - 0.34414 * Cb - 0.71414 * Cr
 *	B = Y + 1.77200 * Cb
 * where Cb and Cr represent the incoming values less MAXJSAMPLE/2.
 * (These numbers are derived from TIFF 6.0 section 21, dated 3-June-92.)
 *
 * To avoid floating-point arithmetic, we represent the fractional constants
 * as integers scaled up by 2^16 (about 4 digits precision); we have to divide
 * the products by 2^16, with appropriate rounding, to get the correct answer.
 * Notice that Y, being an integral input, does not contribute any fraction
 * so it need not participate in the rounding.
 *
 * For even more speed, we avoid doing any multiplications in the inner loop
 * by precalculating the constants times Cb and Cr for all possible values.
 * For 8-bit JSAMPLEs this is very reasonable (only 256 entries per table);
 * for 12-bit samples it is still acceptable.  It's not very reasonable for
 * 16-bit samples, but if you want lossless storage you shouldn't be changing
 * colorspace anyway.
 * The Cr=>R and Cb=>B values can be rounded to integers in advance; the
 * values for the G calculation are left scaled up, since we must add them
 * together before rounding.
 */


/*
 * Initialize for colorspace conversion.
 */

GLOBAL void
ycc_rgb_init ()
{
  static int init_flag = 0;

  if (init_flag == 0)
  {
    INT32 x1, x2, x3, x4;
    int i;
    YCC_RGB_TAB *tab = &ycc_rgb_tab;

    init_flag++;
    x1 = FIX(0.34414) * CENTERJSAMPLE + ONE_HALF;
    x2 = FIX(0.71414) * CENTERJSAMPLE;
    x3 = ONE_HALF - FIX(1.77200) * CENTERJSAMPLE;
    x4 = ONE_HALF - FIX(1.40200) * CENTERJSAMPLE; i = MAXJSAMPLE;
    do
    { /* Cb=>G value is scaled-up -0.34414 * x */
      /* We also add in ONE_HALF so that need not do it in inner loop */
      *tab->cb_g        = x1; x1 -= FIX(0.34414);
      /* Cr=>G value is scaled-up -0.71414 * x */
      *tab->cr_g        = x2; x2 -= FIX(0.71414);
      /* Cb=>B value is nearest int to 1.77200 * x */
      *tab->cb_b        = x3 >> SCALEBITS; x3 += FIX(1.77200);
      /* Cr=>R value is nearest int to 1.40200 * x */
      *((INT32 *)tab)++ = x4 >> SCALEBITS; x4 += FIX(1.40200);
    }
    while (--i >= 0);
  }
}


/*
 * Convert some rows of samples to the output colorspace.
 */

METHODDEF void
ycc_rgb_convert (decompress_info_ptr cinfo, int num_rows,
		 JSAMPIMAGE pixel_data, ...)
{
  long num_cols = cinfo->image_width;
  JSAMPROW range_limit = cinfo->sample_range_limit;
  YCC_RGB_TAB *tab = &ycc_rgb_tab;
  INT32 y = 0, cb = 0, cr = 0;

  while (--num_rows >= 0)
  {
    JSAMPROW ptr0 = pixel_data[0][num_rows],
	     ptr1 = pixel_data[1][num_rows],
	     ptr2 = pixel_data[2][num_rows];
    long col = num_cols;
    do
    { (JSAMPLE) y = *ptr0;
      (JSAMPLE)cb = *ptr1;
      (JSAMPLE)cr = *ptr2;
      /* Note: if the inputs were computed directly from RGB values,
       * range-limiting would be unnecessary here; but due to possible
       * noise in the DCT/IDCT phase, we do need to apply range limits.
       */
      *ptr2++ = range_limit[y +   tab->cb_b[cb]];		/* blue */
      *ptr1++ = range_limit[y + ((tab->cb_g[cb] +		/* green */
				  tab->cr_g[cr]) >> SCALEBITS)];
      *ptr0++ = range_limit[y +   tab->cr_r[cr]];		/* red */
    }
    while (--col > 0);
  }
}


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


/*
 * Initialize for colorspace conversion.
 */

GLOBAL void
rgb_y_init ()
{
  static int init_flag = 0;

  if (init_flag == 0)
  {
    INT32 x1, x2, x3;
    int i;
    RGB_Y_TAB *tab = &rgb_y_tab;

    init_flag++;
    x1 = ONE_HALF; x2 = x3 = 0; i = MAXJSAMPLE;
    do
    { *tab->b_y         = x1; x1 += FIX(0.11400);
      *tab->g_y         = x2; x2 += FIX(0.58700);
      *((INT32 *)tab)++ = x3; x3 += FIX(0.29900);
    }
    while (--i >= 0);
  }
}


/*
 * Convert some rows of samples to the output colorspace.
 */

METHODDEF void
rgb_y_convert (decompress_info_ptr cinfo, int num_rows,
	       JSAMPIMAGE pixel_data, ...)
{
  long num_cols = cinfo->image_width;
  RGB_Y_TAB *tab = &rgb_y_tab;

  while (--num_rows >= 0)
  {
    JSAMPROW ptr0 = pixel_data[0][num_rows],
	     ptr1 = pixel_data[1][num_rows],
	     ptr2 = pixel_data[2][num_rows];
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


/**************** Other cases **************/


METHODDEF void
dummy_convert () { }



/*
 * The method selection routine for output colorspace conversion.
 */

GLOBAL void
jseldcolor (decompress_info_ptr cinfo)
{
  /* Set color_out_comps and conversion method based on requested space */
  cinfo->color_out_comps = cinfo->num_components;
  cinfo->color_convert = dummy_convert;
  cinfo->colorout_init = dummy_convert;
  cinfo->colorout_term = dummy_convert;
  if (cinfo->out_color_space != cinfo->jpeg_color_space)
    switch (cinfo->out_color_space) {
    case CS_GRAYSCALE:
      cinfo->color_out_comps = 1;
      switch (cinfo->jpeg_color_space) {
      case CS_RGB:
	cinfo->color_convert = rgb_y_convert;
	cinfo->colorout_init = rgb_y_init;
      case CS_YCbCr:
	break;
      default:
	ERREXIT(cinfo->emethods, "Unsupported color conversion request");
      }
      break;
    case CS_RGB:
      if (cinfo->jpeg_color_space == CS_YCbCr) {
	cinfo->color_convert = ycc_rgb_convert;
	cinfo->colorout_init = ycc_rgb_init;
	break;
      }
    default:
      ERREXIT(cinfo->emethods, "Unsupported color conversion request");
    }

  cinfo->final_out_comps = cinfo->color_out_comps;
  if (cinfo->quantize_colors)
    cinfo->final_out_comps = 1;	/* single colormapped output component */
}
