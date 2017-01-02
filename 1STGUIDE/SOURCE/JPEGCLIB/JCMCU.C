/*
 * jcmcu.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains MCU extraction routines and quantization scaling.
 * These routines are invoked via the extract_MCUs and
 * extract_init/term methods.
 */

#include "jinclude.h"


/*
 * If this file is compiled with -DDCT_ERR_STATS, it will reverse-DCT each
 * block and sum the total errors across the whole picture.  This provides
 * a convenient method of using real picture data to test the roundoff error
 * of a DCT algorithm.  DCT_ERR_STATS should *not* be defined for a production
 * compression program, since compression is much slower with it defined.
 * Also note that jrevdct.o must be linked into the compressor when this
 * switch is defined.
 */

#ifdef DCT_ERR_STATS
static int dcterrorsum;		/* these hold the error statistics */
static int dcterrormax;
static int dctcoefcount;	/* This will probably overflow on a 16-bit-int machine */
#endif


/* ZAG[i] is the natural-order position of the i'th element of zigzag order. */

static const unsigned char ZAG[DCTSIZE2] = {
  0,  1,  8, 16,  9,  2,  3, 10,
 17, 24, 32, 25, 18, 11,  4,  5,
 12, 19, 26, 33, 40, 48, 41, 34,
 27, 20, 13,  6,  7, 14, 21, 28,
 35, 42, 49, 56, 57, 50, 43, 36,
 29, 22, 15, 23, 30, 37, 44, 51,
 58, 59, 52, 45, 38, 31, 39, 46,
 53, 60, 61, 54, 47, 55, 62, 63
};


LOCAL void
extract_block (JSAMPARRAY input_data, long start_col,
	       JBLOCK output_data, QUANT_TBL_PTR quanttbl)
/* Extract one 8x8 block from the specified location in the sample array; */
/* perform forward DCT, quantization scaling, and zigzag reordering on it. */
{
  /* This routine is heavily used, so it's worth coding it tightly. */
  DCTBLOCK block;
#ifdef DCT_ERR_STATS
  DCTBLOCK svblock;		/* saves input data for comparison */
#endif

  { register JSAMPROW elemptr;
    register DCTELEM *localblkptr = block;
#if DCTSIZE != 8
    register int elemc;
#endif
    register int elemr;

    for (elemr = DCTSIZE; elemr > 0; elemr--) {
      elemptr = *input_data++ + start_col;
#if DCTSIZE == 8		/* unroll the inner loop */
      *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
      *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
      *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
      *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
      *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
      *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
      *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
      *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
#else
      for (elemc = DCTSIZE; elemc > 0; elemc--) {
	*localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
      }
#endif
    }
  }

#ifdef DCT_ERR_STATS
  MEMCOPY(svblock, block, SIZEOF(DCTBLOCK));
#endif

  j_fwd_dct(block);

  { register JCOEF temp, qval;
    register int i;

    for (i = 0; i < DCTSIZE2; i++) {
      qval = (JCOEF) *quanttbl++;
      /* Divide the coefficient value by qval, ensuring proper rounding.
       * Since C does not specify the direction of rounding for negative
       * quotients, we have to force the dividend positive for portability.
       *
       * In most files, at least half of the output values will be zero
       * (at default quantization settings, more like three-quarters...)
       * so we should ensure that this case is fast.  On many machines,
       * a comparison is enough cheaper than a divide to make a special test
       * a win.  Since both inputs will be nonnegative, we need only test
       * for a < b to discover whether a/b is 0.
       * If your machine's division is fast enough, define FAST_DIVIDE.
       */
#ifdef FAST_DIVIDE
#define DIVIDE_BY(a,b)	a /= b
#else
#define DIVIDE_BY(a,b)	if (a >= b) a /= b; else a = 0
#endif
      if ((temp = (JCOEF) block[ZAG[i]]) < 0) {
	temp = -temp;
	temp += qval>>1;	/* for rounding */
	DIVIDE_BY(temp, qval);
	temp = -temp;
      } else {
	temp += qval>>1;	/* for rounding */
	DIVIDE_BY(temp, qval);
      }
      *output_data++ = temp;
    }
  }

#ifdef DCT_ERR_STATS
  j_rev_dct(block);

  { register int diff;
    register short i;

    for (i = 0; i < DCTSIZE2; i++) {
      diff = block[i] - svblock[i];
      if (diff < 0) diff = -diff;
      dcterrorsum += diff;
      if (dcterrormax < diff) dcterrormax = diff;
    }
    dctcoefcount += DCTSIZE2;
  }
#endif
}


/*
 * Extract samples in MCU order, process & hand off to output_method.
 * The input is always exactly N MCU rows worth of data.
 */

METHODDEF void
extract_MCUs (compress_info_ptr cinfo, JSAMPIMAGE image_data,
	      long cur_mcu_row, int num_mcu_rows,
	      MCU_output_method_ptr output_method)
{
  JBLOCK MCU_data[MAX_BLOCKS_IN_MCU];
  long mcurow, start_row, base_row;
  long mcucol, start_col;
  short ci, xcount, ycount;
  JBLOCKROW blk_ptr;
  jpeg_component_info * compptr;
  QUANT_TBL_PTR quant_ptr;

  for (mcurow = cur_mcu_row; --num_mcu_rows >= 0; mcurow++) {
    for (mcucol = 0; mcucol < cinfo->MCUs_per_row; mcucol++) {
      /* Extract data from the image array, DCT it, and quantize it.
       * Dummy blocks at the right or bottom edge are filled in specially.
       * The data in them does not matter for image reconstruction, so we
       * fill them with values that will encode to the smallest amount of
       * data, viz: all zeroes in the AC entries, DC entries equal to previous
       * block's DC value.  (Thanks to Thomas Kinsman for this idea.)
       */
      blk_ptr = MCU_data;
      for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
	compptr = cinfo->cur_comp_info[ci];
	quant_ptr = cinfo->quant_tbl_ptrs[compptr->quant_tbl_no];
	base_row = (cur_mcu_row * compptr->MCU_height)*DCTSIZE;
	start_row = (mcurow * compptr->MCU_height)*DCTSIZE;
	for (ycount = compptr->MCU_height; --ycount >= 0;) {
	  start_col = (mcucol * compptr->MCU_width)*DCTSIZE;
	  for (xcount = compptr->MCU_width; --xcount >= 0;) {
	    if (start_col < compptr->true_comp_width &&
		start_row < compptr->true_comp_height) {
	      extract_block(image_data[ci] + (start_row - base_row),
			    start_col, *blk_ptr, quant_ptr);
	    }
	    else {
	      MEMZERO(*blk_ptr, SIZEOF(*blk_ptr));
	      blk_ptr[0][0] = blk_ptr[-1][0];
	    }
	    blk_ptr++;
	    start_col += DCTSIZE;
	  }
	  start_row += DCTSIZE;
	}
      }
      /* Send the MCU whereever the pipeline controller wants it to go */
      (*output_method) (cinfo, MCU_data);
    }
  }
}


/*
 * Initialize for processing a scan.
 */

METHODDEF void
extract_init (compress_info_ptr cinfo)
{
  /* no work for now */
#ifdef DCT_ERR_STATS
  dcterrorsum = dcterrormax = dctcoefcount = 0;
#endif
}


/*
 * Clean up after a scan.
 */

METHODDEF void
extract_term (compress_info_ptr cinfo)
{
  /* no work for now */
#ifdef DCT_ERR_STATS
  TRACEMS3(cinfo->emethods, 0, "DCT roundoff errors = %d/%d,  max = %d",
	   dcterrorsum, dctcoefcount, dcterrormax);
#endif
}



/*
 * The method selection routine for MCU extraction.
 */

GLOBAL void
jselcmcu (compress_info_ptr cinfo)
{
  /* just one implementation for now */
  cinfo->methods->extract_init = extract_init;
  cinfo->methods->extract_MCUs = extract_MCUs;
  cinfo->methods->extract_term = extract_term;
}
