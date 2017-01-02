#ifndef DOCROP_H
#define DOCROP_H

int docrop( const char * src, const char * des,
			int crop_xoffset, int crop_yoffset,
			int crop_width, int crop_height,
			int transform, int force_grayscale, int scale,
			int optimize_coding,
			int crop_copyoption,
			int processing_mode,
			int copyfiletime,
			int progressive);

#endif  /* DOCROP_H */
