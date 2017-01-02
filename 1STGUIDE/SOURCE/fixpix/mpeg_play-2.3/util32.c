/* 
 * util32.c --
 *
 *      Miscellaneous functions that deal with 32 bit color displays.
 *
 */

/*
 * Copyright (c) 1995 The Regents of the University of California.
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
 * Portions of this software Copyright (c) 1995 Brown University.
 * All rights reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement
 * is hereby granted, provided that the above copyright notice and the
 * following two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF BROWN
 * UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * BROWN UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
 * BASIS, AND BROWN UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "video.h"
#include "proto.h"

/*
   Changes to make the code reentrant:
      deglobalized: matched_depth
      use X variables in xinfo instead of globals
   Additional changes:
      #ifdef DISABLE_DITHER  - don't compile dither code
      fix parameter types for XCreateWindow call
   -lsh@cs.brown.edu (Loring Holden)

   FixPix adaptions by Guido Vollbeding <guivol@esc.de>
     (http://www.esc.de/homes/guivol/fixpix/)
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

/* This value represents the bits_per_pixel value needed for
 * the video stream image output frame allocation.
 * This is necessary because it can be *different* from the
 * color depth value! This value is then set in main() as
 * 'matched_depth' in the VidStream structure.
 */

int screen_bpp; /* assume auto-init as 0 */

/* The following routine initializes the screen_rgb array
 * and screen_bpp value.
 * Since it is executed only once per program run, it does not need
 * to be super-efficient.
 *
 * The method is to draw points in a pixmap with the specified shades
 * of primary colors and then get the corresponding XImage pixel
 * representation.
 * Thus we can get away with any Bit-order/Byte-order dependencies.
 */

static void screen_init(theDisp, theScreen, dispDEEP, theCmap)
  Display *theDisp;
  int theScreen;
  int dispDEEP;
  Colormap theCmap;
{
  Pixmap check_map;
  GC check_gc;
  XColor check_col;
  XImage *check_image;
  int ci, i;

  if (screen_bpp) return;

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
      XSetForeground(theDisp, check_gc, check_col.pixel);
      XDrawPoint(theDisp, check_map, check_gc, 0, 0);
      check_image = XGetImage(theDisp, check_map, 0, 0, 1, 1,
			      AllPlanes, ZPixmap);
      if (check_image) {
	/*
	 * If we have 16-bit output depth, then we double the value
	 * in the top word. This means that we can write out both
	 * pixels in the pixel doubling mode with one op. It is 
	 * harmless in the normal case as storing a 32-bit value
	 * through a short pointer will lose the top bits anyway.
	 * A similar optimisation for Alpha for 64 bit has been
	 * prepared for, but is not yet implemented.
	 */
	switch (check_image->bits_per_pixel) {
	case 8:
	  screen_rgb[ci][i] = *(UINT8 *)check_image->data;
	  break;
	case 16:
	  screen_rgb[ci][i] =
	    ((unsigned long)*(UINT16 *)check_image->data << 16) |
	    (unsigned long)*(UINT16 *)check_image->data;
	  break;
	case 24:
	  screen_rgb[ci][i] =
	    ((unsigned long)*(UINT8 *)check_image->data << 16) |
	    ((unsigned long)*(UINT8 *)(check_image->data + 1) << 8) |
	    (unsigned long)*(UINT8 *)(check_image->data + 2);
	  break;
	case 32:
#ifdef SIXTYFOUR_BIT
	  screen_rgb[ci][i] =
	    ((unsigned long)*(UINT32 *)check_image->data << 32) |
	    (unsigned long)*(UINT32 *)check_image->data;
#else
	  screen_rgb[ci][i] = *(UINT32 *)check_image->data;
#endif
	  break;
	}
	screen_bpp = check_image->bits_per_pixel;
	XDestroyImage(check_image);
      }
    }
  }
  XFreeGC(theDisp, check_gc);
  XFreePixmap(theDisp, check_map);
}


/*
 *--------------------------------------------------------------
 *
 * FindFullColorVisual
 *
 *  Returns a pointer to a full color bit visual on the display
 *
 * Results:
 *      See above.
 *  
 * Side effects:
 *      Unknown.
 *
 *--------------------------------------------------------------
 */
Visual *
FindFullColorVisual (dpy, depth)
     Display *dpy;
     int *depth;
{
  XVisualInfo vinfo;
  XVisualInfo *vinfo_ret;
  int numitems, maxdepth;
  
  vinfo.class = TrueColor;
  
  vinfo_ret = XGetVisualInfo(dpy, VisualClassMask, &vinfo, &numitems);
  
  if (numitems == 0) return NULL;

  maxdepth = 0;
  while(numitems > 0) {
    if (vinfo_ret[numitems-1].depth > maxdepth) {
      maxdepth = vinfo_ret[numitems-1 ].depth;
    }
    numitems--;
  }
  XFree((void *) vinfo_ret);

  if (XMatchVisualInfo(dpy, DefaultScreen(dpy), maxdepth, 
		       TrueColor, &vinfo)) {
    *depth = vinfo.depth;
    return vinfo.visual;
  }
  
  return NULL;
}


/*
 *--------------------------------------------------------------
 *
 * CreateFullColorWindow
 *
 *  Creates a window capable of handling 32 bit color.
 *
 * Results:
 *      See above.
 *  
 * Side effects:
 *      Unknown.
 *
 *--------------------------------------------------------------
 */
void
CreateFullColorWindow (xinfo)
     XInfo *xinfo;
{
  int depth;
  Visual *visual;
  XSetWindowAttributes xswa;
  unsigned long mask;
  unsigned int class;
  int screen;
  Display *dpy=xinfo->display;
  int x = xinfo->hints.x,
      y = xinfo->hints.y;
  unsigned int w = xinfo->hints.width,
               h = xinfo->hints.height;

  screen = DefaultScreen(dpy);
  if (xinfo->visual == NULL)
    if ((xinfo->visual = FindFullColorVisual(dpy, &xinfo->depth)) == NULL)
      return;

  visual = xinfo->visual;
  depth = xinfo->depth;

  if (xinfo->cmap == 0)
    xinfo->cmap = XCreateColormap(dpy, XRootWindow(dpy, screen),
                                  visual, AllocNone);

  screen_init(dpy, screen, depth, xinfo->cmap);

  class = InputOutput;	/* Could be InputOnly */
  mask = CWBackPixel | CWColormap | CWBorderPixel;
  xswa.colormap = xinfo->cmap;
  xswa.background_pixel = BlackPixel(dpy, screen);
  xswa.border_pixel = WhitePixel(dpy, screen);

  xinfo->window = XCreateWindow(dpy, RootWindow(dpy, screen), x, y, w, h,
    (unsigned int) 1, depth, class, visual, mask, &xswa);
}
