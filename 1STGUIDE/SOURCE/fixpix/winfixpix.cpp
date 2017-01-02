/*
 * winfixpix.cpp
 *
 * Sample implementation of FixPix routines for the Windows GDI.
 * It is an excerpt from Jpegcrop Dibview.cpp with FixPix adaptions
 * and should be easy to adapt for other applications.
 *
 * NOTE:
 * In this context, we only use the dither option of FixPix to
 * provide smoother output in 16 bits per pixel display modes.
 * We simply use Windows DIBs (Device Independent Bitmaps) and
 * let Windows do the device dependent rendering itself.
 *
 * Developed 2000-2008 by Guido Vollbeding <guido@jpegclub.org>
 */

/* The following array holds the exact color representations
 * reported by the system.
 * This is useful for less than 24 bit deep displays as a base
 * for additional dithering to get smoother output.
 */

static unsigned char screen_set[3][256];

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

typedef short FSERROR;		/* 16 bits should be enough */
typedef int LOCFSERROR;		/* use 'int' for calculation temps */

typedef struct
{
  unsigned char *colorset;
  FSERROR *fserrors;
}
FSBUF;

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

FSBUF *fs2_init(int width, int nc)
{
  FSBUF *fs;
  FSERROR *p;
  int i;

  fs = (FSBUF *)
    malloc(sizeof(FSBUF) * nc + ((size_t)width + 2) * sizeof(FSERROR) * nc);
  if (fs == 0) return fs;

  for (i = 0; i < nc; i++) fs[i].colorset = screen_set[i];

  p = (FSERROR *)(fs + nc);
  memset(p, 0, ((size_t)width + 2) * sizeof(FSERROR) * nc);

  for (i = 0; i < nc; i++) fs[i].fserrors = p++;

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
 */

int fs2_dither(FSBUF *fs, unsigned char *ptr, int nc, int num_rows, int num_cols, int num_scan)
{
  int abs_nc, ci, row, col;
  LOCFSERROR delta, cur, belowerr, bpreverr;
  unsigned char *dataptr, *colsetptr;
  FSERROR *errorptr;

  if ((abs_nc = nc) < 0) abs_nc = -abs_nc;
  num_scan -= abs_nc;
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
	*dataptr = colsetptr[cur];
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
    ptr += num_scan;
    nc = -nc;
  }
  return nc;
}

/////////////////////////////////////////////////////////////////////////////
// CDibView drawing

void CDibView::OnDraw(CDC* pDC)
{
    static int init_flag = 0;

    CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
    CDibDoc *pDoc = GetDocument();

    CRect rcDisp;
    pDC->GetClipBox(rcDisp);

    if (pDoc->GetHDIB() == NULL)
    {
	pDC->FillSolidRect(rcDisp, 0x00808080L);
	return;
    }

    if (pDoc->m_dithered == 0)
	if (myapp->m_dither == 0 || pDC->GetDeviceCaps(BITSPIXEL) != 16)
	    pDoc->m_dithered = 1;
	else
	{
	    if (init_flag == 0)
	    {
		init_flag = 1;
		for (int i = 0; i < 256; i++)
		{
		    screen_set[2][i] = (unsigned char)
			(pDC->SetPixel(rcDisp.left, rcDisp.top, (COLORREF)i) & 255);
		    screen_set[1][i] = (unsigned char)
			((pDC->SetPixel(rcDisp.left, rcDisp.top, (COLORREF)(i << 8)) >> 8) & 255);
		    screen_set[0][i] = (unsigned char)
			((pDC->SetPixel(rcDisp.left, rcDisp.top, (COLORREF)(i << 16)) >> 16) & 255);
		}
	    }
	    HDIB hDib = pDoc->GetHDIB();
	    if (hDib)
	    {
		LPBITMAPINFOHEADER lpDIB;
		int DIBLineWidth;
		int DIBScanWidth;
		unsigned char *lpBits;

		lpDIB = (LPBITMAPINFOHEADER)GlobalLock(hDib);

		DIBLineWidth = lpDIB->biWidth * lpDIB->biBitCount / 8;
		DIBScanWidth = (DIBLineWidth + 3) & (-4);

		lpBits = (unsigned char *) FindDIBBits((LPSTR) lpDIB);

		FSBUF *fs = fs2_init(lpDIB->biWidth, lpDIB->biBitCount / 8);
		if (fs) {
		    fs2_dither(fs, lpBits, lpDIB->biBitCount / 8,
			       lpDIB->biHeight, lpDIB->biWidth,
			       DIBScanWidth);
		    free(fs);
		    pDoc->m_dithered = 1;
		}

		GlobalUnlock(hDib);
	    }
	}

  ...
