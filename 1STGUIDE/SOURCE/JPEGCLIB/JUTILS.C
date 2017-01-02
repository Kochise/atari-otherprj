/*
 * jutils.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains miscellaneous utility routines needed for both
 * compression and decompression.
 * Note we prefix all global names with "j" to minimize conflicts with
 * a surrounding application.
 */

#include "jinclude.h"


GLOBAL long
jround_up (long a, long b)
/* Compute a rounded up to next multiple of b; a >= 0, b > 0 */
{
  long c; a += b; --a; c = a; c -= c % b; return c;
}


GLOBAL void
jcopy_sample_rows (JSAMPARRAY input_array, int source_row,
		   JSAMPARRAY output_array, int dest_row,
		   int num_rows, long num_cols)
/* Copy some rows of samples from one place to another.
 * num_rows rows are copied from input_array[source_row++]
 * to output_array[dest_row++]; these areas should not overlap.
 * The source and destination arrays must be at least as wide as num_cols.
 */
{
  register size_t count = (size_t) (num_cols * SIZEOF(JSAMPLE));

  input_array += source_row;
  output_array += dest_row;

  do MEMCOPY(*output_array++, *input_array++, count);
  while (--num_rows > 0);
}


GLOBAL void
jcopy_block_row (JBLOCKROW input_row, JBLOCKROW output_row, long num_blocks)
/* Copy a row of coefficient blocks from one place to another. */
{
  MEMCOPY(output_row, input_row, num_blocks * (DCTSIZE2 * SIZEOF(JCOEF)));
}


GLOBAL void
jzero_far (void * target, size_t bytestozero)
/* Zero out a chunk of FAR memory. */
/* This might be sample-array data, block-array data, or alloc_medium data. */
{
  MEMZERO(target, bytestozero);
}
