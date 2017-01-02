/*
 * xfixpix.c
 *
 * Sample implementation of FixPix routines for the X Window System.
 * Currently leaned on the xv-3.10a context, but should be easy to
 * adapt for other applications.
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

unsigned long screen_rgb[3][256];

/* The following array holds the exact color representations
 * reported by the system.
 * This is useful for less than 24 bit deep displays as a base
 * for additional dithering to get smoother output.
 */

unsigned char screen_set[3][256];

/* The following routine initializes the screen_rgb and screen_set
 * arrays.
 * Since it is executed only once per program run, it does not need
 * to be super-efficient.
 *
 * The method is to draw points in a pixmap with the specified shades
 * of primary colors and then get the corresponding XImage pixel
 * representation.
 * Thus we can get away with any Bit-order/Byte-order dependencies.
 *
 * The routine uses some global X variables:
 * theDisp, theScreen, dispDEEP, and theCmap.
 * Adapt these to your application as necessary.
 * I've not passed them in as parameters, since for other platforms
 * than X these may be different (see vfixpix.c), and so the
 * screen_init() interface is unique.
 */

void screen_init(void)
{
  static int init_flag; /* assume auto-init as 0 */
  Pixmap check_map;
  GC check_gc;
  XColor check_col;
  XImage *check_image;
  int ci, i;

  if (init_flag) return;
  init_flag = 1;

  check_map = XCreatePixmap(theDisp, RootWindow(theDisp,theScreen),
			    1, 1, dispDEEP);
  check_gc = XCreateGC(theDisp, check_map, 0, NULL);
  for (ci = 0; ci < 3; ci++) {
    for (i = 0; i < 256; i++) {
      check_col.red = 0;
      check_col.green = 0;
      check_col.blue = 0;
      /* Do proper upscaling from unsigned 8 bit (image data values)
	 to unsigned 16 bit (X color representation). */
      ((unsigned short *)&check_col.red)[ci] = (unsigned short)((i << 8) | i);
      XAllocColor(theDisp, theCmap, &check_col);
      screen_set[ci][i] =
	(((unsigned short *)&check_col.red)[ci] >> 8) & 0xff;
      XSetForeground(theDisp, check_gc, check_col.pixel);
      XDrawPoint(theDisp, check_map, check_gc, 0, 0);
      check_image = XGetImage(theDisp, check_map, 0, 0, 1, 1,
			      AllPlanes, ZPixmap);
      if (check_image) {
	switch (check_image->bits_per_pixel) {
	case 8:
	  screen_rgb[ci][i] = *(CARD8 *)check_image->data;
	  break;
	case 16:
	  screen_rgb[ci][i] = *(CARD16 *)check_image->data;
	  break;
	case 24:
	  screen_rgb[ci][i] =
	    ((unsigned long)*(CARD8 *)check_image->data << 16) |
	    ((unsigned long)*(CARD8 *)(check_image->data + 1) << 8) |
	    (unsigned long)*(CARD8 *)(check_image->data + 2);
	  break;
	case 32:
	  screen_rgb[ci][i] = *(CARD32 *)check_image->data;
	  break;
	}
	XDestroyImage(check_image);
      }
    }
  }
  XFreeGC(theDisp, check_gc);
  XFreePixmap(theDisp, check_map);
}


/* Should better be a runtime switch... */

#define DO_FIXPIX_SMOOTH

#ifdef DO_FIXPIX_SMOOTH

/* The following code is based in part on:
 *
 * jquant1.c
 *
 * Copyright (C) 1991-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains 1-pass color quantization (color mapping) routines.
 * These routines provide mapping to a fixed color map using equally spaced
 * color values.  Optional Floyd-Steinberg or ordered dithering is available.
 */

/* Declarations for Floyd-Steinberg dithering.
 *
 * Errors are accumulated into the array fserrors[], at a resolution of
 * 1/16th of a pixel count.  The error at a given pixel is propagated
 * to its not-yet-processed neighbors using the standard F-S fractions,
 *		...	(here)	7/16
 *		3/16	5/16	1/16
 * We work left-to-right on even rows, right-to-left on odd rows.
 *
 * We can get away with a single array (holding one row's worth of errors)
 * by using it to store the current row's errors at pixel columns not yet
 * processed, but the next row's errors at columns already processed.  We
 * need only a few extra variables to hold the errors immediately around the
 * current column.  (If we are lucky, those variables are in registers, but
 * even if not, they're probably cheaper to access than array elements are.)
 *
 * We provide (#columns + 2) entries per component; the extra entry at each
 * end saves us from special-casing the first and last pixels.
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

FSBUF *fs2_init(int width)
{
  FSBUF *fs;
  FSERROR *p;

  fs = (FSBUF *)
    malloc(sizeof(FSBUF) * 3 + ((size_t)width + 2) * sizeof(FSERROR) * 3);
  if (fs == 0) return fs;

  fs[0].colorset = screen_set[0];
  fs[1].colorset = screen_set[1];
  fs[2].colorset = screen_set[2];

  p = (FSERROR *)(fs + 3);
  memset(p, 0, ((size_t)width + 2) * sizeof(FSERROR) * 3);

  fs[0].fserrors = p;
  fs[1].fserrors = p + 1;
  fs[2].fserrors = p + 2;

  return fs;
}

/* Floyd-Steinberg dithering function.
 *
 * NOTE:
 * (1) The image data referenced by 'ptr' is *overwritten* (input *and*
 *     output) to allow more efficient implementation.
 * (2) Alternate FS dithering is provided by the sign of 'nc'. Pass in
 *     a negative value for right-to-left processing. The return value
 *     provides the right-signed value for subsequent calls!
 * (3) This particular implementation assumes *no* padding between lines!
 *     Adapt this if necessary.
 */

int fs2_dither(FSBUF *fs, unsigned char *ptr, int nc, int num_rows, int num_cols)
{
  int abs_nc, ci, row, col;
  LOCFSERROR delta, cur, belowerr, bpreverr;
  unsigned char *dataptr, *colsetptr;
  FSERROR *errorptr;

  if ((abs_nc = nc) < 0) abs_nc = -abs_nc;
  for (row = 0; row < num_rows; row++) {
    for (ci = 0; ci < abs_nc; ci++, ptr++) {
      dataptr = ptr;
      colsetptr = fs[ci].colorset;
      errorptr = fs[ci].fserrors;
      if (nc < 0) {
	dataptr += (num_cols - 1) * abs_nc;
	errorptr += (num_cols + 1) * abs_nc;
      }
      cur = belowerr = bpreverr = 0;
      for (col = 0; col < num_cols; col++) {
	cur += errorptr[nc];
	cur += 8; cur >>= 4;
	if ((cur += *dataptr) < 0) cur = 0;
	else if (cur > 255) cur = 255;
	*dataptr = cur & 0xff;
	cur -= colsetptr[cur];
	delta = cur << 1; cur += delta;
	bpreverr += cur; cur += delta;
	belowerr += cur; cur += delta;
	errorptr[0] = (FSERROR)bpreverr;
	bpreverr = belowerr;
	belowerr = delta >> 1;
	dataptr += nc;
	errorptr += nc;
      }
      errorptr[0] = (FSERROR)bpreverr;
    }
    ptr += (num_cols - 1) * abs_nc;
    nc = -nc;
  }
  return nc;
}

#endif /* DO_FIXPIX_SMOOTH */


/* The following is an excerpt from xvimage.c with FixPix adaptions.
 * Note the shorter and more efficient code using less variables and less
 * assumptions. We can get away with any Bit-order/Byte-order hassle
 * and associated shifting/masking. At the same time it is more general,
 * since the bits associated with a primary color component can be
 * arbitrarily distributed in the pixel, not necessary continuously.
 */

XImage *Pic24ToXImage(pic24, wide, high)
     byte          *pic24;
     unsigned int   wide, high;
{

  int     i,j;
  XImage *xim;

  ...

  if (theVisual->class == TrueColor || theVisual->class == DirectColor) {

    /************************************************************************/
    /* Non-ColorMapped Visuals:  TrueColor, DirectColor                     */
    /************************************************************************/

    unsigned long xcol;
    int           bperpix, bperline;
    byte         *imagedata, *lip, *ip, *pp;


    xim = XCreateImage(theDisp, theVisual, dispDEEP, ZPixmap, 0, NULL,
		        wide,  high, 32, 0);
    if (!xim) FatalError("couldn't create X image!");

    bperline = xim->bytes_per_line;
    bperpix  = xim->bits_per_pixel;

    imagedata = (byte *) malloc((size_t) (high * bperline));
    if (!imagedata) FatalError("couldn't malloc imagedata");

    xim->data = (char *) imagedata;

    if (bperpix != 8 && bperpix != 16 && bperpix != 24 && bperpix != 32) {
      char buf[128];
      sprintf(buf,"Sorry, no code written to handle %d-bit %s",
	      bperpix, "TrueColor/DirectColor displays!");
      FatalError(buf);
    }

    screen_init();

#ifdef DO_FIXPIX_SMOOTH
#if 0
    /* If we wouldn't have to save the original pic24 image data,
     * the following code would do the dither job by overwriting
     * the image data, and the normal render code would then work
     * without any change on that data.
     * Unfortunately, this approach would hurt the xv assumptions...
     */
    if (bperpix < 24) {
      FSBUF *fs = fs2_init(wide);
      if (fs) {
	fs2_dither(fs, pic24, 3, high, wide);
	free(fs);
      }
    }
#else
    /* ...so we have to take a different approach with linewise
     * dithering/rendering in a loop using a temporary line buffer.
     */
    if (bperpix < 24) {
      FSBUF *fs = fs2_init(wide);
      if (fs) {
	byte *row_buf = malloc((size_t)wide * 3);
	if (row_buf) {
	  int nc = 3;
	  byte *picp = pic24;  lip = imagedata;
	  for (i=0; i<high; i++, lip+=bperline, picp+=(size_t)wide*3) {
	    memcpy(row_buf, picp, (size_t)wide * 3);
	    nc = fs2_dither(fs, row_buf, nc, 1, wide);
	    for (j=0, ip=lip, pp=row_buf; j<wide; j++) {

	      xcol  = screen_rgb[0][*pp++];
	      xcol |= screen_rgb[1][*pp++];
	      xcol |= screen_rgb[2][*pp++];

	      switch (bperpix) {
	      case 8:
		*ip++ = xcol & 0xff;
		break;
	      case 16:
		*((CARD16 *)ip)++ = (CARD16)xcol;
		break;
	      }
	    }
	  }
	  free(row_buf);
	  free(fs);

	  return xim;
	}
	free(fs);
      }
    }
#endif
#endif

    lip = imagedata;  pp = pic24;
    for (i=0; i<high; i++, lip+=bperline) {
      for (j=0, ip=lip; j<wide; j++) {

	xcol  = screen_rgb[0][*pp++];
	xcol |= screen_rgb[1][*pp++];
	xcol |= screen_rgb[2][*pp++];

	switch (bperpix) {
	case 8:
	  *ip++ = xcol & 0xff;
	  break;
	case 16:
	  *((CARD16 *)ip)++ = (CARD16)xcol;
	  break;
	case 24:
	  *ip++ = (xcol >> 16) & 0xff;
	  *ip++ = (xcol >> 8)  & 0xff;
	  *ip++ =  xcol        & 0xff;
	  break;
	case 32:
	  *((CARD32 *)ip)++ = (CARD32)xcol;
	  break;
	}
      }
    }
  }

  ...

  return xim;
}
