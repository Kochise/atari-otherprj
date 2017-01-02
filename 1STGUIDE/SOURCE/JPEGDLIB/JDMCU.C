/*
 * jdmcu.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains MCU disassembly and IDCT control routines.
 * These routines are invoked via the disassemble_MCU, reverse_DCT, and
 * disassemble_init/term methods.
 */

#include "jinclude.h"


/*
 * Perform inverse DCT on each block in an MCU's worth of data;
 * output the results into a sample array starting at row start_row.
 * NB: start_row can only be nonzero when dealing with a single-component
 * scan; otherwise we'd have to pass different offsets for different
 * components, since the heights of interleaved MCU rows can vary.
 * But the pipeline controller logic is such that this is not necessary.
 */

METHODDEF void
disassemble_MCU (decompress_info_ptr cinfo, long start_row)
{
  long ri;
  JBLOCKROW browptr;
  JBLOCKROW bendptr;
  DCTELEM max_val = MAXJSAMPLE;
  DCTELEM cen_val = CENTERJSAMPLE;
  jpeg_component_info *compptr;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan) {
    /* iterate through all blocks in MCU */
    for (ri = 0; (short)ri < compptr->MCU_height; ri++) {
      browptr = compptr->coeff_data[ri];
      bendptr = browptr + compptr->MCU_width;
#ifndef JCOEF_EQU_DCTELEM
      do {
	/* copy the data into a local DCTBLOCK.  This allows for change of
	 * representation (if DCTELEM != JCOEF).  On 80x86 machines it also
	 * brings the data back from FAR storage to NEAR storage.
	 */
	DCTBLOCK block;
	{
	  register DCTELEM *localblkptr = block;
	  register int elem = DCTSIZE2;

	  do *localblkptr++ = (DCTELEM) *((JCOEFPTR)browptr)++;
	  while (--elem > 0);
	}

	j_rev_dct(block, block + 1);	/* perform inverse DCT */
	{
	  register DCTELEM *localblkptr = block;
#else
#define localblkptr ((DCTELEM *)browptr)
      j_rev_dct(localblkptr, (DCTELEM *)bendptr); /* perform inverse DCT */
      do {
	{
#endif
	/* Output the data into the sample array.
	 * Note change from signed to unsigned representation:
	 * DCT calculation works with values +-CENTERJSAMPLE,
	 * but sample arrays always hold 0..MAXJSAMPLE.
	 * We have to do range-limiting because of
	 * quantization errors in the DCT/IDCT phase.
	 */
	  register int elemr = DCTSIZE;
	  register JSAMPARRAY srowptr = compptr->sampled_data +
					(start_row + ri) * DCTSIZE;
	  do
	  { register JSAMPROW elemptr = *srowptr;
	    register DCTELEM val;
#if DCTSIZE == 8		/* unroll the inner loop */
	    if ((val = cen_val + *localblkptr++) < 0) val = 0;
	    if (val > max_val) val = max_val;
	    *elemptr++ = val;
	    if ((val = cen_val + *localblkptr++) < 0) val = 0;
	    if (val > max_val) val = max_val;
	    *elemptr++ = val;
	    if ((val = cen_val + *localblkptr++) < 0) val = 0;
	    if (val > max_val) val = max_val;
	    *elemptr++ = val;
	    if ((val = cen_val + *localblkptr++) < 0) val = 0;
	    if (val > max_val) val = max_val;
	    *elemptr++ = val;
	    if ((val = cen_val + *localblkptr++) < 0) val = 0;
	    if (val > max_val) val = max_val;
	    *elemptr++ = val;
	    if ((val = cen_val + *localblkptr++) < 0) val = 0;
	    if (val > max_val) val = max_val;
	    *elemptr++ = val;
	    if ((val = cen_val + *localblkptr++) < 0) val = 0;
	    if (val > max_val) val = max_val;
	    *elemptr++ = val;
	    if ((val = cen_val + *localblkptr++) < 0) val = 0;
	    if (val > max_val) val = max_val;
	    *elemptr++ = val;
#else
	    register int elemc = DCTSIZE;
	    do {
	      if ((val = cen_val + *localblkptr++) < 0) val = 0;
	      if (val > max_val) val = max_val;
	      *elemptr++ = val;
	    } while (--elemc > 0);
#endif
	    *srowptr++ = elemptr;
	  } while (--elemr > 0);
	}
      } while (browptr < bendptr);
    }
  }
}


METHODDEF void
dequantize_MCU (decompress_info_ptr cinfo)
{
  jpeg_component_info *compptr;
  JBLOCKARRAY coeff_data;
  UINT16 *quanttbl;
  JBLOCKROW block;
  short w, h;
  long k;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan) {
    coeff_data = compptr->coeff_data;
    quanttbl = compptr->quant_val;
    h = compptr->MCU_height;
    do {
      block = *coeff_data++;
      w = compptr->MCU_width;
      do {
	k = (1 - DCTSIZE2) * 2;
	do
	  *(short *)((char *)block + (DCTSIZE2 - 1) * 2 + k) *=
	    *(short *)((char *)quanttbl + (DCTSIZE2 - 1) * 2 + k);
	while ((k += 2) <= 0);
	block++;
      } while (--w > 0);
    }
    while (--h > 0);
  }
}


METHODDEF void
dequant_init (decompress_info_ptr cinfo)
{
  jpeg_component_info *compptr;
  JQUANT_TBL *qtblptr;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan) {
    qtblptr = cinfo->first_quant_tbl;
    for (;;) {
      if (qtblptr == NULL)
	ERREXIT(cinfo->emethods, "Use of undefined quantization table");
      if (qtblptr->quant_tbl_id == compptr->quant_tbl_no)
	break;
      qtblptr = qtblptr->next_quant_tbl;
    }
    compptr->quant_val = qtblptr->quantval;
  }
}


/*
 * The method selection routine for MCU disassembly.
 */

GLOBAL void
jseldmcu (decompress_info_ptr cinfo)
{
  cinfo->disassemble_MCU = disassemble_MCU;
  cinfo->dequantize_MCU = dequantize_MCU;
  cinfo->dequant_init = dequant_init;
}
