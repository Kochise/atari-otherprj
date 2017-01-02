/*
 * jdsample.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains upsampling routines.
 * These routines are invoked via the upsample and
 * upsample_init/term methods.
 *
 * An excellent reference for image resampling is
 *   Digital Image Warping, George Wolberg, 1990.
 *   Pub. by IEEE Computer Society Press, Los Alamitos, CA. ISBN 0-8186-8944-7.
 */

#include "jinclude.h"


/*
 * Upsample pixel values of a single component.
 * This version handles any integral sampling ratios.
 *
 * This is not used for typical JPEG files, so it need not be fast.
 * Nor, for that matter, is it particularly accurate: the algorithm is
 * simple replication of the input pixel onto the corresponding output
 * pixels.  The hi-falutin sampling literature refers to this as a
 * "box filter".  A box filter tends to introduce visible artifacts,
 * so if you are actually going to use 3:1 or 4:1 sampling ratios
 * you would be well advised to improve this code.
 */

METHODDEF void
int_upsample (long input_cols, int input_rows,
	      long output_cols, int output_rows,
	      JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr)
{
  JSAMPARRAY output_data = *output_data_ptr;
  JSAMPROW inptr, outptr;
  JSAMPLE invalue;
  long incol, h;
  int inrow, v;

  inrow = input_rows;
  do {
    v = output_rows;
    do {
      inptr = *input_data;
      outptr = *output_data++;
      incol = input_cols;
      do {
	h = output_cols; invalue = GETJSAMPLE(*inptr++);
	do *outptr++ = invalue; while ((h -= input_cols) > 0);
      } while (--incol > 0);
    } while ((v -= input_rows) > 0);
    input_data++;
  } while (--inrow > 0);
}


/*
 * Upsample pixel values of a single component.
 * This version handles the common case of 2:1 horizontal and 1:1 vertical.
 *
 * The upsampling algorithm is linear interpolation between pixel centers,
 * also known as a "triangle filter".  This is a good compromise between
 * speed and visual quality.  The centers of the output pixels are 1/4 and 3/4
 * of the way between input pixel centers.
 */

METHODDEF void
h2v1_upsample (long input_cols, int input_rows,
	       long output_cols, int output_rows,
	       JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr)
{
  JSAMPARRAY output_data = *output_data_ptr;
  JSAMPROW inptr, outptr;
  int invalue;
  long colctr;

  do {
    inptr = *input_data++;
    outptr = *output_data++;

    /* Special case for first column */
    invalue = GETJSAMPLE(*inptr++);
    *outptr++ = (JSAMPLE) invalue;
    *outptr++ = (JSAMPLE) ((invalue * 3 + GETJSAMPLE(*inptr) + 2) >> 2);

    for (colctr = input_cols - 2; --colctr >= 0;) {
      /* General case: 3/4 * nearer pixel + 1/4 * further pixel */
      invalue = GETJSAMPLE(*inptr++) * 3 + 2;
      *outptr++ = (JSAMPLE) ((invalue + GETJSAMPLE(inptr[-2])) >> 2);
      *outptr++ = (JSAMPLE) ((invalue + GETJSAMPLE(*inptr)) >> 2);
    }

    /* Special case for last column */
    invalue = GETJSAMPLE(*inptr);
    *outptr++ = (JSAMPLE) ((invalue * 3 + GETJSAMPLE(*--inptr) + 2) >> 2);
    *outptr++ = (JSAMPLE) invalue;
  } while (--input_rows > 0);
}


/*
 * Upsample pixel values of a single component.
 * This version handles the common case of 2:1 horizontal and 2:1 vertical.
 *
 * The upsampling algorithm is linear interpolation between pixel centers,
 * also known as a "triangle filter".  This is a good compromise between
 * speed and visual quality.  The centers of the output pixels are 1/4 and 3/4
 * of the way between input pixel centers.
 */

METHODDEF void
h2v2_upsample (long input_cols, int input_rows,
	       long output_cols, int output_rows,
	       JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr)
{
  JSAMPARRAY output_data = *output_data_ptr;
  JSAMPROW inptr0, inptr1, outptr;
#ifdef EIGHT_BIT_SAMPLES
  int thiscolsum, lastcolsum, nextcolsum;
#else
  INT32 thiscolsum, lastcolsum, nextcolsum;
#endif
  int v;
  long colctr;

  v = 0;
  do {
    /* inptr0 points to nearest input row, inptr1 points to next nearest */
    if ((v = ~v) != 0) {
      inptr0 = input_data[0];
      inptr1 = input_data[-1];	/* next nearest is row above */
    } else {
      inptr0 = *input_data++;
      inptr1 = input_data[0];	/* next nearest is row below */
    }
    outptr = *output_data++;

    thiscolsum = GETJSAMPLE(*inptr0++) * 3 + GETJSAMPLE(*inptr1++);
    lastcolsum = thiscolsum;

    for (colctr = input_cols; --colctr > 0;) {
      /* General case: 3/4 * nearer pixel + 1/4 * further pixel in each */
      /* dimension, thus 9/16, 3/16, 3/16, 1/16 overall */
      nextcolsum = thiscolsum * 3 + 8;
      lastcolsum += nextcolsum; *outptr++ = (JSAMPLE) (lastcolsum >>= 4);
      lastcolsum = thiscolsum;
      thiscolsum = GETJSAMPLE(*inptr0++) * 3 + GETJSAMPLE(*inptr1++);
      nextcolsum += thiscolsum; *outptr++ = (JSAMPLE) (nextcolsum >>= 4);
    }

    /* Special case for last column */
    nextcolsum = thiscolsum * 3 + 8;
    lastcolsum += nextcolsum; *outptr++ = (JSAMPLE) (lastcolsum >>= 4);
    nextcolsum += thiscolsum; *outptr++ = (JSAMPLE) (nextcolsum >>= 4);
  } while (--output_rows > 0);
}


/*
 * Upsample pixel values of a single component.
 * This version handles the special case of a full-size component.
 */

METHODDEF void
fullsize_upsample (long input_cols, int input_rows,
		   long output_cols, int output_rows,
		   JSAMPARRAY input_data, JSAMPARRAY * output_data_ptr)
{
  *output_data_ptr = input_data;
}



/*
 * The method selection routine for upsampling.
 * Note that we must select a routine for each component.
 */

GLOBAL void
jselupsample (decompress_info_ptr cinfo)
{
  jpeg_component_info * compptr;

  if (cinfo->CCIR601_sampling)
    ERREXIT(cinfo->emethods, "CCIR601 upsampling not implemented yet");

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan) {
    if (compptr->h_samp_factor == cinfo->max_h_samp_factor &&
	compptr->v_samp_factor == cinfo->max_v_samp_factor)
      compptr->resample = fullsize_upsample;
    else if (compptr->h_samp_factor * 2 == cinfo->max_h_samp_factor &&
	     compptr->v_samp_factor == cinfo->max_v_samp_factor)
      compptr->resample = h2v1_upsample;
    else if (compptr->h_samp_factor * 2 == cinfo->max_h_samp_factor &&
	     compptr->v_samp_factor * 2 == cinfo->max_v_samp_factor)
      compptr->resample = h2v2_upsample;
    else if ((cinfo->max_h_samp_factor % compptr->h_samp_factor) == 0 &&
	     (cinfo->max_v_samp_factor % compptr->v_samp_factor) == 0)
      compptr->resample = int_upsample;
    else
      ERREXIT(cinfo->emethods, "Fractional upsampling not implemented yet");
  }
}
