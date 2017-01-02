/*
 * jdhuff.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains Huffman entropy decoding routines.
 * These routines are invoked via the methods entropy_decode
 * and entropy_decode_init/term.
 */

#include "jinclude.h"


/*
 * Code for extracting the next N bits from the input stream.
 * (N never exceeds 15 for JPEG data.)
 * This needs to go as fast as possible!
 *
 * We read source bytes into get_buffer and dole out bits as needed.
 */

static const int bmask[16] =	/* bmask[n] is mask for n rightmost bits */
  { 0, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF,
    0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF };

#define get_bits(n, d, b, cinfo)	\
{					\
  if ((b -= n) < 0)			\
    do {				\
      d <<= 8; JGETC(cinfo, (char)d);	\
      if (++(char)d == 0) {		\
	do JGETC(cinfo, (char)d);	\
	while (++(char)d == 0);		\
	if (--(char)d) goto finish;	\
      }					\
      --(char)d;			\
    } while ((b += 8) < 0);		\
}

#define get_bit1(d, b, cinfo)		\
{					\
  if (--b < 0) {			\
    JGETC(cinfo, (char)d);		\
    if (++(char)d == 0) {		\
      do JGETC(cinfo, (char)d);		\
      while (++(char)d == 0);		\
      if (--(char)d) goto finish;	\
    }					\
    --(char)d;				\
    b += 8;				\
  }					\
}


/* Figure F.16: extract next coded symbol from input stream */

#define huff_DECODE(cinfo, htbl, data, bl, code)	\
{					\
  data &= (1L << bl) - 1; code = 2;	\
  for (;;) {				\
    if (--bl < 0) {			\
      data <<= 8;			\
      if (--code < 0) goto finish;	\
      JGETC(cinfo, (char)data);		\
      if (++(char)data == 0) {		\
	do JGETC(cinfo, (char)data);	\
	while (++(char)data == 0);	\
	if (--(char)data) goto finish;	\
      }					\
      --(char)data;			\
      bl += 8;				\
    }					\
    { INT32 temp = *htbl++;		\
      htbl += temp; temp <<= bl;	\
      if ((data -= temp) < 0) break;	\
    }					\
  }					\
  (UINT8)code = htbl[data >> bl];	\
}


/*
 * Check for a restart marker & resynchronize decoder.
 */

LOCAL void
process_restart (decompress_info_ptr cinfo)
{
  jpeg_component_info *compptr;

  /* Advance past the RSTn marker */
  (*cinfo->read_restart_marker) (cinfo);

  /* Reset DC prediction / EOB run count to 0 */
  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan)
    compptr->last_comp_val = 0;

  /* Reset restart counter */
  cinfo->restarts_to_go = cinfo->restart_interval;
}


/* ZAG[i] is the natural-order position of the i'th element of zigzag order.
 */

const unsigned char ZAG[DCTSIZE2] = {
  0,  1,  8, 16,  9,  2,  3, 10,
 17, 24, 32, 25, 18, 11,  4,  5,
 12, 19, 26, 33, 40, 48, 41, 34,
 27, 20, 13,  6,  7, 14, 21, 28,
 35, 42, 49, 56, 57, 50, 43, 36,
 29, 22, 15, 23, 30, 37, 44, 51,
 58, 59, 52, 45, 38, 31, 39, 46,
 53, 60, 61, 54, 47, 55, 62, 63
};


/*
 * Huffman MCU decoding.
 * Each of these routines decodes and returns one MCU's worth of
 * Huffman-compressed coefficients. 
 * The coefficients are reordered from zigzag order into natural array order,
 * but are not dequantized.
 *
 * The i'th block of the MCU is stored into the block pointed to by
 * MCU_data[i].  WE ASSUME THIS AREA IS INITIALLY ZEROED BY THE CALLER.
 */

/*
 * MCU decoding for DC initial scan (either spectral selection,
 * or first pass of successive approximation).
 */

METHODDEF void
decode_mcu_DC_first (decompress_info_ptr cinfo)
{   
  INT32 data;
  int bl, code, s;
  short ypos, xpos;
  UINT8 *htbl;
  JBLOCKROW block;
  jpeg_component_info *compptr;

  /* Account for restart interval, process restart marker if needed */
  if (cinfo->restart_interval) {
    if (cinfo->restarts_to_go == 0)
      process_restart(cinfo);
    cinfo->restarts_to_go--;
  }

  if ((bl = cinfo->bits_left) < 0) return;
  data = cinfo->get_buffer;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan)
    for (ypos = 0; ypos < compptr->MCU_height; ypos++) {
      block = compptr->coeff_data[ypos];
      xpos = compptr->MCU_width;
      do {
	/* Section F.2.2.1: decode the DC coefficient difference */
	/* Convert DC difference to actual value, update last_dc_val */
	htbl = compptr->dc_huffval;
	huff_DECODE(cinfo, htbl, data, bl, code);

	if ((s = code & 15) != 0) {
	  get_bits(s, data, bl, cinfo);
	  code = (int)(data >> bl) & bmask[s];
	  /* Figure F.12: extend sign bit */
	  if (code <= bmask[s - 1]) code -= bmask[s];
	  compptr->last_comp_val += code;
	}

	/* Scale and output the DC coefficient (assumes ZAG[0] = 0) */
	(*block)[0] = (JCOEF) (compptr->last_comp_val << cinfo->Al);

	block++;
      } while (--xpos > 0);
    }

  finish:
  cinfo->get_buffer = data;
  cinfo->bits_left = bl;
}


/*
 * MCU decoding for AC initial scan (either spectral selection,
 * or first pass of successive approximation).
 */

METHODDEF void
decode_mcu_AC_first (decompress_info_ptr cinfo)
{   
  INT32 data;
  int bl, code, s, k;
  UINT8 *htbl;
  JBLOCKROW block;
  jpeg_component_info *compptr;

  /* Account for restart interval, process restart marker if needed */
  if (cinfo->restart_interval) {
    if (cinfo->restarts_to_go == 0)
      process_restart(cinfo);
    cinfo->restarts_to_go--;
  }

  if ((bl = cinfo->bits_left) < 0) return;
  data = cinfo->get_buffer;

  /* There is always only one block per MCU */
  compptr = cinfo->first_comp_in_scan;

  if (compptr->last_comp_val)	/* if it's a band of zeroes... */
    compptr->last_comp_val--;	/* ...process it now (we do nothing) */
  else {
    block = compptr->coeff_data[0];
    k = cinfo->Ss;
    do {
      htbl = compptr->ac_huffval;
      huff_DECODE(cinfo, htbl, data, bl, code);

      if ((s = code & 15) != 0) {
	get_bits(s, data, bl, cinfo);
	if ((k += code >>= 4) > cinfo->Se) break;
	code = (int)(data >> bl) & bmask[s];
	/* Figure F.12: extend sign bit */
	if (code <= bmask[s - 1]) code -= bmask[s];
	/* Scale coefficient and output in natural (dezigzagged) order */
	(*block)[ZAG[k]] = (JCOEF) (code << cinfo->Al);
      } else {
	code >>= 4;
	if (code != 15) {
	  if (code) {		/* EOBr, run length is 2^r + appended bits */
	    compptr->last_comp_val = 1 << code;
	    get_bits(code, data, bl, cinfo);
	    compptr->last_comp_val += (int)(data >> bl) & bmask[code];
	    compptr->last_comp_val--;	/* this band is processed */
	  }				/* at this moment */
	  break;		/* force end-of-band */
	}
	k += code;
      }
    } while (++k <= cinfo->Se);
  }

  finish:
  cinfo->get_buffer = data;
  cinfo->bits_left = bl;
}


/*
 * MCU decoding for DC successive approximation refinement scan.
 * Note: we assume such scans can be multi-component, although the spec
 * is not very clear on the point.
 */

METHODDEF void
decode_mcu_DC_refine (decompress_info_ptr cinfo)
{   
  INT32 data;
  int bl, p1;
  short ypos, xpos;
  JBLOCKROW block;
  jpeg_component_info *compptr;

  p1 = 1 << cinfo->Al;		/* 1 in the bit position being coded */

  /* Account for restart interval, process restart marker if needed */
  if (cinfo->restart_interval) {
    if (cinfo->restarts_to_go == 0)
      process_restart(cinfo);
    cinfo->restarts_to_go--;
  }

  if ((bl = cinfo->bits_left) < 0) return;
  data = cinfo->get_buffer;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan)
    for (ypos = 0; ypos < compptr->MCU_height; ypos++) {
      block = compptr->coeff_data[ypos];
      xpos = compptr->MCU_width;
      do {
	/* Encoded data is simply the next bit of the two's-complement DC value */
	get_bit1(data, bl, cinfo);
	if (data & (1L << bl))
	  (*block)[0] |= p1;
	  /* Note: since we use |=, repeating the assignment later is safe */

	block++;
      } while (--xpos > 0);
    }

  finish:
  cinfo->get_buffer = data;
  cinfo->bits_left = bl;
}


/*
 * MCU decoding for AC successive approximation refinement scan.
 */

METHODDEF void
decode_mcu_AC_refine (decompress_info_ptr cinfo)
{   
  INT32 data;
  int bl, code, s, k;
  UINT8 *htbl;
  JCOEFPTR thiscoef;
  JBLOCKROW block;
  jpeg_component_info *compptr;
  int p1 = 1 << cinfo->Al;	/* 1 in the bit position being coded */
  int m1 = (-1) << cinfo->Al;	/* -1 in the bit position being coded */

  /* Account for restart interval, process restart marker if needed */
  if (cinfo->restart_interval) {
    if (cinfo->restarts_to_go == 0)
      process_restart(cinfo);
    cinfo->restarts_to_go--;
  }

  if ((bl = cinfo->bits_left) < 0) return;
  data = cinfo->get_buffer;

  /* There is always only one block per MCU */
  compptr = cinfo->first_comp_in_scan;

  block = compptr->coeff_data[0];
  k = cinfo->Ss;

  if (compptr->last_comp_val == 0) {
    do {
      htbl = compptr->ac_huffval;
      huff_DECODE(cinfo, htbl, data, bl, code);

      if ((s = code & 15) != 0) {
	code >>= 4;
	get_bit1(data, bl, cinfo); /* size of new coef should always be 1 */
	if (data & (1L << bl))
	  s = p1;		/* newly nonzero coef is positive */
	else
	  s = m1;		/* newly nonzero coef is negative */
      } else {
	code >>= 4;
	if (code != 15) {	/* EOBr, run length is 2^r + appended bits */
	  compptr->last_comp_val = 1 << code;
	  if (code) {		/* EOBr, r > 0 */
	    get_bits(code, data, bl, cinfo);
	    compptr->last_comp_val += (int)(data >> bl) & bmask[code];
	  }
	  break;		/* rest of block is handled by EOB logic */
	}
	/* note s = 0 for processing ZRL */
      }
      /* Advance over already-nonzero coefs and r still-zero coefs,
       * appending correction bits to the nonzeroes.  A correction bit is 1
       * if the absolute value of the coefficient must be increased.
       */
      do {
	thiscoef = (*block) + ZAG[k++];
	if (*thiscoef) {
	  get_bit1(data, bl, cinfo);
	  if (data & (1L << bl))
	    if (*thiscoef >= 0)
	      *thiscoef += p1;
	    else
	      *thiscoef += m1;
	} else if (--code < 0) {
	  /* Reached target zero coefficient */
	  *thiscoef = (JCOEF) s;
	  break;
	}
      } while (k <= cinfo->Se);
    } while (k <= cinfo->Se);
  }

  if (compptr->last_comp_val) {
    /* Scan any remaining coefficient positions after the end-of-band
     * (the last newly nonzero coefficient, if any).  Append a correction
     * bit to each already-nonzero coefficient.  A correction bit is 1
     * if the absolute value of the coefficient must be increased.
     */
    do {
      thiscoef = (*block) + ZAG[k++];
      if (*thiscoef) {
	get_bit1(data, bl, cinfo);
	if (data & (1L << bl))
	  if (*thiscoef >= 0)
	    *thiscoef += p1;
	  else
	    *thiscoef += m1;
      }
    } while (k <= cinfo->Se);
    compptr->last_comp_val--;	/* count one block completed in EOB run */
  }

  finish:
  cinfo->get_buffer = data;
  cinfo->bits_left = bl;
}


/*
 * Decode and return one MCU's worth of Huffman-compressed coefficients.
 * This routine also handles quantization descaling and zigzag reordering
 * of coefficient values.
 *
 * The i'th block of the MCU is stored into the block pointed to by
 * MCU_data[i].  WE ASSUME THIS AREA HAS BEEN ZEROED BY THE CALLER.
 * (Wholesale zeroing is usually a little faster than retail...)
 */

METHODDEF void
huff_decode_mcu (decompress_info_ptr cinfo)
{
  INT32 data;
  int bl, code, s, k;
  short ypos, xpos;
  UINT8 *htbl;
  JBLOCKROW block;
  jpeg_component_info *compptr;

  /* Account for restart interval, process restart marker if needed */
  if (cinfo->restart_interval) {
    if (cinfo->restarts_to_go == 0)
      process_restart(cinfo);
    cinfo->restarts_to_go--;
  }

  if ((bl = cinfo->bits_left) < 0) return;
  data = cinfo->get_buffer;

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan)
    for (ypos = 0; ypos < compptr->MCU_height; ypos++) {
      block = compptr->coeff_data[ypos];
      xpos = compptr->MCU_width;
      do {
	/* Decode a single block's worth of coefficients */

	/* Section F.2.2.1: decode the DC coefficient difference */
	/* Convert DC difference to actual value, update last_dc_val */
	htbl = compptr->dc_huffval;
	huff_DECODE(cinfo, htbl, data, bl, code);

	if ((s = code & 15) != 0) {
	  get_bits(s, data, bl, cinfo);
	  code = (int)(data >> bl) & bmask[s];
	  /* Figure F.12: extend sign bit */
	  if (code <= bmask[s - 1]) code -= bmask[s];
	  compptr->last_comp_val += code;
	}

	/* Descale and output the DC coefficient (assumes ZAG[0] = 0) */
	(*block)[0] = (JCOEF)
	  (compptr->last_comp_val * compptr->quant_val[0]);

	/* Section F.2.2.2: decode the AC coefficients */
	/* Since zero values are skipped, output area must be zeroed beforehand */
	k = 1 - DCTSIZE2;
	do {
	  htbl = compptr->ac_huffval;
	  huff_DECODE(cinfo, htbl, data, bl, code);

	  if ((s = code & 15) != 0) {
	    get_bits(s, data, bl, cinfo);
	    if ((k += code >>= 4) >= 0) break;
	    code = (int)(data >> bl) & bmask[s];
	    /* Figure F.12: extend sign bit */
	    if (code <= bmask[s - 1]) code -= bmask[s];
	    /* Descale coefficient and output in natural (dezigzagged) order */
	    (*block)[ZAG[DCTSIZE2 + k]] = (JCOEF)
	      (code * compptr->quant_val[ZAG[DCTSIZE2 + k]]);
	  } else {
	    code >>= 4;
	    if (code != 15)
	      break;
	    k += code;
	  }
	} while (++k < 0);

	block++;
      } while (--xpos > 0);
    }

  finish:
  cinfo->get_buffer = data;
  cinfo->bits_left = bl;
}


LOCAL UINT8 *
fix_huff_tbl (decompress_info_ptr cinfo,
	      JHUFF_TBL *htblptr, short huff_tbl_no)
{
  while (htblptr) {
    if (htblptr->huff_tbl_id == huff_tbl_no)
      return htblptr->huffval;
    htblptr = htblptr->next_huff_tbl;
  }
  ERREXIT(cinfo->emethods, "Use of undefined Huffman table");
}


/*
 * Initialize for a Huffman-compressed scan.
 * This is invoked after reading the SOS marker.
 */

METHODDEF void
huff_decoder_init (decompress_info_ptr cinfo)
{
  jpeg_component_info *compptr;

  if (cinfo->progressive_mode) {

    /* Validate scan parameters */
    if (cinfo->Ss == 0) {
      if (cinfo->Se)
	goto bad;
    } else {
      /* need not check Ss/Se < 0 since they came from unsigned bytes */
      if (cinfo->Se < cinfo->Ss || cinfo->Se >= DCTSIZE2)
	goto bad;
      /* AC scans may have only one component */
      if (cinfo->comps_in_scan != 1)
	goto bad;
    }
    if (cinfo->Ah) {
      /* Successive approximation refinement scan: must have Al = Ah-1. */
      if (cinfo->Ah - 1 != cinfo->Al)
	goto bad;
    }
    if (cinfo->Al > 13)	{	/* need not check for < 0 */
      bad:
      ERREXIT4(cinfo->emethods,
	       "Invalid progressive parameters Ss=%d Se=%d Ah=%d Al=%d",
	       cinfo->Ss, cinfo->Se, cinfo->Ah, cinfo->Al);
    }
  }

  for (compptr = cinfo->first_comp_in_scan; compptr;
       compptr = compptr->next_comp_in_scan) {

    /* Make sure requested tables are present */
    if (cinfo->progressive_mode == 0) {
      compptr->dc_huffval =
	fix_huff_tbl(cinfo, cinfo->first_dc_huff_tbl, compptr->dc_tbl_no);
      compptr->ac_huffval =
	fix_huff_tbl(cinfo, cinfo->first_ac_huff_tbl, compptr->ac_tbl_no);
    } else if (cinfo->Ss) {
      compptr->ac_huffval =
	fix_huff_tbl(cinfo, cinfo->first_ac_huff_tbl, compptr->ac_tbl_no);
    } else if (cinfo->Ah == 0) {	/* DC refinement needs no table */
      compptr->dc_huffval =
	fix_huff_tbl(cinfo, cinfo->first_dc_huff_tbl, compptr->dc_tbl_no);
    }

    /* Initialize DC prediction / EOB run count to 0 */
    compptr->last_comp_val = 0;
  }

  /* Select MCU decoding routine */
  cinfo->entropy_decode =
    cinfo->progressive_mode == 0 ? huff_decode_mcu : cinfo->Ah == 0
      ? (cinfo->Ss == 0 ? decode_mcu_DC_first : decode_mcu_AC_first)
      : (cinfo->Ss == 0 ? decode_mcu_DC_refine : decode_mcu_AC_refine);

  /* Initialize restart counter */
  cinfo->restarts_to_go = cinfo->restart_interval;
}


/*
 * Finish up at the end of a Huffman-compressed scan.
 */

METHODDEF void
huff_decoder_term (decompress_info_ptr cinfo)
{
  /* No work needed */
}


/*
 * The method selection routine for Huffman entropy decoding.
 */

GLOBAL void
jseldhuffman (decompress_info_ptr cinfo)
{
#ifdef D_ARITH_CODING_SUPPORTED
  if (cinfo->arith_code) return;
#endif
  cinfo->entropy_decode_init = huff_decoder_init;
  cinfo->entropy_decode_term = huff_decoder_term;
}
