/*
 * jdpipe.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains decompression pipeline controllers.
 * These routines are invoked via the d_pipeline_controller method.
 *
 * There are two basic pipeline controllers.  The simpler one handles a
 * single-scan JPEG file (single component or fully interleaved) with no
 * color quantization or 1-pass quantization.  In this case, the file can
 * be processed in one top-to-bottom pass.  The more complex controller is
 * used when 2-pass color quantization is requested and/or the JPEG file
 * has multiple scans (noninterleaved or partially interleaved).  In this
 * case, the entire image must be buffered up in a "big" array.
 */

#include "jinclude.h"


/*
 * About the data structures:
 *
 * The processing chunk size for upsampling is referred to in this file as
 * a "row group": a row group is defined as Vk (v_samp_factor) sample rows of
 * any component while downsampled, or Vmax (max_v_samp_factor) unsubsampled
 * rows.  In an interleaved scan each MCU row contains exactly DCTSIZE row
 * groups of each component in the scan.  In a noninterleaved scan an MCU row
 * is one row of blocks, which might not be an integral number of row groups;
 * therefore, we read in Vk MCU rows to obtain the same amount of data as we'd
 * have in an interleaved scan.
 * To provide context for the upsampling step, we have to retain the last
 * two row groups of the previous MCU row while reading in the next MCU row
 * (or set of Vk MCU rows).  To do this without copying data about, we create
 * a rather strange data structure.  Exactly DCTSIZE+2 row groups of samples
 * are allocated, but we create two different sets of pointers to this array.
 * The second set swaps the last two pairs of row groups.  By working
 * alternately with the two sets of pointers, we can access the data in the
 * desired order.
 *
 * Cross-block smoothing also needs context above and below the "current" row.
 * Since this is an optional feature, I've implemented it in a way that is
 * much simpler but requires more than the minimum amount of memory.  We
 * simply allocate three extra MCU rows worth of coefficient blocks and use
 * them to "read ahead" one MCU row in the file.  For a typical 1000-pixel-wide
 * image with 2x2,1x1,1x1 sampling, each MCU row is about 50Kb; an 80x86
 * machine may be unable to apply cross-block smoothing to wider images.
 */


/*
 * Utility routines: common code for pipeline controllers
 */

LOCAL long
per_scan_setup (decompress_info_ptr cinfo)
{
  jpeg_component_info *compptr = cinfo->first_comp_in_scan;

  if (compptr->next_comp_in_scan == 0) {

    /* Compute all derived info for a noninterleaved (single-component) scan */

#if DCTSIZE == 8
    cinfo->MCU_cols_in_scan = (compptr->true_comp_width + 7) >> 3;
    cinfo->MCU_rows_in_scan = (compptr->true_comp_height + 7) >> 3;
#else
    cinfo->MCU_cols_in_scan = (compptr->true_comp_width + DCTSIZE-1) / DCTSIZE;
    cinfo->MCU_rows_in_scan = (compptr->true_comp_height + DCTSIZE-1) / DCTSIZE;
#endif

    /* for noninterleaved scan, always one block per MCU */
    compptr->MCU_width = 1;
    compptr->MCU_height = 1;

    /* compute physical dimensions of component */
    compptr->downsampled_width = cinfo->MCU_cols_in_scan * DCTSIZE;
    compptr->downsampled_height = cinfo->MCU_rows_in_scan * DCTSIZE;

    (*cinfo->d_per_scan_method_selection) (cinfo);

    /* need to read Vk MCU rows to obtain Vk block rows */
    return compptr->v_samp_factor;
  }

  /* Compute all derived info for an interleaved (multi-component) scan */

  cinfo->MCU_cols_in_scan = (cinfo->image_width
			     + cinfo->max_h_samp_factor * DCTSIZE - 1)
			    / (cinfo->max_h_samp_factor * DCTSIZE);

  cinfo->MCU_rows_in_scan = (cinfo->image_height
			     + cinfo->max_v_samp_factor * DCTSIZE - 1)
			    / (cinfo->max_v_samp_factor * DCTSIZE);

  do {
    /* for interleaved scan, sampling factors give # of blocks per component */
    compptr->MCU_width = compptr->h_samp_factor;
    compptr->MCU_height = compptr->v_samp_factor;

    /* compute physical dimensions of component */
    compptr->downsampled_width =
      cinfo->MCU_cols_in_scan * compptr->MCU_width * DCTSIZE;
    compptr->downsampled_height =
      cinfo->MCU_rows_in_scan * compptr->MCU_height * DCTSIZE;

    compptr = compptr->next_comp_in_scan;
  } while (compptr);

  (*cinfo->d_per_scan_method_selection) (cinfo);

  /* in an interleaved scan, one MCU row provides Vk block rows */
  return 1;
}


LOCAL void
alloc_MCU_coeff (decompress_info_ptr cinfo)
/* Allocate one MCU's worth of coefficient blocks */
{
  jpeg_component_info *compptr;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan)
    compptr->coeff_data = (*cinfo->emethods->alloc_small_barray)
      ((long) compptr->MCU_width, (long) compptr->MCU_height);
}


LOCAL void
zero_MCU_coeff (decompress_info_ptr cinfo)
{
  jpeg_component_info *compptr;
  JBLOCKARRAY p;
  short h;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan) {
    p = compptr->coeff_data;
    h = compptr->MCU_height;
    do MEMZERO(*p++, (long) compptr->MCU_width * SIZEOF(JBLOCK));
    while (--h > 0);
  }
}


LOCAL void
alloc_full_coeff (decompress_info_ptr cinfo)
{
  jpeg_component_info *compptr;
  long w, h;
  JBLOCKARRAY p;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan) {
#if DCTSIZE == 8
    w = compptr->downsampled_width >> 3;
    h = compptr->downsampled_height >> 3;
#else
    w = compptr->downsampled_width / DCTSIZE;
    h = compptr->downsampled_height / DCTSIZE;
#endif
    p = (*cinfo->emethods->alloc_small_barray) (w, h);
    compptr->coeff_data = p;
    w *= SIZEOF(JBLOCK); do MEMZERO(*p++, w); while (--h > 0);
  }
}


LOCAL void
free_coeff (decompress_info_ptr cinfo)
{
  jpeg_component_info *compptr;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan)
    (*cinfo->emethods->free_small_barray) (compptr->coeff_data);
}


LOCAL void
alloc_sampling_buffer (decompress_info_ptr cinfo)
/* Create a downsampled-data buffer having the desired structure */
/* (see comments at head of file) */
{
  jpeg_component_info *compptr;
  JSAMPARRAY buf0, buf1;
  JSAMPROW sbuf;
  long vs, i;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan) {
    vs = compptr->v_samp_factor;	/* row group height */
    buf0 = (JSAMPARRAY) (*cinfo->emethods->alloc_small)
		(vs * (2 * (DCTSIZE+4) * SIZEOF(JSAMPROW) +
		compptr->downsampled_width * ((DCTSIZE+2) * SIZEOF(JSAMPLE))))
	   + vs;		/* want one row group at negative offsets */
    compptr->sampled_data = buf0;
    compptr->sampled_data1 = buf1 = buf0 + vs * (DCTSIZE+4);
    sbuf = (JSAMPROW) (buf1 + vs * (DCTSIZE+3));
    /* First copy the workspace pointers as-is */
    for (i = 0; i < vs * (DCTSIZE+2); i++) {
      buf0[i] = sbuf;
      buf1[i] = sbuf;
      sbuf += compptr->downsampled_width;
    }
    /* In the second list, put the last four row groups in swapped order */
    for (i = 0; i < vs * 2; i++) {
      buf1[vs*DCTSIZE + i] = buf0[vs*(DCTSIZE-2) + i];
      buf1[vs*(DCTSIZE-2) + i] = buf0[vs*DCTSIZE + i];
    }
    /* The wraparound pointers at top and bottom will be filled later
     * (see set_wraparound_pointers, below).  Initially we want the "above"
     * pointers to duplicate the first actual data line.  This only needs
     * to happen in buf0.
     */
    sbuf = buf0[0]; do *--buf0 = sbuf; while (--vs > 0);
  }
}


LOCAL void
free_sampling_buffer (decompress_info_ptr cinfo)
{
  jpeg_component_info *compptr;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan)
    (*cinfo->emethods->free_small)
      ((compptr->sampled_data < compptr->sampled_data1 ?
	compptr->sampled_data : compptr->sampled_data1) -
	compptr->v_samp_factor);
}


LOCAL void
set_wraparound_pointers (decompress_info_ptr cinfo)
/* Set up the "wraparound" pointers at top and bottom of the pointer lists.
 * This changes the pointer list state from top-of-image to the normal state.
 */
{
  jpeg_component_info *compptr;
  JSAMPARRAY buf0, buf1;
  long vs, i;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan) {
    vs = compptr->v_samp_factor;	/* row group height */
    buf0 = compptr->sampled_data;
    buf1 = compptr->sampled_data1;
    for (i = 0; i < vs; i++) {
      buf0[i - vs] = buf0[vs*(DCTSIZE+1) + i];
      buf1[i - vs] = buf1[vs*(DCTSIZE+1) + i];
      buf0[vs*(DCTSIZE+2) + i] = buf0[i];
      buf1[vs*(DCTSIZE+2) + i] = buf1[i];
    }
  }
}


LOCAL void
set_bottom_pointers (decompress_info_ptr cinfo)
/* Change the pointer lists to duplicate the last sample row at the
 * bottom of the image.
 */
{
  jpeg_component_info *compptr;
  JSAMPARRAY sample_ptr;
  JSAMPROW sample_row;
  short vs;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan) {
    vs = compptr->v_samp_factor;	/* row group height */
    sample_ptr = compptr->sampled_data + vs * (DCTSIZE+2);
    sample_row = sample_ptr[-1];
    do *sample_ptr++ = sample_row;
    while (--vs > 0);
  }
}


LOCAL void
swap_sampling_buffer (decompress_info_ptr cinfo)
{
  jpeg_component_info *compptr;
  JSAMPARRAY sample_ptr;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan) {
    sample_ptr = compptr->sampled_data;
    compptr->sampled_data = compptr->sampled_data1;
    compptr->sampled_data1 = sample_ptr;
  }
}


LOCAL void
realign_sampling_buffer (decompress_info_ptr cinfo, long start_row)
{
  jpeg_component_info *compptr;
  JSAMPARRAY sample_ptr;
  short h;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan) {
    sample_ptr = compptr->sampled_data + start_row * DCTSIZE;
    h = compptr->MCU_height * DCTSIZE;
    do *sample_ptr++ -= compptr->downsampled_width;
    while (--h > 0);
  }
}


/*
 * Several decompression processes need to range-limit values to the range
 * 0..MAXJSAMPLE; the input value may fall somewhat outside this range
 * due to noise introduced by quantization, roundoff error, etc.  These
 * processes are inner loops and need to be as fast as possible.  On most
 * machines, particularly CPUs with pipelines or instruction prefetch,
 * a (range-check-less) C table lookup
 *		x = sample_range_limit[x];
 * is faster than explicit tests
 *		if (x < 0)  x = 0;
 *		else if (x > MAXJSAMPLE)  x = MAXJSAMPLE;
 * These processes all use a common table prepared by the routine below.
 *
 * The table will work correctly for x within MAXJSAMPLE+1 of the legal
 * range.  This is a much wider range than is needed for most cases,
 * but the wide range is handy for color quantization.
 * Note that the table is allocated in near data space on PCs; it's small
 * enough and used often enough to justify this.
 */

LOCAL void
prepare_range_limit_table (decompress_info_ptr cinfo)
/* Allocate and fill in the sample_range_limit table */
{
  JSAMPLE * table;
  int i, jmax;

  table = (JSAMPLE *) (*cinfo->emethods->alloc_small)
			(3 * (MAXJSAMPLE+1) * SIZEOF(JSAMPLE));
  cinfo->sample_range_limit = table + (MAXJSAMPLE+1);
  i = 0; jmax = MAXJSAMPLE;
  do {
    table[(MAXJSAMPLE+1)*2] = jmax;	/* x beyond MAXJSAMPLE */
    table[MAXJSAMPLE+1] = (JSAMPLE) i;	/* sample_range_limit[x] = x */
    *table++ = 0;			/* sample_range_limit[x] = 0 for x<0 */
  } while (++i <= jmax);
}


LOCAL void
duplicate_row (JSAMPARRAY image_data,
	       long num_cols, int source_row, int num_rows)
/* Duplicate the source_row at source_row+1 .. source_row+num_rows */
/* This happens only at the bottom of the image, */
/* so it needn't be super-efficient */
{
  do jcopy_sample_rows(image_data, source_row,
		       image_data, source_row+num_rows, 1, num_cols);
  while (--num_rows > 0);
}


LOCAL void
alloc_fullsize_buffer (decompress_info_ptr cinfo, long fullsize_width)
{
  jpeg_component_info *compptr;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan)
    if (cinfo->max_h_samp_factor != compptr->h_samp_factor ||
	cinfo->max_v_samp_factor != compptr->v_samp_factor)
      *compptr->fullsize_data_ptr =
	(*cinfo->emethods->alloc_small_sarray)
	  (fullsize_width, (long) cinfo->max_v_samp_factor);
}


LOCAL void
emit_groups (decompress_info_ptr cinfo, long fullsize_width,
	     short cur, short num)
{
  jpeg_component_info *compptr;
  long out_rows;

  out_rows = cinfo->max_v_samp_factor;
  do {
    /* Do upsampling expansion of the current row group (for each component). */
    for (compptr = cinfo->first_comp_in_scan; compptr;
	 compptr = compptr->next_comp_in_scan)
      (*compptr->resample)
	(compptr->downsampled_width, compptr->v_samp_factor,
	 fullsize_width, (int) out_rows,
	 compptr->sampled_data + compptr->v_samp_factor * cur,
	 compptr->fullsize_data_ptr);

    /* Do color processing and output of full-size rows. */
    /* This is not used when doing 2-pass color quantization. */
    /* Assumes 1-pass color quantization works in output colorspace. */

    if (out_rows > cinfo->pixel_rows_left)
      out_rows = cinfo->pixel_rows_left;

    (*cinfo->color_convert) (cinfo, (int) out_rows, cinfo->fullsize_data);

    if (cinfo->quantize_colors)
      (*cinfo->color_quantize) (cinfo, (int) out_rows, cinfo->fullsize_data);
    
    (*cinfo->put_pixel_rows) (cinfo, (int) out_rows, cinfo->fullsize_data);

    if ((cinfo->pixel_rows_left -= out_rows) == 0) break;
    ++cur;
  } while (--num);
}



/*
 * Decompression pipeline controller used for single-scan files
 * without 2-pass color quantization.
 */

METHODDEF void
simple_dcontroller (decompress_info_ptr cinfo)
{
  long fullsize_width;		/* # of samples per row in full-size buffers */
  long mcu_rows_per_loop;	/* # of MCU rows processed per outer loop */
  long ri;

  /* Compute dimensions of full-size pixel buffers */
  /* Note these are the same whether interleaved or not. */
  fullsize_width = jround_up(cinfo->image_width,
			     (long) cinfo->max_h_samp_factor * DCTSIZE);

  /* Prepare for single scan containing all components */
  mcu_rows_per_loop = per_scan_setup(cinfo);

  cinfo->total_passes++;

  /* Allocate working memory: */
  /* coeff_data holds a single MCU of coefficient blocks */
  alloc_MCU_coeff(cinfo);
  /* sampled_data is sample data before upsampling */
  alloc_sampling_buffer(cinfo);
  /* fullsize_data is sample data after upsampling */
  alloc_fullsize_buffer(cinfo, fullsize_width);

  prepare_range_limit_table(cinfo);

  /* Initialize to read scan data */

  (*cinfo->entropy_decode_init) (cinfo);
  (*cinfo->dequant_init) (cinfo);

  /* Loop over scan's data: */
  /* max_v_samp_factor pixel rows are processed per loop */

  cinfo->pixel_rows_left = cinfo->image_height;

  for (cinfo->MCU_row_counter = 0;
       cinfo->MCU_row_counter < cinfo->MCU_rows_in_scan;) {
    (*cinfo->progress_monitor) (cinfo);

    /* Obtain v_samp_factor block rows of each component in the scan. */
    /* This is a single MCU row if interleaved, multiple MCU rows if not. */
    /* In the noninterleaved case there might be fewer than v_samp_factor */
    /* block rows remaining; if so, pad with copies of the last pixel row */
    /* so that upsampling doesn't have to treat it as a special case. */

    for (ri = 0; ri < mcu_rows_per_loop; ri++) {
      if (cinfo->MCU_row_counter < cinfo->MCU_rows_in_scan) {
	/* OK to actually read an MCU row. */
	for (cinfo->MCU_col_counter = 0;
	     cinfo->MCU_col_counter < cinfo->MCU_cols_in_scan;
	     cinfo->MCU_col_counter++) {
	  /* Pre-zero the target area to speed up entropy decoder */
	  /* (we assume wholesale zeroing is faster than retail) */
	  zero_MCU_coeff(cinfo);

	  (*cinfo->entropy_decode) (cinfo);

	  (*cinfo->disassemble_MCU) (cinfo, ri);
	}
	/* Realign the sampling buffers */
	realign_sampling_buffer(cinfo, ri);
      } else {
	/* Need to pad out with copies of the last downsampled row. */
	/* This can only happen if there is just one component. */
	duplicate_row(cinfo->first_comp_in_scan->sampled_data,
		      cinfo->first_comp_in_scan->downsampled_width,
		      (int) ri * DCTSIZE - 1, DCTSIZE);
      }
      cinfo->MCU_row_counter++;
    }

    /* Upsample the data */
    /* First time through is a special case */

    if (cinfo->MCU_row_counter == mcu_rows_per_loop) {
      /* Emit first row groups with dummy above-context */
      emit_groups(cinfo, fullsize_width, 0, DCTSIZE-1);
      /* After the first MCU row, */
      /* change wraparound pointers to normal state */
      set_wraparound_pointers(cinfo);
    } else {
      /* Emit last row group of previous set */
      emit_groups(cinfo, fullsize_width, DCTSIZE+1, 1);
      /* Emit first through next-to-last row groups of this set */
      emit_groups(cinfo, fullsize_width, 0, DCTSIZE-1);
    }

    /* Switch to other downsampled-data buffer */
    swap_sampling_buffer(cinfo);

  } /* end of outer loop */

  if (cinfo->pixel_rows_left) {
    /* Emit the last row group with dummy below-context */
    set_bottom_pointers(cinfo);
    emit_groups(cinfo, fullsize_width, DCTSIZE+1, 1);
  }

  /* Clean up after the scan */
  (*cinfo->entropy_decode_term) (cinfo);
  (*cinfo->read_scan_trailer) (cinfo);
  cinfo->completed_passes++;

  /* Verify that we've seen the whole input file */
  if (cinfo->comps_in_scan == cinfo->num_components &&
      (*cinfo->read_scan_header) (cinfo))
    WARNMS(cinfo->emethods, "Didn't expect more than one scan");

  /* Release working memory */
  /* (no work -- we let free_all release what's needful) */
}


/*
 * Decompression pipeline controller used for multiple-scan files
 * and/or 2-pass color quantization.
 */

METHODDEF void
complex_dcontroller (decompress_info_ptr cinfo)
{
  long fullsize_width;		/* # of samples per row in full-size buffers */
  long mcu_rows_per_loop;	/* # of MCU rows processed per outer loop */
  long ri;
  short i;
  jpeg_component_info *compptr;

  /* Compute dimensions of full-size pixel buffers */
  /* Note these are the same whether interleaved or not. */
  fullsize_width = jround_up(cinfo->image_width,
			     (long) cinfo->max_h_samp_factor * DCTSIZE);

  for (ri = 0, compptr = cinfo->first_comp_in_scan; compptr;
       ri++, compptr = compptr->next_comp_in_scan)
    (jpeg_component_info *) cinfo->comp_info[ri].quant_val = compptr;

  for (ri = 0, compptr = (jpeg_component_info *) &cinfo->first_comp_in_scan;
       (short) ri < cinfo->num_components;
       ri++, compptr = compptr->next_comp_in_scan)
    compptr->next_comp_in_scan = cinfo->comp_info + ri;
  compptr->next_comp_in_scan = 0;

  per_scan_setup(cinfo);

  /* Allocate working memory: */
  /* coeff_data holds a full frame of coefficient blocks */
  alloc_full_coeff(cinfo);
  /* sampled_data is sample data before upsampling */
  alloc_sampling_buffer(cinfo);
  /* fullsize_data is sample data after upsampling */
  alloc_fullsize_buffer(cinfo, fullsize_width);

  prepare_range_limit_table(cinfo);

  for (ri = 0, compptr = (jpeg_component_info *) &cinfo->first_comp_in_scan;
       (short) ri < cinfo->comps_in_scan;
       ri++, compptr = compptr->next_comp_in_scan)
    compptr->next_comp_in_scan =
      (jpeg_component_info *) cinfo->comp_info[ri].quant_val;
  compptr->next_comp_in_scan = 0;

  /* Account for passes needed (color quantizer adds its passes separately).
   * If multiscan file, we guess that each component has its own scan,
   * and increment completed_passes by the number of components in the scan.
   */

  cinfo->total_passes += cinfo->num_components	/* guessed # of scans */
			 + 1;			/* count output pass */

  /* Loop over scans in file */

  do {
    /* Prepare for this scan */

    per_scan_setup(cinfo);

    /* Initialize to read scan data */

    (*cinfo->entropy_decode_init) (cinfo);
    if (cinfo->progressive_mode == 0)
      (*cinfo->dequant_init) (cinfo);

    /* Loop over scan's data */

    for (cinfo->MCU_row_counter = 0;
	 cinfo->MCU_row_counter < cinfo->MCU_rows_in_scan;
	 cinfo->MCU_row_counter++) {
      (*cinfo->progress_monitor) (cinfo);
      /* OK to actually read an MCU row. */
      for (cinfo->MCU_col_counter = 0;
	   cinfo->MCU_col_counter < cinfo->MCU_cols_in_scan;
	   cinfo->MCU_col_counter++) {
	(*cinfo->entropy_decode) (cinfo);

	/* Realign the full-size buffers */
	for (compptr = cinfo->first_comp_in_scan; compptr;
	     compptr = compptr->next_comp_in_scan) {
	  JBLOCKARRAY coeff_ptr = compptr->coeff_data;
	  for (i = 0; i < compptr->MCU_height; i++)
	    *coeff_ptr++ += compptr->MCU_width;
	}
      }
      /* Realign the full-size buffers */
      for (compptr = cinfo->first_comp_in_scan; compptr;
	   compptr = compptr->next_comp_in_scan) {
	JBLOCKARRAY coeff_ptr = compptr->coeff_data;
	for (i = 0; i < compptr->MCU_height; i++)
#if DCTSIZE == 8
	  *coeff_ptr++ -= compptr->downsampled_width >> 3;
#else
	  *coeff_ptr++ -= compptr->downsampled_width / DCTSIZE;
#endif
	compptr->coeff_data = coeff_ptr;
      }
    } /* end of loop over scan's data */

    /* Realign the full-size buffers */
    for (compptr = cinfo->first_comp_in_scan; compptr;
	 compptr = compptr->next_comp_in_scan)
#if DCTSIZE == 8
      compptr->coeff_data -= compptr->downsampled_height >> 3;
#else
      compptr->coeff_data -= compptr->downsampled_height / DCTSIZE;
#endif

    /* Clean up after the scan */
    (*cinfo->entropy_decode_term) (cinfo);
    (*cinfo->read_scan_trailer) (cinfo);
    cinfo->completed_passes++;

    /* Repeat if there is another scan */
  } while ((*cinfo->read_scan_header) (cinfo));

  for (ri = 0, compptr = (jpeg_component_info *) &cinfo->first_comp_in_scan;
       (short) ri < cinfo->num_components;
       ri++, compptr = compptr->next_comp_in_scan)
    compptr->next_comp_in_scan = cinfo->comp_info + ri;
  compptr->next_comp_in_scan = 0;

  /* Prepare for this scan */

  mcu_rows_per_loop = per_scan_setup(cinfo);

  /* Initialize to read scan data */

  if (cinfo->progressive_mode)
    (*cinfo->dequant_init) (cinfo);

  /* Loop over scan's data: */
  /* max_v_samp_factor pixel rows are processed per loop */

  cinfo->pixel_rows_left = cinfo->image_height;

  for (cinfo->MCU_row_counter = 0;
       cinfo->MCU_row_counter < cinfo->MCU_rows_in_scan;) {
    (*cinfo->progress_monitor) (cinfo);

    /* Obtain v_samp_factor block rows of each component in the scan. */
    /* This is a single MCU row if interleaved, multiple MCU rows if not. */
    /* In the noninterleaved case there might be fewer than v_samp_factor */
    /* block rows remaining; if so, pad with copies of the last pixel row */
    /* so that upsampling doesn't have to treat it as a special case. */

    for (ri = 0; ri < mcu_rows_per_loop; ri++) {
      if (cinfo->MCU_row_counter < cinfo->MCU_rows_in_scan) {
	/* OK to actually read an MCU row. */
	for (cinfo->MCU_col_counter = 0;
	     cinfo->MCU_col_counter < cinfo->MCU_cols_in_scan;
	     cinfo->MCU_col_counter++) {
	  if (cinfo->progressive_mode)
	    (*cinfo->dequantize_MCU) (cinfo);

	  (*cinfo->disassemble_MCU) (cinfo, ri);

	  /* Realign the full-size buffers */
	  for (compptr = cinfo->first_comp_in_scan; compptr;
	       compptr = compptr->next_comp_in_scan) {
	    JBLOCKARRAY coeff_ptr = compptr->coeff_data;
	    for (i = 0; i < compptr->MCU_height; i++)
	      *coeff_ptr++ += compptr->MCU_width;
	  }
	}
	/* Realign the full-size buffers */
	for (compptr = cinfo->first_comp_in_scan; compptr;
	     compptr = compptr->next_comp_in_scan) {
	  JBLOCKARRAY coeff_ptr = compptr->coeff_data;
	  for (i = 0; i < compptr->MCU_height; i++)
#if DCTSIZE == 8
	    *coeff_ptr++ -= compptr->downsampled_width >> 3;
#else
	    *coeff_ptr++ -= compptr->downsampled_width / DCTSIZE;
#endif
	  compptr->coeff_data = coeff_ptr;
	}
	/* Realign the sampling buffers */
	realign_sampling_buffer(cinfo, ri);
      } else {
	/* Need to pad out with copies of the last downsampled row. */
	/* This can only happen if there is just one component. */
	duplicate_row(cinfo->first_comp_in_scan->sampled_data,
		      cinfo->first_comp_in_scan->downsampled_width,
		      (int) ri * DCTSIZE - 1, DCTSIZE);
      }
      cinfo->MCU_row_counter++;
    }

    /* Upsample the data */
    /* First time through is a special case */

    if (cinfo->MCU_row_counter == mcu_rows_per_loop) {
      /* Emit first row groups with dummy above-context */
      emit_groups(cinfo, fullsize_width, 0, DCTSIZE-1);
      /* After the first MCU row, */
      /* change wraparound pointers to normal state */
      set_wraparound_pointers(cinfo);
    } else {
      /* Emit last row group of previous set */
      emit_groups(cinfo, fullsize_width, DCTSIZE+1, 1);
      /* Emit first through next-to-last row groups of this set */
      emit_groups(cinfo, fullsize_width, 0, DCTSIZE-1);
    }

    /* Switch to other downsampled-data buffer */
    swap_sampling_buffer(cinfo);

  } /* end of outer loop */

  if (cinfo->pixel_rows_left) {
    /* Emit the last row group with dummy below-context */
    set_bottom_pointers(cinfo);
    emit_groups(cinfo, fullsize_width, DCTSIZE+1, 1);
  }

  /* Clean up after the scan */
  cinfo->completed_passes++;

  /* Release working memory */
  /* (no work -- we let free_all release what's needful) */
}


/*
 * The method selection routine for decompression pipeline controllers.
 * Note that at this point we've already read the JPEG header and first SOS,
 * so we can tell whether the input is one scan or not.
 */

GLOBAL void
jseldpipeline (decompress_info_ptr cinfo)
{
  cinfo->d_pipeline_controller =
    cinfo->progressive_mode ||
    cinfo->comps_in_scan != cinfo->num_components &&
    (cinfo->jpeg_color_space != CS_YCbCr ||
     cinfo->out_color_space != CS_GRAYSCALE ||
     cinfo->first_comp_in_scan != cinfo->comp_info)
    /* it's a multiple-scan file */
  ? complex_dcontroller
    /* it's a single-scan file */
  : simple_dcontroller;
}
