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

typedef struct
{
  long	cr_r[257],	/* => table for Cr to R conversion */
	cb_b[257],	/* => table for Cb to B conversion */
	cr_g[257],	/* => table for Cr to G conversion */
	cb_g[257];	/* => table for Cb to G conversion */
}
YCC_RGB_TAB;

extern YCC_RGB_TAB ycc_rgb_tab;

/* Define BOOLEAN, TRUE, and FALSE. */

#define BOOLEAN int
#define TRUE 1
#define FALSE 0

/* Define buffer length. */

#define BUF_LENGTH 80L*1024

/* Set ring buffer size. */

#define RING_BUF_SIZE 3

/* Macros for picture code type. */

#define I_TYPE 1
#define P_TYPE 2
#define B_TYPE 3

/* Start codes. */

#define SEQ_END_CODE 0x01b7
#define SEQ_START_CODE 0x01b3
#define GOP_START_CODE 0x01b8
#define PICTURE_START_CODE 0x0100
#define SLICE_MIN_START_CODE 0x0101
#define SLICE_MAX_START_CODE 0x01af
#define EXT_START_CODE 0x01b5
#define USER_START_CODE 0x01b2

/* System Layer Junk */

#define START_CODE_PREFIX        0x0100
#define ISO_11172_END_CODE       0x01b9
#define PACK_START_CODE          0x01ba
#define SYSTEM_HEADER_START_CODE 0x01bb

#define PACK_HEADER_SIZE   8
#define SYSTEM_HEADER_SIZE 6

#define STD_AUDIO_STREAM_ID 0xb8
#define STD_VIDEO_STREAM_ID 0xb9
#define MIN_STREAM_ID_ID    0xbc
#define RESERVED_STREAM_ID  0xbc
#define PRIVATE_STREAM_1_ID 0xbd
#define PADDING_STREAM_ID   0xbe
#define PRIVATE_STREAM_2_ID 0xbf

#define STD_SYSTEM_CLOCK_FREQ 90000L
#define MUX_RATE_SCALE_FACTOR 50
#define MAX_STREAMS 8

/* Macros used with macroblock address decoding. */

#define MB_STUFFING 34
#define MB_ESCAPE 35

/* Lock flags for pict images. */

#define DISPLAY_LOCK 0x01
#define PAST_LOCK 0x02
#define FUTURE_LOCK 0x04

/* Temporary definition of time stamp structure. */

typedef long TimeStamp;

/* Structure with reconstructed pixel values. */

typedef struct pict_image {
  unsigned char *luminance;	/* Luminance plane.	*/
  unsigned char *Cr;		/* Cr plane.		*/
  unsigned char *Cb;		/* Cb plane.		*/
  TimeStamp show_time;		/* Presentation time.	*/
  int locked, pad;		/* Lock flag.		*/
} PictImage;

/* Group of pictures structure. */

typedef struct GoP {
  char *ext_data;		/* Extension data. 			*/
  char *user_data;		/* User data.				*/
  BOOLEAN drop_flag;		/* Flag indicating dropped frame.	*/
  int tc_hours; 		/* Hour component of time code.		*/
  int tc_minutes;		/* Minute component of time code.	*/
  int tc_seconds;		/* Second component of time code.	*/
  int tc_pictures;		/* Picture counter of time code.	*/
  BOOLEAN closed_gop;		/* Indicates no pred. vectors to
				   previous group of pictures.		*/
  BOOLEAN broken_link;		/* B frame unable to be decoded.	*/
} GoP;

/* Picture structure. */

typedef struct pict {
  unsigned temp_ref;		/* Temporal reference.		*/
  unsigned code_type;		/* Frame type: P, B, I		*/
  unsigned vbv_delay;		/* Buffer delay.		*/
  BOOLEAN full_pel_forw_vector; /* Forw. vectors specified in
				   full pixel values flag.	*/
  int forw_r_size;		/* Used for vector decoding.	*/
  BOOLEAN full_pel_back_vector; /* Back vectors specified in
				   full pixel values flag.	*/
  int back_r_size;		/* Used in decoding.		*/
  char *extra_info;		/* Extra bit picture info. 	*/
  char *ext_data;		/* Extension data. 		*/
  char *user_data;		/* User data.			*/
} Pict;

/* Slice structure. */

typedef struct slice {
  char *extra_info;		/* Extra bit slice info.	*/
  int quant_scale;		/* Quantization scale.		*/
} Slice;

/* Macroblock structure. */

typedef struct macroblock {
  long mb_address;		/* Macroblock address.			*/
  long past_mb_addr;		/* Previous mblock address.		*/
  int cbp;			/* Coded block pattern.			*/
  BOOLEAN mb_intra;		/* Intracoded mblock flag.		*/
  BOOLEAN bpict_past_forw;	/* Past B frame forw. vector flag.	*/
  BOOLEAN bpict_past_back;	/* Past B frame back vector flag.	*/
  long recon_right_for;		/* Past right forw. vector.		*/
  long recon_down_for;		/* Past down forw. vector.		*/
  long recon_right_back;	/* Past right back vector.		*/
  long recon_down_back;		/* Past down back vector.		*/
} Macroblock;

/* Block structure. */

typedef struct block {
  short dct_recon[8][8];	/* Reconstructed dct coeff matrix.	*/
  short dct_dc_y_past;		/* Past lum. dc dct coefficient.	*/
  short dct_dc_cr_past; 	/* Past cr dc dct coefficient.		*/
  short dct_dc_cb_past; 	/* Past cb dc dct coefficient.		*/
} Block;

/* Video stream structure. */

typedef struct vid_stream {
  int bits_left;		/* Bits remaining in stream.		*/
  int init_flag;
  unsigned long curBits;
  unsigned long *buffer;	/* Pointer to next data in buffer.	*/
  long buf_length;		/* Length of remaining buffer.		*/
  int h_size;			/* Horiz. size in pixels.		*/
  int v_size;			/* Vert. size in pixels.		*/
  int mb_height;		/* Vert. size in mblocks.		*/
  int mb_width; 		/* Horiz. size in mblocks.		*/
  unsigned long bit_rate;	/* Bit rate.				*/
  unsigned long vbv_buffer_size;/* Minimum buffer size.			*/
  unsigned char intra_quant_matrix[64];
		/* Quantization matrix for intracoded frames.		*/
  unsigned char non_intra_quant_matrix[64];
		/* Quantization matrix for non intracoded frames.	*/
  char *ext_data;		/* Extension data.			*/
  char *user_data;		/* User data.				*/
  GoP group;			/* Current group of pict.		*/
  Pict picture; 		/* Current picture.			*/
  Slice slice;			/* Current slice.			*/
  unsigned char aspect_ratio;	/* Code for aspect ratio.		*/
  unsigned char picture_rate;	/* Code for picture rate.		*/
  Macroblock mblock;		/* Current macroblock.			*/
  Block block;			/* Current block.			*/
  BOOLEAN const_param_flag;	/* Contrained parameter flag.		*/
  PictImage *past;		/* Past predictive frame.		*/
  PictImage *future;		/* Future predictive frame.		*/
  PictImage *current;		/* Current frame.			*/
  PictImage ring[RING_BUF_SIZE];	/* Ring buffer of frames.	*/
  unsigned char *red;		/* Red plane.				*/
  unsigned char *green;		/* Green plane.				*/
  unsigned char *blue;		/* Blue plane.				*/
  unsigned char *old_ptr;
  void (*y_scale)( unsigned char *src, unsigned char *des,
		   int src_cols, int src_rows, int des_cols );
  void (*c_scale)( unsigned char *src, unsigned char *des,
		   int src_cols, int src_rows, int des_cols );
  void (*c_convert)( struct vid_stream *vs );
  void (*correct_underflow)( struct vid_stream *vs );
  void (*put_message)( const char *msg, ... );
  void *(*alloc)( struct vid_stream *vs, unsigned long size );
  int (*free)( struct vid_stream *vs, void *ptr );
  long (*get_more_data)( struct vid_stream *vs, unsigned char **buf_ptr );
  long realTimeStart;
  int No_P_Flag;		/* P skip flag.				*/
  int No_B_Flag;		/* B skip flag.				*/
  int color;
  int scale;
  int dest_width;
  int dest_height;
  unsigned char *vid_buf;	/* System Layer Junk...			*/
  unsigned char *sys_buf;
  long bytes_left;
  unsigned short packet_length;
  int EOF_flag;
  int VideoStreamID;
  int AudioStreamID;
} VidStream;

/* Declaration of functions. */

int mpegVidRsrc( VidStream *vid_stream, TimeStamp time_stamp );
void mpegNewScale( VidStream *vid_stream, int new_scale );
void *ex_alloc( VidStream *vid_stream, unsigned long size );
void ComputeVector( long *recon_ptr,
		    int r_size, BOOLEAN full_pel_vector,
		    long motion_code, long motion_r );
void ParseReconBlock( VidStream *vid_stream, int n );
void ParseAwayBlock( VidStream *vid_stream, int n );
void j_rev_dct( short *beg_data, short *end_data );
void init_tables( void );
void PrintTimeInfo( VidStream *vid_stream );
void vid_correct_underflow( VidStream *vid_stream );
void sys_correct_underflow( VidStream *vid_stream );
void ycc_rgb_init( void );
