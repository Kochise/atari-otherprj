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
#include <string.h>

#include "video.h"
#include "util.h"

#define IGNORE_EXTRA_DATA


/*
 *--------------------------------------------------------------
 *
 * next_start_code --
 *
 *	Parses off bitstream until start code reached. When done
 *      next 4 bytes of bitstream will be start code. Bit offset
 *      reset to 0.
 *
 * Results:
 *	Start code.
 *
 * Side effects:
 *	Bit stream irreversibly parsed.
 *
 *--------------------------------------------------------------
 */

unsigned long next_start_code( VidStream *vid_stream )
{
  unsigned long data;

  /* Align buffer to next byte. */

  vid_stream->bits_left &= ~7;

  for (;;) {
    show_bits(24,data)
    if (--data == 0) break;
    flush_bits(8)
  }

  view_bits32(data)
  return data;
}


#pragma warn -par
/*
 *--------------------------------------------------------------
 *
 * get_ext_data --
 *
 *	Assumes that bit stream is at begining of extension
 *      data. Parses off extension data into dynamically
 *      allocated space until start code is hit.
 *
 * Results:
 *	Pointer to dynamically allocated memory containing
 *      extension data.
 *
 * Side effects:
 *	Bit stream irreversibly parsed.
 *
 *--------------------------------------------------------------
 */

unsigned long get_ext_data( VidStream *vid_stream, char **Ptr )
{
#ifdef IGNORE_EXTRA_DATA
  /* Flush off start code. */
  flush_bits32

  return next_start_code(vid_stream);
#else
  unsigned long data;
  long size, marker;
  char *dataPtr, *newPtr;

  /* Flush off start code. */
  flush_bits32

  /* Free old buffer. */

  if (*Ptr) (*vid_stream->free)(vid_stream, *Ptr);

  /* Initialize size of ext data buffer and allocate. */

  size = EXT_BUF_SIZE;
  dataPtr = ex_alloc(vid_stream, size);

  /* Initialize marker to keep place in ext data buffer. */

  marker = 0;

  /* While next data is not start code... */

  for (;;)
  {
    show_bits(24,data)
    if (--data == 0) break;

    /* Get next byte of ext data. */

    get_bits(8,data);

    /* Put ext data into ext data buffer. Advance marker. */

    dataPtr[marker++] = (char) data;

    /* If end of ext data buffer reached, resize data buffer. */

    if (marker == size) {
      newPtr = ex_alloc(vid_stream, size + EXT_BUF_SIZE);
      memcpy(newPtr, dataPtr, size);
      (*vid_stream->free)(vid_stream, dataPtr);
      size += EXT_BUF_SIZE;
      dataPtr = newPtr;
    }
  }

  /* Realloc data buffer to free any extra space. */
  /* Return pointer to ext data buffer. */
  /* return realloc(dataPtr, marker); */

  *Ptr = dataPtr;

  view_bits32(data)
  return data;
#endif
}
#pragma warn +par


#pragma warn -par
/*
 *--------------------------------------------------------------
 *
 * get_extra_bit_info --
 *
 *	Parses off extra bit info stream into dynamically
 *      allocated memory. Extra bit info is indicated by
 *      a flag bit set to 1, followed by 8 bits of data.
 *      This continues until the flag bit is zero. Assumes
 *      that bit stream set to first flag bit in extra
 *      bit info stream.
 *
 * Results:
 *	Pointer to dynamically allocated memory with extra
 *      bit info in it. Flag bits are NOT included.
 *
 * Side effects:
 *	Bit stream irreversibly parsed.
 *
 *--------------------------------------------------------------
 */

void get_extra_bit_info( VidStream *vid_stream, char **Ptr )
{
  unsigned long data;
#ifdef IGNORE_EXTRA_DATA

  for (;;) {
    get_bits1(data)
    if (data == 0) break;
    flush_bits(8)
  }
#else
  long size, marker;
  char *dataPtr, *newPtr;

  /* Free old buffer. */

  if (*Ptr) (*vid_stream->free)(vid_stream, *Ptr);

  /* Get first flag bit. */
  get_bits1(data)

  /* If flag is false, return NULL pointer (i.e. no extra bit info). */

  if (data == 0) { *Ptr = 0; return; }

  /* Initialize size of extra bit info buffer and allocate. */

  size = EXT_BUF_SIZE;
  dataPtr = ex_alloc(vid_stream, size);

  /* Reset marker to hold place in buffer. */

  marker = 0;

  /* While flag bit is true. */

  do {

    /* Get next 8 bits of data. */
    get_bits(8,data)

    /* Place in extra bit info buffer. */

    dataPtr[marker++] = (char) data;

    /* If buffer is full, reallocate. */

    if (marker == size) {
      newPtr = ex_alloc(vid_stream, size + EXT_BUF_SIZE);
      memcpy(newPtr, dataPtr, size);
      (*vid_stream->free)(vid_stream, dataPtr);
      size += EXT_BUF_SIZE;
      dataPtr = newPtr;
    }

    /* Get next flag bit. */
    get_bits1(data)
  }
  while (data);

  /* Reallocate buffer to free extra space. */
  /* Return pointer to extra bit info buffer. */
  /* return realloc(dataPtr, marker); */

  *Ptr = dataPtr;
#endif
}
#pragma warn +par

static void Fsysread( VidStream *vid_stream )
{
  static long sys_end_mark[1] = { ISO_11172_END_CODE };
  long count;

  if (vid_stream->EOF_flag) goto eof;

  if ((count = (*vid_stream->get_more_data)
	(vid_stream, &vid_stream->sys_buf)) <= 0) {
    vid_stream->EOF_flag++;
    /*
       Make 32 bits equal to sys end code
       in order to prevent messy data
       from infinite recursion.
    */
    eof:
    vid_stream->sys_buf = (unsigned char *)sys_end_mark;
    count = 4;
  }
  vid_stream->bytes_left += count;
}

#define MAKESTMT(stuff)	  do { stuff } while (0)

#define FGETC(vs, dest)   \
  MAKESTMT( if (--(vs)->bytes_left < 0) Fsysread(vs); \
	    dest = *(vs)->sys_buf++; )

#define FSKIPC(vs)   \
  MAKESTMT( if (--(vs)->bytes_left < 0) Fsysread(vs); \
	    (vs)->sys_buf++; )

/*
 *-----------------------------------------------------------
 *
 *  ReadStartCode
 *
 *      Parses a start code out of the stream
 *
 *  Results:  start code
 *
 *-----------------------------------------------------------
 */
static int ReadStartCode( VidStream *vid_stream )
{
  long startCode;

  FGETC(vid_stream, (char)startCode); startCode <<= 8;
  FGETC(vid_stream, (char)startCode); startCode <<= 8;
  do {
    FGETC(vid_stream, (char)startCode); startCode <<= 8;
  }
  while (startCode != START_CODE_PREFIX);
  FGETC(vid_stream, (char)startCode);
  return (int)startCode;
}

/*
 *-----------------------------------------------------------------
 *
 *  ReadPackHeader
 *
 *      Parses out the PACK header
 *
 *-------------------------------------------------------------------
 */
static void ReadPackHeader( VidStream *vid_stream )
{
  int numRead;

  numRead = PACK_HEADER_SIZE;
  do FSKIPC(vid_stream);
  while (--numRead);
}

/*
 *------------------------------------------------------------------
 *
 *   ReadSystemHeader
 *
 *      Parse out the system header, setup out stream IDs for parsing packets
 *
 *   Results:  Sets AudioStreamID and VideoStreamID in vid_stream
 *
 *------------------------------------------------------------------
 */
static void ReadSystemHeader( VidStream *vid_stream )
{
  long headerSize;
  int i, streamID;

  FGETC(vid_stream, headerSize); headerSize <<= 8;
  FGETC(vid_stream, (char)headerSize);
  i = SYSTEM_HEADER_SIZE;
  while (--i >= 0 && --headerSize >= 0)
    FSKIPC(vid_stream);
  while (--headerSize >= 0) {
    FGETC(vid_stream, streamID);
    if ((char)streamID >= 0) break;
    if (--headerSize >= 0) FSKIPC(vid_stream);
    if (--headerSize >= 0) FSKIPC(vid_stream);
    switch (streamID >> 4) {
    case 0xc:
    case 0xd:
      vid_stream->AudioStreamID = streamID;
      break;
    case 0xe:
      if (vid_stream->VideoStreamID == 0)
	vid_stream->VideoStreamID = streamID;
      break;
    }
  }
  while (--headerSize >= 0) FSKIPC(vid_stream);
}

/*
 *-----------------------------------------------------------------
 *
 *  ReadPacket
 *
 *      Reads a single packet out of the stream, and puts it in the
 *      buffer if it is video.
 *
 *  Results:  Returns: 0 - not video packet we want
 *		       1 - got video packet into buffer
 *
 *-----------------------------------------------------------------
 */
static int ReadPacket( VidStream *vid_stream, int packetID )
{
  unsigned char nextByte;
  unsigned short packetLength;

  FGETC(vid_stream, (char)packetLength); packetLength <<= 8;
  FGETC(vid_stream, (char)packetLength);
  if (packetID != vid_stream->VideoStreamID) {
    if ((packetID >> 4) != 0xe || vid_stream->VideoStreamID != 0) {
      do FSKIPC(vid_stream);
      while (--packetLength);
      return 0;
    }
    vid_stream->VideoStreamID = packetID;
  }
  do {
    packetLength--;
    FGETC(vid_stream, nextByte);
  }
  while ((char)nextByte < 0);
  if ((nextByte >> 6) == 0x01) {
    packetLength -= 2;
    FSKIPC(vid_stream);
    FGETC(vid_stream, nextByte);
  }
  nextByte >>= 4;
  if (nextByte == 0x02) {
    packetLength -= 4;
    nextByte = 4; do FSKIPC(vid_stream); while (--nextByte);
  }
  else if (nextByte == 0x03) {
    packetLength -= 9;
    nextByte = 9; do FSKIPC(vid_stream); while (--nextByte);
  }
  vid_stream->packet_length = packetLength;
  return 1;
}

/*
 *----------------------------------------------------------
 *
 *  read_sys
 *
 *      Parse out a packet of the system layer MPEG file.
 *
 *  Results:  Returns 0 if error or EOF
 *            Returns 1 if more data read
 *
 *----------------------------------------------------------
 */
static int read_sys( VidStream *vid_stream )
{
  int startCode;

  for (;;) {
    startCode = ReadStartCode(vid_stream);
    if (startCode == ISO_11172_END_CODE) break;
    if (startCode == PACK_START_CODE)
      ReadPackHeader(vid_stream);
    else if (startCode == SYSTEM_HEADER_START_CODE)
      ReadSystemHeader(vid_stream);
    else if (ReadPacket(vid_stream, startCode & 0xff)) return 1;
  }
  return 0;
}

/*
 *--------------------------------------------------------------
 *
 * correct_underflow --
 *
 *	Called when buffer does not have sufficient data to
 *      satisfy request for bits.
 *      Fills the buffer with more data.
 *
 * Results:
 *      None really.
 *
 * Side effects:
 *	buf_length and buffer pointer in VidStream structure
 *      may be changed.
 *
 *--------------------------------------------------------------
 */

static void eof_correct_underflow( VidStream *vid_stream )
{
  /*
     Make 32 bits equal to 0 and 32 bits after that
     equal to seq end code in order to prevent
     messy data from infinite recursion.
  */
  static unsigned long seq_end_mark[2] = { 0, SEQ_END_CODE };

  vid_stream->buffer = seq_end_mark;
  vid_stream->buf_length += 2;
}

void vid_correct_underflow( VidStream *vid_stream )
{
  long num_read;

  if ((num_read = (*vid_stream->get_more_data)
	(vid_stream, &(unsigned char *)vid_stream->buffer)) <= 0)
    (*(vid_stream->correct_underflow = eof_correct_underflow))
      (vid_stream);
  else {
    while (num_read & 3) ((char *)vid_stream->buffer)[num_read++] = 0;
    num_read >>= 2;
    vid_stream->buf_length += num_read;
  }
}

void sys_correct_underflow( VidStream *vid_stream )
{
  unsigned char *pbuf;
  long num_read;

  pbuf = vid_stream->vid_buf;
  vid_stream->buffer = (unsigned long *)pbuf;
  num_read = BUF_LENGTH;
  do {
    if (vid_stream->packet_length == 0)
      if (read_sys(vid_stream) == 0) break;
    --vid_stream->packet_length;
    FGETC(vid_stream, *pbuf++);
  }
  while (--num_read);
  if ((num_read = pbuf - vid_stream->vid_buf) <= 0)
    (*(vid_stream->correct_underflow = eof_correct_underflow))
      (vid_stream);
  else {
    while (num_read & 3) { *pbuf++ = 0; num_read++; }
    num_read >>= 2;
    vid_stream->buf_length += num_read;
  }
}
