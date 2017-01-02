/*
 * fixpix.h
 *
 * Sample declarations for FixPix data structures and utility functions.
 * Platform independent.
 *
 * Developed 1996-2008 by Guido Vollbeding <guido@jpegclub.org>
 */

/* The following array represents the pixel values for each shade
 * of the primary color components.
 * If 'p' is a pointer to a source image rgb-byte-triplet, we can
 * construct the output pixel value simply by 'oring' together
 * the corresponding components:
 *
 *	unsigned char *p;
 *	unsigned long pixval;
 *
 *	pixval  = screen_rgb[0][*p++];
 *	pixval |= screen_rgb[1][*p++];
 *	pixval |= screen_rgb[2][*p++];
 *
 * This is both efficient and generic, since the only assumption
 * is that the primary color components have separate bits.
 * The order and distribution of bits does not matter, and we
 * don't need additional variables and shifting/masking code.
 * The array size is 3 KBytes total and thus very reasonable.
 */

extern unsigned long screen_rgb[3][256];

/* The following array holds the exact color representations
 * reported by the system.
 * This is useful for less than 24 bit deep displays as a base
 * for additional dithering to get smoother output.
 */

extern unsigned char screen_set[3][256];

/* The following routine initializes the screen_rgb and screen_set
 * arrays.
 * The routine may use some global display variables dependent on
 * the platform (see xfixpix.c, vfixpix.c).
 */

void screen_init(void);


/* Declarations for Floyd-Steinberg dithering.
 */

typedef INT16 FSERROR;		/* 16 bits should be enough */
typedef int LOCFSERROR;		/* use 'int' for calculation temps */

typedef struct { unsigned char *colorset;
		 FSERROR *fserrors;
	       } FSBUF;

/* Floyd-Steinberg initialization function.
 *
 * It is called 'fs2_init' since it's specialized for our purpose and
 * could be embedded in a more general FS-package.
 *
 * Returns a malloced FSBUF pointer which has to be passed as first
 * parameter to subsequent 'fs2_dither' calls.
 * The FSBUF structure does not need to be referenced by the calling
 * application, it can be treated from the app like a void pointer.
 *
 * The current implementation does only require to free() this returned
 * pointer after processing.
 *
 * Returns NULL if malloc fails.
 *
 * NOTE: The FSBUF structure is designed to allow the 'fs2_dither'
 * function to work with an *arbitrary* number of color components
 * at runtime! This is an enhancement over the IJG code base :-).
 * Only fs2_init() specifies the (maximum) number of components.
 */

FSBUF *fs2_init(int width);

/* Floyd-Steinberg dithering function.
 *
 * NOTE:
 * (1) The image data referenced by 'ptr' is *overwritten* (input *and*
 *     output) to allow more efficient implementation.
 * (2) Alternate FS dithering is provided by the sign of 'nc'. Pass in
 *     a negative value for right-to-left processing. The return value
 *     provides the right-signed value for subsequent calls!
 * (3) The provided implementation assumes *no* padding between lines!
 *     Adapt this if necessary.
 */

int fs2_dither(FSBUF *fs, unsigned char *ptr, int nc, int num_rows, int num_cols);
