/*
 * jchuff.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains Huffman entropy encoding routines.
 * These routines are invoked via the methods entropy_encode,
 * entropy_encode_init/term, and entropy_optimize.
 */

#include "jinclude.h"


LOCAL void
fix_huff_tbl (HUFF_TBL * htbl)
/* Compute derived values for a Huffman table */
{
  UINT8 b, * p;
  UINT16 code;
  int i, l;

  /* Figure C.3: generate encoding tables */
  /* These are code and size indexed by symbol value */

  /* Set any codeless symbols to have code length 0;
   * this allows emit_bits to detect any attempt to emit such symbols.
   */
  MEMZERO(htbl->ehufsi, SIZEOF(htbl->ehufsi));

  code = 0; p = htbl->hufftbl; l = 1;
  do {
    if ((b = *p++) != 0) {
      do {
	i = *p++;
	htbl->ehufsi[i] = (char) l;
	htbl->ehufco[i] = code;
	code++;
      } while (--b);
    }
    code <<= 1;
  } while (++l <= 16);
}


/* Outputting bits to the file */

/* Only the right 24 bits of huff_put_buffer are used; the valid bits are
 * left-justified in this part.  At most 16 bits can be passed to emit_bits
 * in one call, and we never retain more than 7 bits in huff_put_buffer
 * between calls, so 24 bits are sufficient.
 */

INLINE
LOCAL void
emit_bits (compress_info_ptr cinfo, UINT16 code, int size)
{
  /* This routine is heavily used, so it's worth coding tightly. */
  register INT32 c = (((INT32) 1) << size) - 1;

  (UINT16)c &= code;		/* Mask off any excess bits from code */

  cinfo->put_bits += size;	/* new number of bits in buffer */

  /* if size is 0, caller used an invalid Huffman table entry */
  if (size == 0)
    ERREXIT(cinfo->emethods, "Missing Huffman code table entry");

  c <<= 24 - cinfo->put_bits;	/* align incoming bits */

  cinfo->put_buffer |= c;	/* and merge with old buffer contents */

  while (cinfo->put_bits >= 8) {
    c = cinfo->put_buffer >> 16;

    do JPUTC(cinfo,c);
    while (++(char)c == 0);	/* need to stuff a zero byte? */

    cinfo->put_buffer <<= 8;
    cinfo->put_bits -= 8;
  }
}


LOCAL void
flush_bits (compress_info_ptr cinfo)
{
  emit_bits(cinfo, (UINT16) 0x7F, 7);	/* fill any partial byte with ones */
  cinfo->put_buffer = 0;		/* and reset bit-buffer to empty */
  cinfo->put_bits = 0;
}



/*
 * Initialize for a Huffman-compressed scan.
 * This is invoked after writing the SOS marker.
 */

METHODDEF void
huff_init (compress_info_ptr cinfo)
{
  long ci;
  jpeg_component_info * compptr;

  /* Initialize variables */
  cinfo->put_buffer = 0;
  cinfo->put_bits = 0;

  for (ci = 0; (short)ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    /* Make sure requested tables are present */
    if ((compptr->dc_tbl_ptr =
	  cinfo->dc_huff_tbl_ptrs[compptr->dc_tbl_no]) == NULL ||
	(compptr->ac_tbl_ptr =
	  cinfo->ac_huff_tbl_ptrs[compptr->ac_tbl_no]) == NULL)
      ERREXIT(cinfo->emethods, "Use of undefined Huffman table");
    /* Compute derived values for Huffman tables */
    /* We may do this more than once for same table, but it's not a big deal */
    fix_huff_tbl(compptr->dc_tbl_ptr);
    fix_huff_tbl(compptr->ac_tbl_ptr);
    /* Initialize DC predictions to 0 */
    compptr->last_comp_val = 0;
  }

  /* Initialize restart counter */
  cinfo->restarts_to_go = cinfo->restart_interval;
}


/*
 * Emit a restart marker & resynchronize predictions.
 */

LOCAL void
emit_restart (compress_info_ptr cinfo)
{
  long ci;

  flush_bits(cinfo);

  (*cinfo->methods->write_restart_marker) (cinfo);

  /* Re-initialize DC predictions to 0 */
  for (ci = 0; (short)ci < cinfo->comps_in_scan; ci++)
    cinfo->cur_comp_info[ci]->last_comp_val = 0;

  /* Reset restart counter */
  cinfo->restarts_to_go = cinfo->restart_interval;
}


/*
 * Encode and output one MCU's worth of Huffman-compressed coefficients.
 */

METHODDEF void
huff_encode (compress_info_ptr cinfo, JBLOCK *MCU_data)
{
  short blkn, ci;
  int temp, temp2, nbits, k;
  JCOEF *block, *last_blk_ptr;
  HUFF_TBL *htbl;
  jpeg_component_info *compptr;
  jpeg_component_info **curcomp;

  block = *MCU_data;

  /* Account for restart interval, emit restart marker if needed */
  if (cinfo->restart_interval) {
    if (cinfo->restarts_to_go == 0)
      emit_restart(cinfo);
    cinfo->restarts_to_go--;
  }

  curcomp = cinfo->cur_comp_info;
  ci = cinfo->comps_in_scan;
  do {
    compptr = *curcomp++;
    blkn = compptr->MCU_blocks;
    do {
      htbl = compptr->dc_tbl_ptr;

      /* Encode the DC coefficient difference per section F.1.2.1 */

      nbits = 0;

      /* Convert DC value to difference, update last_comp_val */
      if ((temp = (int)*block++ - compptr->last_comp_val) != 0) {
	compptr->last_comp_val += temp;

	if ((temp2 = temp) < 0) {
	  temp = -temp;		/* temp is abs value of input */
	  /* For a negative input, want temp2 = bitwise complement of abs(input) */
	  /* This code assumes we are on a two's complement machine */
	  temp2--;
	}

	/* Find the number of bits needed for the magnitude of the coefficient */
	do nbits++; while (temp >>= 1);
      }

      /* Emit the Huffman-coded symbol for the number of bits */
      emit_bits(cinfo, htbl->ehufco[nbits], htbl->ehufsi[nbits]);

      /* Emit that number of bits of the value, if positive, */
      /* or the complement of its magnitude, if negative. */
      if (nbits)		/* emit_bits rejects calls with size 0 */
	emit_bits(cinfo, (UINT16) temp2, nbits);

      htbl = compptr->ac_tbl_ptr;

      /* Encode the AC coefficients per section F.1.2.2 */

      k = DCTSIZE2 - 1;
      last_blk_ptr = block;
      for (;;) {
	if ((temp = (int)*block++) == 0) {
	  if (--k == 0) {
	    /* If the last coef(s) were zero, emit an end-of-block code */
	    emit_bits(cinfo, htbl->ehufco[0], htbl->ehufsi[0]);
	    break;
	  }
	} else {
	  if ((temp2 = temp) < 0) {
	    temp = -temp;	/* temp is abs value of input */
	    /* This code assumes we are on a two's complement machine */
	    temp2--;
	  }

	  /* Find the number of bits needed for the magnitude of the coefficient */
	  nbits = 0; do nbits++; while (temp >>= 1);

	  /* temp = run length of zeros */
	  temp = (int)(block - last_blk_ptr) - 1;

	  /* if run length >= 16, must emit special run-length-16 codes (0xF0) */
	  while (temp >= 16) {
	    temp -= 16;
	    emit_bits(cinfo, htbl->ehufco[0xF0], htbl->ehufsi[0xF0]);
	  }

	  /* Emit Huffman symbol for run length / number of bits */
	  temp <<= 4; temp += nbits;
	  emit_bits(cinfo, htbl->ehufco[temp], htbl->ehufsi[temp]);

	  /* Emit that number of bits of the value, if positive, */
	  /* or the complement of its magnitude, if negative. */
	  emit_bits(cinfo, (UINT16) temp2, nbits);

	  if (--k == 0) break;
	  last_blk_ptr = block;
	}
      }
    } while (--blkn > 0);
  } while (--ci > 0);
}




/*
 * Huffman coding optimization.
 *
 * This actually is optimization, in the sense that we find the best possible
 * Huffman table(s) for the given data.  We first scan the supplied data and
 * count the number of uses of each symbol that is to be Huffman-coded.
 * (This process must agree with the code above.)  Then we build an
 * optimal Huffman coding tree for the observed counts.
 */

#ifdef ENTROPY_OPT_SUPPORTED


/* These are static so htest_one_block can find 'em */
static long * dc_count_ptrs[NUM_HUFF_TBLS];
static long * ac_count_ptrs[NUM_HUFF_TBLS];


LOCAL void
gen_huff_coding (compress_info_ptr cinfo, HUFF_TBL *htbl, long freq[])
/* Generate the optimal coding for the given counts */
{
#define MAX_CLEN 32	/* assumed maximum initial code length */
  UINT8 bits[MAX_CLEN];	/* bits[k-1] = # of symbols with code length k */
  short codesize[257];	/* codesize[k] = code length of symbol k */
  short others[257];	/* next symbol in current branch of tree */
  UINT8 * p;
  int c1, c2;
  int i, j;
  long v;

  /* This algorithm is explained in section K.2 of the JPEG standard */

  MEMZERO(bits, SIZEOF(bits));
  MEMZERO(codesize, SIZEOF(codesize));
  for (i = 0; i < 257; i++)
    others[i] = -1;		/* init links to empty */

  freq[256] = 1;		/* make sure there is a nonzero count */
  /* including the pseudo-symbol 256 in the Huffman procedure guarantees
   * that no real symbol is given code-value of all ones, because 256
   * will be placed in the largest codeword category.
   */

  /* Huffman's basic algorithm to assign optimal code lengths to symbols */

  for (;;) {
    /* Find the smallest nonzero frequency, set c1 = its symbol */
    /* In case of ties, take the larger symbol number */
    c1 = -1;
    v = 1000000000L;
    for (i = 0; i <= 256; i++) {
      if (freq[i] && freq[i] <= v) {
	v = freq[i];
	c1 = i;
      }
    }

    /* Find the next smallest nonzero frequency, set c2 = its symbol */
    /* In case of ties, take the larger symbol number */
    c2 = -1;
    v = 1000000000L;
    for (i = 0; i <= 256; i++) {
      if (freq[i] && freq[i] <= v && i != c1) {
	v = freq[i];
	c2 = i;
      }
    }

    /* Done if we've merged everything into one frequency */
    if (c2 < 0)
      break;

    /* Else merge the two counts/trees */
    freq[c1] += freq[c2];
    freq[c2] = 0;

    /* Increment the codesize of everything in c1's tree branch */
    codesize[c1]++;
    while (others[c1] >= 0) {
      c1 = others[c1];
      codesize[c1]++;
    }

    others[c1] = c2;		/* chain c2 onto c1's tree branch */

    /* Increment the codesize of everything in c2's tree branch */
    codesize[c2]++;
    while (others[c2] >= 0) {
      c2 = others[c2];
      codesize[c2]++;
    }
  }

  /* Now count the number of symbols of each code length */
  for (i = 0; i <= 256; i++) {
    if (codesize[i]) {
      /* The JPEG standard seems to think that this can't happen, */
      /* but I'm paranoid... */
      if (codesize[i] > MAX_CLEN)
	ERREXIT(cinfo->emethods, "Huffman code size table overflow");

      bits[codesize[i]-1]++;
    }
  }

  /* JPEG doesn't allow symbols with code lengths over 16 bits, so if the pure
   * Huffman procedure assigned any such lengths, we must adjust the coding.
   * Here is what the JPEG spec says about how this next bit works:
   * Since symbols are paired for the longest Huffman code, the symbols are
   * removed from this length category two at a time.  The prefix for the pair
   * (which is one bit shorter) is allocated to one of the pair; then,
   * skipping the BITS entry for that prefix length, a code word from the next
   * shortest nonzero BITS entry is converted into a prefix for two code words
   * one bit longer.
   */

  for (i = MAX_CLEN-1; i >= 16; i--) {
    while (bits[i] > 0) {
      j = i - 2;		/* find length of new prefix to be used */
      while (bits[j] == 0)
	j--;

      bits[i] -= 2;		/* remove two symbols */
      bits[i-1]++;		/* one goes in this length */
      bits[j+1] += 2;		/* two new symbols in this length */
      bits[j]--;		/* symbol of this length is now a prefix */
    }
  }

  /* Remove the count for the pseudo-symbol 256 from the largest codelength */
  while (bits[i] == 0)		/* find largest codelength still in use */
    i--;
  bits[i]--;

  /* Return a list of the counts and symbols sorted by code length */
  /* It's not real clear to me why we don't need to consider the codelength
   * changes made above, but the JPEG spec seems to think this works.
   */
  p = htbl->hufftbl;
  for (i = 1; i <= 16; i++) {
    *p++ = bits[i-1];
    for (j = 0; j <= 255; j++) {
      if (codesize[j] == i) {
	*p++ = (UINT8) j;
      }
    }
  }
}



/*
 * Trial-encode one MCU's worth of Huffman-compressed coefficients.
 */

LOCAL void
htest_encode (compress_info_ptr cinfo, JBLOCK *MCU_data)
{
  short blkn, ci;
  int temp, nbits, k;
  JCOEF *block, *last_blk_ptr;
  long *counts;
  jpeg_component_info *compptr;
  jpeg_component_info **curcomp;

  block = *MCU_data;

  /* Take care of restart intervals if needed */
  if (cinfo->restart_interval) {
    if (cinfo->restarts_to_go == 0) {
      /* Re-initialize DC predictions to 0 */
      curcomp = cinfo->cur_comp_info;
      ci = cinfo->comps_in_scan;
      do {
	compptr = *curcomp++;
	compptr->last_comp_val = 0;
      } while (--ci > 0);
      /* Update restart state */
      cinfo->restarts_to_go = cinfo->restart_interval;
    }
    cinfo->restarts_to_go--;
  }

  curcomp = cinfo->cur_comp_info;
  ci = cinfo->comps_in_scan;
  do {
    compptr = *curcomp++;
    blkn = compptr->MCU_blocks;
    do {
      counts = dc_count_ptrs[compptr->dc_tbl_no];

      /* Encode the DC coefficient difference per section F.1.2.1 */

      /* Convert DC value to difference, update last_comp_val */
      if ((temp = (int)*block++ - compptr->last_comp_val) != 0) {
	compptr->last_comp_val += temp;

	/* Find the number of bits needed for the magnitude of the coefficient */
	if (temp < 0) temp = -temp;
	do counts++; while (temp >>= 1);
      }

      /* Count the Huffman symbol for the number of bits */
      ++*counts;

      counts = ac_count_ptrs[compptr->ac_tbl_no];

      /* Encode the AC coefficients per section F.1.2.2 */

      k = DCTSIZE2 - 1;
      last_blk_ptr = block;
      for (;;) {
	if ((temp = (int)*block++) == 0) {
	  if (--k == 0) {
	    /* If the last coef(s) were zero, emit an end-of-block code */
	    counts[0]++;
	    break;
	  }
	} else {
	  /* Find the number of bits needed for the magnitude of the coefficient */
	  if (temp < 0) temp = -temp;
	  nbits = 0; do nbits++; while (temp >>= 1);

	  /* temp = run length of zeros */
	  temp = (int)(block - last_blk_ptr) - 1;

	  /* if run length >= 16, must emit special run-length-16 codes (0xF0) */
	  while (temp >= 16) {
	    temp -= 16;
	    counts[0xF0]++;
	  }

	  /* Count Huffman symbol for run length / number of bits */
	  temp <<= 4; temp += nbits;
	  counts[temp]++;

	  if (--k == 0) break;
	  last_blk_ptr = block;
	}
      }
    } while (--blkn > 0);
  } while (--ci > 0);
}



/*
 * Find the best coding parameters for a Huffman-coded scan.
 * When called, the scan data has already been converted to a sequence of
 * MCU groups of quantized coefficients, which are stored in a "big" array.
 * The source_method knows how to iterate through that array.
 * On return, the MCU data is unmodified, but the Huffman tables referenced
 * by the scan components may have been altered.
 */

METHODDEF void
huff_optimize (compress_info_ptr cinfo, MCU_output_caller_ptr source_method)
/* Optimize Huffman-coding parameters (Huffman symbol table) */
{
  int i, tbl;
  HUFF_TBL **htblptr;

  /* Allocate and zero the count tables */
  /* Note that gen_huff_coding expects 257 entries in each table! */

  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    dc_count_ptrs[i] = NULL;
    ac_count_ptrs[i] = NULL;
  }

  for (i = 0; i < cinfo->comps_in_scan; i++) {
    /* Create DC table */
    tbl = cinfo->cur_comp_info[i]->dc_tbl_no;
    if (dc_count_ptrs[tbl] == NULL) {
      dc_count_ptrs[tbl] = (long *) (*cinfo->emethods->alloc_small)
					(257 * SIZEOF(long));
      MEMZERO(dc_count_ptrs[tbl], 257 * SIZEOF(long));
    }
    /* Create AC table */
    tbl = cinfo->cur_comp_info[i]->ac_tbl_no;
    if (ac_count_ptrs[tbl] == NULL) {
      ac_count_ptrs[tbl] = (long *) (*cinfo->emethods->alloc_small)
					(257 * SIZEOF(long));
      MEMZERO(ac_count_ptrs[tbl], 257 * SIZEOF(long));
    }
  }

  /* Initialize DC predictions to 0 */
  for (i = 0; i < cinfo->comps_in_scan; i++)
    cinfo->cur_comp_info[i]->last_comp_val = 0;

  /* Initialize restart stuff */
  cinfo->restarts_to_go = cinfo->restart_interval;

  /* Scan the MCU data, count symbol uses */
  (*source_method) (cinfo, htest_encode);

  /* Now generate optimal Huffman tables */
  for (tbl = 0; tbl < NUM_HUFF_TBLS; tbl++) {
    if (dc_count_ptrs[tbl] != NULL) {
      htblptr = & cinfo->dc_huff_tbl_ptrs[tbl];
      if (*htblptr == NULL)
	*htblptr = (HUFF_TBL *) (*cinfo->emethods->alloc_small) (SIZEOF(HUFF_TBL));
      /* Set sent_table FALSE so updated table will be written to JPEG file. */
      (*htblptr)->sent_table = FALSE;
      /* Compute the optimal Huffman encoding */
      gen_huff_coding(cinfo, *htblptr, dc_count_ptrs[tbl]);
      /* Release the count table */
      (*cinfo->emethods->free_small) ((void *) dc_count_ptrs[tbl]);
    }
    if (ac_count_ptrs[tbl] != NULL) {
      htblptr = & cinfo->ac_huff_tbl_ptrs[tbl];
      if (*htblptr == NULL)
	*htblptr = (HUFF_TBL *) (*cinfo->emethods->alloc_small) (SIZEOF(HUFF_TBL));
      /* Set sent_table FALSE so updated table will be written to JPEG file. */
      (*htblptr)->sent_table = FALSE;
      /* Compute the optimal Huffman encoding */
      gen_huff_coding(cinfo, *htblptr, ac_count_ptrs[tbl]);
      /* Release the count table */
      (*cinfo->emethods->free_small) ((void *) ac_count_ptrs[tbl]);
    }
  }
}


#endif /* ENTROPY_OPT_SUPPORTED */


/*
 * The method selection routine for Huffman entropy encoding.
 */

GLOBAL void
jselchuffman (compress_info_ptr cinfo)
{
#ifdef D_ARITH_CODING_SUPPORTED
  if (cinfo->arith_code) return;
#endif
  cinfo->methods->entropy_encode_init = huff_init;
  cinfo->methods->entropy_encode = huff_encode;
  cinfo->methods->entropy_encode_term = flush_bits;
#ifdef ENTROPY_OPT_SUPPORTED
  cinfo->methods->entropy_optimize = huff_optimize;
  /* The standard Huffman tables are only valid for 8-bit data precision.
   * If the precision is higher, force optimization on so that usable
   * tables will be computed.  This test can be removed if default tables
   * are supplied that are valid for the desired precision.
   */
  if (cinfo->data_precision > 8)
    cinfo->optimize_coding = TRUE;
  if (cinfo->optimize_coding)
    cinfo->total_passes++;	/* one pass needed for entropy optimization */
#endif
}
