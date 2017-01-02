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

/* Size increment of extension data buffers. */

#define EXT_BUF_SIZE 1024

/* External declarations for bitstream i/o operations. */
extern const unsigned long BitMask[33];
extern unsigned long get_ext_data( VidStream *vid_stream, char **Ptr );
extern unsigned long next_start_code( VidStream *vid_stream );
extern void get_extra_bit_info( VidStream *vid_stream, char **Ptr );

#define get_bits1(result)                           \
{                                                   \
  if (--vid_stream->bits_left < 0) {                \
    vid_stream->bits_left += 32;                    \
    vid_stream->curBits = *vid_stream->buffer++;    \
    if (--vid_stream->buf_length == 0)              \
      (*vid_stream->correct_underflow)(vid_stream); \
  }                                                 \
  result = 1;                                       \
  result &= vid_stream->curBits >> vid_stream->bits_left; \
}

#define grab_bits1(result)                          \
{                                                   \
  if (--vid_stream->bits_left < 0) {                \
    vid_stream->bits_left += 32;                    \
    vid_stream->curBits = *vid_stream->buffer++;    \
    if (--vid_stream->buf_length == 0)              \
      (*vid_stream->correct_underflow)(vid_stream); \
  }                                                 \
  result = vid_stream->curBits >> vid_stream->bits_left; \
}

#define get_bits(num, result)                       \
{                                                   \
  result = vid_stream->curBits;                     \
  if ((vid_stream->bits_left -= num) < 0) {         \
    result <<= -vid_stream->bits_left;              \
    vid_stream->bits_left += 32;                    \
    {                                               \
      unsigned long temp = *vid_stream->buffer++;   \
      vid_stream->curBits = temp;                   \
      result |= temp >>= vid_stream->bits_left;     \
    }                                               \
    if (--vid_stream->buf_length == 0)              \
      (*vid_stream->correct_underflow)(vid_stream); \
  }                                                 \
  else result >>= vid_stream->bits_left;            \
  result &= BitMask[num];                           \
}

#define grab_bits(num, result)                      \
{                                                   \
  result = vid_stream->curBits;                     \
  if ((vid_stream->bits_left -= num) < 0) {         \
    result <<= -vid_stream->bits_left;              \
    vid_stream->bits_left += 32;                    \
    {                                               \
      unsigned long temp = *vid_stream->buffer++;   \
      vid_stream->curBits = temp;                   \
      result |= temp >>= vid_stream->bits_left;     \
    }                                               \
    if (--vid_stream->buf_length == 0)              \
      (*vid_stream->correct_underflow)(vid_stream); \
  }                                                 \
  else result >>= vid_stream->bits_left;            \
}

#define show_bits(num, result)                      \
{                                                   \
  int temp = -(num);                                \
  result = vid_stream->curBits;                     \
  if ((temp += vid_stream->bits_left) < 0) {        \
    temp = -temp;                                   \
    result <<= temp;                                \
    result |= *vid_stream->buffer >> (32 - temp);   \
  }                                                 \
  else result >>= temp;                             \
  result &= BitMask[num];                           \
}

#define view_bits(num, result)                      \
{                                                   \
  int temp = -(num);                                \
  result = vid_stream->curBits;                     \
  if ((temp += vid_stream->bits_left) < 0) {        \
    temp = -temp;                                   \
    result <<= temp;                                \
    result |= *vid_stream->buffer >> (32 - temp);   \
  }                                                 \
  else result >>= temp;                             \
}

#define view_bits32(result)                         \
{                                                   \
  int temp = vid_stream->bits_left;                 \
  result = vid_stream->curBits << (32 - temp);      \
  result |= *vid_stream->buffer >> temp;            \
}

#define flush_bits(num)                             \
{                                                   \
  if ((vid_stream->bits_left -= num) < 0) {         \
    vid_stream->bits_left += 32;                    \
    vid_stream->curBits = *vid_stream->buffer++;    \
    if (--vid_stream->buf_length == 0)              \
      (*vid_stream->correct_underflow)(vid_stream); \
  }                                                 \
}

#define flush_bits32                                \
{                                                   \
  vid_stream->curBits = *vid_stream->buffer++;      \
  if (--vid_stream->buf_length == 0)                \
    (*vid_stream->correct_underflow)(vid_stream);   \
}
