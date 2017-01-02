typedef struct
{
  short version;
  unsigned short headlen;
  short planes,
	pat_run,
	pix_width,
	pix_height,
	sl_width,
	sl_height;
}
IMG_HEADER;

/* Public struct for buffered file i/o.
 * data_func must be 'read' for file input, 'write' for file output.
 * It has the task to update the bytes_left entry with size of next
 * data if read, size of output buffer if write, and to set the pbuf
 * entry to point to the next data if read, output buffer if write.
 * NOTE: Because of the structure of the below input fetch macros,
 *       it is important to UPDATE (add, not set) the bytes_left
 *       entry with the new data buffer size if read!
 * The public struct should be used as substruct being the first
 * entry in a specific struct, which holds additional data (i.e.
 * data buffer pointer and data buffer size) to be used by data_func.
 * Look in the main module to see what is required.
 */

typedef struct fbufpub
{
  char *pbuf;
  long bytes_left;
  void (*data_func)(struct fbufpub *fp);
}
FBUFPUB;

/* Public struct for image data processing. */

typedef struct ibufpub
{
  char *pbuf;
  long bytes_left;
  void (*put_line)(struct ibufpub *ip);
  char *pat_buf;
  short pat_run;
  char vrc, pad;
}
IBUFPUB;


/* The following macros handle buffered file i/o and
 * image output based on the introduced structs.
 */

#define MAKESTMT(stuff)	  do { stuff } while (0)

#define FGETC(fp, dest)   \
  MAKESTMT( if (--(fp)->bytes_left < 0) (*(fp)->data_func)(fp); \
	    dest = *(fp)->pbuf++; )

#define FSKIPC(fp)   \
  MAKESTMT( if (--(fp)->bytes_left < 0) (*(fp)->data_func)(fp); \
	    (fp)->pbuf++; )

#define FPUTC(fp, ch)   \
  MAKESTMT( *(fp)->pbuf++ = ch; \
	    if (--(fp)->bytes_left == 0) (*(fp)->data_func)(fp); )

#define FCOPYC(src, des)   \
  MAKESTMT( if (--(src)->bytes_left < 0) (*(src)->data_func)(src); \
	    *(des)->pbuf++ = *(src)->pbuf++; \
	    if (--(des)->bytes_left == 0) (*(des)->data_func)(des); )

#define ISKIPC(ip)   \
  MAKESTMT( (ip)->pbuf++; \
	    if (--(ip)->bytes_left == 0) (*(ip)->put_line)(ip); )

#define IPUTC(ip, ch)   \
  MAKESTMT( *(ip)->pbuf++ = ch; \
	    if (--(ip)->bytes_left == 0) (*(ip)->put_line)(ip); )

#define FICOPYC(src, des)   \
  MAKESTMT( if (--(src)->bytes_left < 0) (*(src)->data_func)(src); \
	    *(des)->pbuf++ = *(src)->pbuf++; \
	    if (--(des)->bytes_left == 0) (*(des)->put_line)(des); )

/* Automatic HiByte/LoByte data conversion is provided by the
 * following macros to avoid use of the standard ntohs/htons
 * scheme here.
 * This ensures unique interpretation/representation of external
 * data in natural "Network" Order (HiByte first, then LoByte)
 * by different hosts (especially for Intel-Systems).
 */

#define FGETW(fp, dest)   \
  MAKESTMT( if (--(fp)->bytes_left < 0) (*(fp)->data_func)(fp); \
	    dest = *((unsigned char *)(fp)->pbuf)++; \
	    dest <<= 8; \
	    if (--(fp)->bytes_left < 0) (*(fp)->data_func)(fp); \
	    dest |= *((unsigned char *)(fp)->pbuf)++; )

#define FPUTW(fp, wd)   \
  MAKESTMT( *(fp)->pbuf++ = (char)(wd >> 8); \
	    if (--(fp)->bytes_left == 0) (*(fp)->data_func)(fp); \
	    *(fp)->pbuf++ = (char)wd; \
	    if (--(fp)->bytes_left == 0) (*(fp)->data_func)(fp); )


#define FGETL(fp, dest)   \
  MAKESTMT( if (--(fp)->bytes_left < 0) (*(fp)->data_func)(fp); \
	    dest = *((unsigned char *)(fp)->pbuf)++; \
	    dest <<= 8; \
	    if (--(fp)->bytes_left < 0) (*(fp)->data_func)(fp); \
	    dest |= *((unsigned char *)(fp)->pbuf)++; \
	    dest <<= 8; \
	    if (--(fp)->bytes_left < 0) (*(fp)->data_func)(fp); \
	    dest |= *((unsigned char *)(fp)->pbuf)++; \
	    dest <<= 8; \
	    if (--(fp)->bytes_left < 0) (*(fp)->data_func)(fp); \
	    dest |= *((unsigned char *)(fp)->pbuf)++; )

/* Prototypes for public library functions. */

void level_3_decode(FBUFPUB *input, IBUFPUB *image);

IBUFPUB *encode_init(IMG_HEADER *input_header, FBUFPUB *output,
		     void *(*user_malloc)(long size),
		     void (*user_exit)(void),
		     short out_lev, short out_pat);

IBUFPUB *l3p2_encode_init(IMG_HEADER *input_header, FBUFPUB *output,
			  void *(*user_malloc)(long size),
			  void (*user_exit)(void));

IBUFPUB *l3p1_encode_init(IMG_HEADER *input_header, FBUFPUB *output,
			  void *(*user_malloc)(long size),
			  void (*user_exit)(void));

IBUFPUB *l2p2_encode_init(IMG_HEADER *input_header, FBUFPUB *output,
			  void *(*user_malloc)(long size),
			  void (*user_exit)(void));

IBUFPUB *l2p1_encode_init(IMG_HEADER *input_header, FBUFPUB *output,
			  void *(*user_malloc)(long size),
			  void (*user_exit)(void));

IBUFPUB *l1p2_encode_init(IMG_HEADER *input_header, FBUFPUB *output,
			  void *(*user_malloc)(long size),
			  void (*user_exit)(void));

IBUFPUB *l1p1_encode_init(IMG_HEADER *input_header, FBUFPUB *output,
			  void *(*user_malloc)(long size),
			  void (*user_exit)(void));

/* Extended TIMG header.
 * NOTE:
 *   - The bitplane layout in the pixel data is in
 *     Low-High-Order:
 *       R0 R1 R2 ... G0 G1 G2 ... B0 B1 B2 ...
 *     where R0 is the plane for bit 0 (low) of red, etc.
 *   - The actual implementation requires the component bit
 *     counts to be positive (which doesn't restrict the
 *     available range too much, I think :-) or zero.
 *   - The sum of the component bit counts must be less than
 *     or equal to the 'planes' value in the img header.
 *     If it is less, then the remaining bitplanes build
 *     the Alpha-Channel.
 */

typedef struct
{
  IMG_HEADER ih;		/* standard img header */
  long magic;			/* 'TIMG' */
  short comps,			/* 3 for RGB(A), 1 for Y(A) */
	red,			/* means gray if comps = 1 */
	green,			/* only valid if comps = 3 */
	blue;			/* only valid if comps = 3 */
}
TIMG_HEADER;

/* Public struct for TrueColor output.
 * Unlike the FBUFPUB and IBUFPUB objects, the pixel data buffer
 * is provided by the caller of put_true_pix_row, and parameters
 * are passed as arguments to put_true_pix_row.
 * The reason for introducing the public struct instead of using
 * only the function pointer is, that passing the public struct
 * pointer to put_true_pix_row allows easier access to additional
 * local variables to be used by the output module for further
 * processing (dithering, rendering, etc.), similar to the use
 * of FBUFPUB and IBUFPUB.
 *
 * The output data format is in usual TrueColor form with 8 bits
 * (1 byte) per component in unsigned intensity representation
 * (0 = minimum, 255 = maximum), using conventional interleaved
 * component layout (i.e. all components of one pixel appear
 * successively).
 *
 * The exact data layout depends on the value of num_comps.
 * There are 4 possible values:
 *
 *   num_comps  colorspace       data layout (1 letter per byte)
 * -------------------------------------------------------------
 *      1       Y (Gray)         YYYYYYYYYYYY...
 *      2       YA (Gray-Alpha)  YAYAYAYAYAYA...
 *      3       RGB              RGBRGBRGBRGB...
 *      4       RGBA (RGB-Alpha) RGBARGBARGBA...
 *
 * num_cols is the number of pixels in the row, thus
 * num_cols * num_comps is the total row data size.
 */

typedef struct tputpub
{
  void (*put_true_pix_row)(struct tputpub *tp, unsigned char *data,
			   short num_cols, short num_comps);
}
TPUTPUB;

IBUFPUB *true_out_init(TIMG_HEADER *th, TPUTPUB *true_put,
		       void *(*user_malloc)(long size),
		       void (*user_exit)(void),
		       short out_comps);
