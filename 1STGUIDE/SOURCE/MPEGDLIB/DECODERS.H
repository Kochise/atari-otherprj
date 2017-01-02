/*
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */
/*
 * decoders.h
 *
 * This file contains the declarations of structures required for Huffman
 * decoding
 *
 */

/* Include util.h for bit i/o parsing macros. */

#include "util.h"

/* Code for unbound values in decoding tables */
#define ERROR -1
#define DCT_ERROR 63

#define MACRO_BLOCK_STUFFING 34
#define MACRO_BLOCK_ESCAPE 35

/* Special values for DCT Coefficients */
#define END_OF_BLOCK 63
#define ESCAPE 61

/* Structure for an entry in the decoding table of
 * macroblock_address_increment */
typedef struct {
  int value;		/* value for macroblock_address_increment */
  int num_bits;		/* length of the Huffman code */
} mb_addr_inc_entry;

/* Decoding table for macroblock_address_increment */
extern mb_addr_inc_entry mb_addr_inc[2048];


/* Structure for an entry in the decoding table of macroblock_type */
typedef struct {
  BOOLEAN mb_quant;           /* macroblock_quant */
  BOOLEAN mb_motion_forward;  /* macroblock_motion_forward */
  BOOLEAN mb_motion_backward; /* macroblock_motion_backward */
  BOOLEAN mb_pattern;         /* macroblock_pattern */
  BOOLEAN mb_intra;           /* macroblock_intra */
  int num_bits;               /* length of the Huffman code */
} mb_type_entry;

/* Decoding table for macroblock_type in predictive-coded pictures */
extern mb_type_entry mb_type_P[64];

/* Decoding table for macroblock_type in bidirectionally-coded pictures */
extern mb_type_entry mb_type_B[64];


/* External declaration of coded block pattern table. */

extern const int coded_block_pattern[512];


/* Structure for an entry in the decoding table of motion vectors */
typedef struct {
  int code;		/* value for motion_horizontal_forward_code,
			 * motion_vertical_forward_code,
			 * motion_horizontal_backward_code, or
			 * motion_vertical_backward_code.
			 */
  int num_bits;		/* length of the Huffman code */
} motion_vectors_entry;


/* Decoding table for motion vectors */
extern motion_vectors_entry motion_vectors[2048];


#define DecodeDCTDCSizeLum(val, index)   \
{                                        \
  show_bits(9,index)                     \
  val = (short) index >> 4;              \
  if (((unsigned char)val = dct_dc_size_luminance[val]) == 0) \
    (unsigned char)val = dct_dc_size_luminance1[(short)index - 0x01f0]; \
  flush_bits(val & 15)                   \
}

#define DecodeDCTDCSizeChrom(val, index) \
{                                        \
  show_bits(10,index)                    \
  val = (short) index >> 5;              \
  if (((unsigned char)val = dct_dc_size_chrominance[val]) == 0) \
    (unsigned char)val = dct_dc_size_chrominance1[(short)index - 0x03e0]; \
  flush_bits(val & 15)                   \
}

#define DecodeDCTCoeffFirst(run, level, index) \
{                                        \
  view_bits(16,index)                    \
  level = (short) index;                 \
  (unsigned short)level >>= 8;           \
                                         \
  if ((run = dct_coeff_first[level]) == 0) \
    if (level == 0) {                    \
      run = dct_coeff_tbl_0[(short)index]; \
    }                                    \
    else if (--level == 0) {             \
      (unsigned short)index >>= 4;       \
      run = dct_coeff_tbl_1[(short)index - 16]; \
    }                                    \
    else {                               \
      (unsigned short)index >>= 6;       \
      run = dct_coeff_tbl_2[(short)index - 8]; \
    }                                    \
  flush_bits((run & 15) + 1)             \
  if ((unsigned short)run >>= 4) {       \
    level = run & 0x3f;                  \
    run >>= 6;                           \
    grab_bits1(index)                    \
    if (index & 1) level = -level;       \
  }                                      \
  else {  /* ESCAPE */                   \
    get_bits(14,index)                   \
    level = (char) index;                \
    run = (short) index >> 8;            \
    if (((char)index <<= 1) == 0) {      \
      grab_bits(8,index)                 \
      (char)level = (char)index;         \
    }                                    \
  }                                      \
}

#define DecodeDCTCoeffNext(run, level, index, count) \
{                                        \
  view_bits(16,index)                    \
  level = (short) index;                 \
  (unsigned short)level >>= 8;           \
                                         \
  if ((run = dct_coeff_next[level]) == 0) \
    if (level == 0) {                    \
      run = dct_coeff_tbl_0[(short)index]; \
    }                                    \
    else if (--level == 0) {             \
      (unsigned short)index >>= 4;       \
      run = dct_coeff_tbl_1[(short)index - 16]; \
    }                                    \
    else {                               \
      (unsigned short)index >>= 6;       \
      run = dct_coeff_tbl_2[(short)index - 8]; \
    }                                    \
  flush_bits((run & 15) + 1)             \
  if ((unsigned short)run >>= 4) {       \
    level = run & 0x3f;                  \
    run >>= 6;                           \
    count += run; if (count > 63) break; \
    grab_bits1(index)                    \
    if (index & 1) level = -level;       \
  }                                      \
  else {  /* ESCAPE */                   \
    get_bits(14,index)                   \
    level = (char) index;                \
    run = (short) index >> 8;            \
    if (((char)index <<= 1) == 0) {      \
      grab_bits(8,index)                 \
      (char)level = (char)index;         \
    }                                    \
    count += run; if (count > 63) break; \
  }                                      \
}

/*
 *--------------------------------------------------------------
 *
 * DecodeCBP --
 *
 *	Huffman Decoder for coded_block_pattern; location in which the
 *      decoded result will be placed is being passed as argument. The
 *      decoded values are obtained by doing a table lookup on
 *      coded_block_pattern.
 *
 * Results:
 *	The decoded value for coded_block_pattern or ERROR for unbound
 *      values will be placed in the location specified.
 *
 * Side effects:
 *	Bit stream is irreversibly parsed.
 *
 *--------------------------------------------------------------
 */
#define DecodeCBP(ptr, data)             \
{                                        \
  show_bits(9,data)                      \
  ptr = coded_block_pattern[data];       \
  flush_bits(ptr & 15)                   \
  ptr >>= 4;                             \
}

/*
 *--------------------------------------------------------------
 *
 * DecodeMBTypeI --
 *
 *	Huffman Decoder for macro_block_type in intra-coded pictures;
 *      locations in which the decoded results: macroblock_quant,
 *      macroblock_motion_forward, macro_block_motion_backward,
 *      macroblock_pattern, macro_block_intra, will be placed are
 *      being passed as argument.
 *
 * Results:
 *	The various decoded values for macro_block_type in intra-coded
 *      pictures or ERROR for unbound values will be placed in the
 *      locations specified.
 *
 * Side effects:
 *	Bit stream is irreversibly parsed.
 *
 *--------------------------------------------------------------
 */
#define DecodeMBTypeI(data,quant,motion_forw,motion_back,pattern,intra) \
{                    \
  show_bits(2,data)  \
  motion_forw = 0;   \
  motion_back = 0;   \
  pattern = 0;       \
  intra = 1;         \
  if (--data == 0) { \
    quant = 1;       \
    flush_bits(2)    \
  }                  \
  else if (++data) { \
    quant = 0;       \
    flush_bits(1)    \
  }                  \
  else               \
    quant = ERROR;   \
}

/*
 *--------------------------------------------------------------
 *
 * DecodeMBTypeP --
 *
 *	Huffman Decoder for macro_block_type in predictive-coded pictures;
 *      locations in which the decoded results: macroblock_quant,
 *      macroblock_motion_forward, macro_block_motion_backward,
 *      macroblock_pattern, macro_block_intra, will be placed are
 *      being passed as argument. The decoded values are obtained by
 *      doing a table lookup on mb_type_P.
 *
 * Results:
 *	The various decoded values for macro_block_type in
 *      predictive-coded pictures or ERROR for unbound values will be
 *      placed in the locations specified.
 *
 * Side effects:
 *	Bit stream is irreversibly parsed.
 *
 *--------------------------------------------------------------
 */
#define DecodeMBTypeP(data,quant,motion_forw,motion_back,pattern,intra) \
{                                                   \
  show_bits(6,data)                                 \
  quant = mb_type_P[data].mb_quant;                 \
  motion_forw = mb_type_P[data].mb_motion_forward;  \
  motion_back = mb_type_P[data].mb_motion_backward; \
  pattern = mb_type_P[data].mb_pattern;             \
  intra = mb_type_P[data].mb_intra;                 \
  flush_bits(mb_type_P[data].num_bits)              \
}

/*
 *--------------------------------------------------------------
 *
 * DecodeMBTypeB --
 *
 *	Huffman Decoder for macro_block_type in bidirectionally-coded
 *      pictures;locations in which the decoded results: macroblock_quant,
 *      macroblock_motion_forward, macro_block_motion_backward,
 *      macroblock_pattern, macro_block_intra, will be placed are
 *      being passed as argument. The decoded values are obtained by
 *      doing a table lookup on mb_type_B.
 *
 * Results:
 *	The various decoded values for macro_block_type in
 *      bidirectionally-coded pictures or ERROR for unbound values will
 *      be placed in the locations specified.
 *
 * Side effects:
 *	Bit stream is irreversibly parsed.
 *
 *--------------------------------------------------------------
 */
#define DecodeMBTypeB(data,quant,motion_forw,motion_back,pattern,intra) \
{                                                   \
  show_bits(6,data)                                 \
  quant = mb_type_B[data].mb_quant;                 \
  motion_forw = mb_type_B[data].mb_motion_forward;  \
  motion_back = mb_type_B[data].mb_motion_backward; \
  pattern = mb_type_B[data].mb_pattern;             \
  intra = mb_type_B[data].mb_intra;                 \
  flush_bits(mb_type_B[data].num_bits)              \
}

/*
 *--------------------------------------------------------------
 *
 * DecodeMBAddrInc --
 *
 *      Huffman Decoder for macro_block_address_increment; the location
 *      in which the result will be placed is being passed as argument.
 *      The decoded value is obtained by doing a table lookup on
 *      mb_addr_inc.
 *
 * Results:
 *      The decoded value for macro_block_address_increment or ERROR
 *      for unbound values will be placed in the location specified.
 *
 * Side effects:
 *      Bit stream is irreversibly parsed.
 *
 *--------------------------------------------------------------
 */

#define DecodeMBAddrInc(ptr, data)       \
{				         \
  show_bits(11,data)		         \
  ptr = mb_addr_inc[data].value;         \
  flush_bits(mb_addr_inc[data].num_bits) \
}

/*
 *--------------------------------------------------------------
 *
 * DecodeMotionVectors --
 *
 *      Huffman Decoder for the various motion vectors, including
 *      motion_horizontal_forward_code, motion_vertical_forward_code,
 *      motion_horizontal_backward_code, motion_vertical_backward_code.
 *      Location where the decoded result will be placed is being passed
 *      as argument. The decoded values are obtained by doing a table
 *      lookup on motion_vectors.
 *
 * Results:
 *      The decoded value for the motion vector or ERROR for unbound
 *      values will be placed in the location specified.
 *
 * Side effects:
 *      Bit stream is irreversibly parsed.
 *
 *--------------------------------------------------------------
 */

#define DecodeMotionVectors(ptr, data)      \
{                                           \
  show_bits(11,data)                        \
  ptr = motion_vectors[data].code;          \
  flush_bits(motion_vectors[data].num_bits) \
}
