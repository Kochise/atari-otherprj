/*
//  Source code for generic Win32 JPEG DLL
//  This DLL encapsulates the libJPEG functions
//
//  Copyright 1998 M. Scott Heiman
//  All Rights Reserved
//
// You may use the software for any purpose you see fit.  You may modify
// it, incorporate it in a commercial application, use it for school,
// even turn it in as homework.  You must keep the Copyright in the
// header and source files.  This software is not in the "Public Domain".
// You may use this software at your own risk.  I have made a reasonable
// effort to verify that this software works in the manner I expect it to;
// however,...
//
// THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS" AND
// WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE, INCLUDING
// WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR FITNESS FOR A
// PARTICULAR PURPOSE. IN NO EVENT SHALL MICHAEL S. HEIMAN BE LIABLE TO
// YOU OR ANYONE ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL, INDIRECT OR
// CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING
// WITHOUT LIMITATION, LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE,
// OR THE CLAIMS OF THIRD PARTIES, WHETHER OR NOT MICHAEL S. HEIMAN HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
// POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Portions of this code were copied from the tiff2dib.c written by
// Philippe Tenenhaus.  The tiff2dib file was bundled with the tifflib
// package
//
// Other portions were blatantly stolen from example.c that was
// bundled with the libjpeg source.
*/

#include <stdio.h>
#include <setjmp.h>

#include "stdafx.h"

#include "jpeglib.h"

/* error handling structures  */

struct win_jpeg_error_mgr {
  struct jpeg_error_mgr pub;    /* public fields */
  jmp_buf setjmp_buffer;        /* for return to caller */
};

typedef struct win_jpeg_error_mgr *error_ptr;

METHODDEF(noreturn_t) win_jpeg_error_exit( j_common_ptr cinfo )
{
  error_ptr myerr = (error_ptr)cinfo->err;
  (*cinfo->err->output_message)(cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}

/* helper functions and macros */
#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))

HANDLE CreateDIB(DWORD dwWidth, DWORD dwHeight, WORD wBitCount);
static WORD PaletteSize(LPSTR lpDIB);
static WORD DIBNumColors(LPSTR lpDIB);
static LPSTR FindDIBBits(LPSTR lpDIB);

/* Read_JPEG_File
// 1) reads the data from a JPEG file
// 2) creates an HDIB and stores the data in it
// 3) returns the HDIB
// portions of this subroutine copied from example.c in the libjpeg source
// and from the libtiff examples */
HANDLE Read_JPEG_File( const char *filename, int scale, int grayscale,
                       int *image_width, int *image_height,
                       int *MCUwidth, int *MCUheight,
                       int *scale_denom )
{
  struct jpeg_decompress_struct cinfo;
  struct win_jpeg_error_mgr jerr;
  HANDLE hDib = 0;
  LPBITMAPINFO lpbmi;
  LPBITMAPINFOHEADER lpDIB;
  LPSTR lpBits;
  int DIBComponents;
  int DIBLineWidth;
  int DIBScanWidth;
  int i;
  JSAMPROW buffer[1];
  JSAMPARRAY exbuf;
  JDIMENSION j;
  FILE *infile;

  infile = fopen( filename, "rb" );
  if ( !infile )
    return 0;

/* overide the error_exit handler */
  cinfo.err = jpeg_std_error( &jerr.pub );
  jerr.pub.error_exit = win_jpeg_error_exit;
/* if this is true some weird error has already occured */
  if ( setjmp(jerr.setjmp_buffer) ) {
    if ( hDib ) {
      GlobalUnlock( hDib );
      if ( cinfo.output_scanline < cinfo.output_height ) {
        GlobalFree( hDib );
        hDib = 0;
      }
    }
    jpeg_destroy_decompress( &cinfo );
    fclose( infile );
    return hDib;
  }

/* initialize the jpeg decompression object */
  jpeg_create_decompress( &cinfo );

/* specify the data source */
  jpeg_stdio_src( &cinfo, infile );

/* read file parameters */
  jpeg_read_header( &cinfo, TRUE );

/* set parameters for decompression
   add here if needed */
  if (scale)
    cinfo.scale_num = scale;

  if (grayscale) /* force monochrome output */
    if (cinfo.jpeg_color_space == JCS_YCCK)
      cinfo.out_color_space = JCS_YCCK;
    else
      cinfo.out_color_space = JCS_GRAYSCALE;

/* start decompressor */
  jpeg_start_decompress( &cinfo );

  *image_width = (int) cinfo.image_width;
  *image_height = (int) cinfo.image_height;
  *MCUwidth = cinfo.max_h_samp_factor * cinfo.scale_denom;
  *MCUheight = cinfo.max_v_samp_factor * cinfo.scale_denom;
  *scale_denom = cinfo.scale_denom;

/* check for supported output color space */
  switch (cinfo.out_color_space) {
  case JCS_GRAYSCALE:
  case JCS_YCCK:
    DIBComponents = 1;
    break;
  case JCS_RGB:
  case JCS_CMYK:
    DIBComponents = 3;
    break;
  default:
    jpeg_destroy_decompress( &cinfo );
    fclose( infile );
    MessageBox(0,"Unsupported output color space","JPEG Error",
               MB_OK | MB_ICONEXCLAMATION );
    return 0;
  }

/* we now have the correct dimensions in the cinfo structure.
   Let's create an HDIB to hold the data */
  hDib = CreateDIB((DWORD)cinfo.output_width, (DWORD)cinfo.output_height,
                   (WORD)(8*DIBComponents));

  lpDIB = (LPBITMAPINFOHEADER) GlobalLock(hDib);
  if ( !lpDIB ) {
    jpeg_destroy_decompress( &cinfo );
    fclose( infile );
    MessageBox(0,"Failed to create a DIB Handle","JPEG Error",
               MB_OK | MB_ICONEXCLAMATION );
    return 0;
  }

/* if this is a grayscale image, initialize the palette */
  if (DIBComponents == 1) {
    *MCUwidth = cinfo.scale_denom;
    *MCUheight = cinfo.scale_denom;
    lpbmi = (LPBITMAPINFO) lpDIB;
    for ( i = 0; i < 256; i++ ) {
      lpbmi->bmiColors[i].rgbRed   = (BYTE)i;
      lpbmi->bmiColors[i].rgbGreen = (BYTE)i;
      lpbmi->bmiColors[i].rgbBlue  = (BYTE)i;
    }
  }

/* if this is a CMYK/YCCK image, allocate extra buffer */
  if (cinfo.output_components == 4)
    exbuf = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE,
		 cinfo.output_width * 4, 1);

/* point to the place where the pixel data starts and determine the
   number of bytes in each DIB scan line */
  lpBits = FindDIBBits((LPSTR) lpDIB);
  DIBLineWidth = cinfo.output_width * DIBComponents;
  DIBScanWidth = (DIBLineWidth + 3) & (-4);
/* point to the bottom row of the DIB data.  DIBs are stored bottom-to-top,
   JPEGS are stored top-to-bottom. */
  lpBits += DIBScanWidth * cinfo.output_height;

/* loop through the decompression object and store the data in the DIB */
  while ( cinfo.output_scanline < cinfo.output_height ) {
    lpBits -= DIBScanWidth;
    switch (cinfo.out_color_space) {
    case JCS_GRAYSCALE:
      *buffer = (JSAMPROW) lpBits;
      jpeg_read_scanlines( &cinfo, buffer, 1 );
      break;
    case JCS_RGB:
      *buffer = (JSAMPROW) lpBits;
      jpeg_read_scanlines( &cinfo, buffer, 1 );
      for ( i = 0; i < DIBLineWidth; i += 3 ) {
        BYTE temp = lpBits[i];
        lpBits[i] = lpBits[i + 2];
        lpBits[i + 2] = temp;
      }
      break;
    case JCS_CMYK:
      jpeg_read_scanlines( &cinfo, exbuf, 1 );
      for ( j = 0; j < cinfo.output_width; j++ ) {
	int k = GETJSAMPLE(exbuf[0][4*j + 3]);
        lpBits[3*j]     = (BYTE)
	  ((GETJSAMPLE(exbuf[0][4*j + 2]) * k) / MAXJSAMPLE);
        lpBits[3*j + 1] = (BYTE)
	  ((GETJSAMPLE(exbuf[0][4*j + 1]) * k) / MAXJSAMPLE);
        lpBits[3*j + 2] = (BYTE)
	  ((GETJSAMPLE(exbuf[0][4*j]) * k) / MAXJSAMPLE);
      }
      break;
    default: /* YCCK */
      jpeg_read_scanlines( &cinfo, exbuf, 1 );
      for ( i = 0; i < DIBLineWidth; i++ ) {
        lpBits[i] = (BYTE)
	  (((MAXJSAMPLE - GETJSAMPLE(exbuf[0][4*i])) *
	    GETJSAMPLE(exbuf[0][4*i + 3])) / MAXJSAMPLE);
      }
      break;
    }
  }

/* finish decompression */
  jpeg_finish_decompress( &cinfo );

  GlobalUnlock( hDib );

/* release decompression object */
  jpeg_destroy_decompress( &cinfo );

  fclose( infile );
  return hDib;
}

/**********************************************************************/
/*************************************************************************
 * All the following functions were created by microsoft, they are
 * parts of the sample project "wincap" given with the SDK Win32.
 *
 * Microsoft says that :
 *
 *  You have a royalty-free right to use, modify, reproduce and
 *  distribute the Sample Files (and/or any modified version) in
 *  any way you find useful, provided that you agree that
 *  Microsoft has no warranty obligations or liability for any
 *  Sample Application Files which are modified.
 *
 ************************************************************************/

HANDLE CreateDIB(DWORD dwWidth, DWORD dwHeight, WORD wBitCount)
{
  BITMAPINFOHEADER bi;         /* bitmap header */
  LPBITMAPINFOHEADER lpbi;     /* pointer to BITMAPINFOHEADER */
  DWORD dwLen;                 /* size of memory block */
  HANDLE hDIB;
  DWORD dwBytesPerLine;        /* Number of bytes per scanline */


   /* Make sure bits per pixel is valid */
  if (wBitCount <= 1)
      wBitCount = 1;
  else if (wBitCount <= 4)
      wBitCount = 4;
  else if (wBitCount <= 8)
      wBitCount = 8;
  else if (wBitCount <= 24)
      wBitCount = 24;
  else
      wBitCount = 4;  /* set default value to 4 if parameter is bogus */

  /* initialize BITMAPINFOHEADER */
  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = dwWidth;         /* fill in width from parameter */
  bi.biHeight = dwHeight;       /* fill in height from parameter */
  bi.biPlanes = 1;              /* must be 1 */
  bi.biBitCount = wBitCount;    /* from parameter */
  bi.biCompression = BI_RGB;
/* 0's here mean "default"  */
  bi.biSizeImage = (dwWidth*dwHeight*wBitCount)/8;
  bi.biXPelsPerMeter = 0;
  bi.biYPelsPerMeter = 0;
  bi.biClrUsed = 0;
  bi.biClrImportant = 0;

  /* calculate size of memory block required to store the DIB.  This
     block should be big enough to hold the BITMAPINFOHEADER, the color
     table, and the bits */

  dwBytesPerLine = (((wBitCount * dwWidth) + 31) / 32 * 4);
  dwLen = bi.biSize + PaletteSize((LPSTR)&bi) + dwBytesPerLine * dwHeight;

  /* alloc memory block to store our bitmap  */
  hDIB = GlobalAlloc(GHND, dwLen);

  /* major bummer if we couldn't get memory block */
  if (hDIB == NULL) return NULL;

  /* lock memory and get pointer to it  */
  lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

  /* use our bitmap info structure to fill in first part of
     our DIB with the BITMAPINFOHEADER */
  *lpbi = bi;

  /* Since we don't know what the colortable and bits should contain,
     just leave these blank.  Unlock the DIB and return the HDIB. */

  GlobalUnlock(hDIB);

  /* return handle to the DIB */
  return hDIB;
}


WORD FAR PaletteSize(LPSTR lpDIB)
{
  /* calculate the size required by the palette */
  if (IS_WIN30_DIB (lpDIB))
      return (WORD)(DIBNumColors(lpDIB) * sizeof(RGBQUAD));
  else
      return (WORD)(DIBNumColors(lpDIB) * sizeof(RGBTRIPLE));
}


WORD DIBNumColors(LPSTR lpDIB)
{
  WORD wBitCount;  /* DIB bit count  */

  /*  If this is a Windows-style DIB, the number of colors in the
   *  color table can be less than the number of bits per pixel
   *  allows for (i.e. lpbi->biClrUsed can be set to some value).
   *  If this is the case, return the appropriate value.
   */

  if (IS_WIN30_DIB(lpDIB))
  {
    DWORD dwClrUsed;

    dwClrUsed = ((LPBITMAPINFOHEADER)lpDIB)->biClrUsed;
    if (dwClrUsed)
        return (WORD)dwClrUsed;
  }

  /*  Calculate the number of colors in the color table based on
   *  the number of bits per pixel for the DIB.
   */
  if (IS_WIN30_DIB(lpDIB))
      wBitCount = ((LPBITMAPINFOHEADER)lpDIB)->biBitCount;
  else
      wBitCount = ((LPBITMAPCOREHEADER)lpDIB)->bcBitCount;

  /* return number of colors based on bits per pixel */
  switch (wBitCount)
  {
    case 1:
      return 2;

    case 4:
      return 16;

    case 8:
      return 256;

    default:
      return 0;
  }
}

LPSTR FindDIBBits(LPSTR lpDIB)
{
  return (lpDIB + *(LPDWORD)lpDIB + PaletteSize(lpDIB));
}
