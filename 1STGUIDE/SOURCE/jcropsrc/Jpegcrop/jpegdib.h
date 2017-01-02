/*
 * header file for generic JPEG DLL
 * Two functions are exported from the DLL:
 * 1) Read_JPEG_File opens a JPEG file and returns the contents in a DIB
 * 2) Write_JPEG_File stores the contents of a DIB in a JPEG file
 */

#ifndef _JPEGDLL_
#define _JPEGDLL_

HANDLE Read_JPEG_File( const char *filename, int scale, int grayscale,
					   int *image_width, int *image_height,
					   int *MCUwidth, int *MCUheight,
					   int *scale_denom );

#endif    /* _JPEGDLL_ */
