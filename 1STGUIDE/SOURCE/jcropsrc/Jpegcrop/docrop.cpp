#include <stdio.h>
#include <setjmp.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include "jpeglib.h"
extern "C" {
#include "transupp.h"
}

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

static JCOPY_OPTION copyoption;	/* -copy switch */
static jpeg_transform_info transformoption; /* image transformation options */

static struct _stat statbuf;
static struct _utimbuf timebuf;
static int status;

int docrop( const char * src, const char * des,
            int crop_xoffset, int crop_yoffset,
            int crop_width, int crop_height,
            int transform, int force_grayscale, int scale,
            int optimize_coding,
            int crop_copyoption,
            int processing_mode,
            int copyfiletime,
            int progressive)
{
  struct jpeg_decompress_struct srcinfo;
  struct jpeg_compress_struct dstinfo;
  struct win_jpeg_error_mgr jsrcerr, jdsterr;
  jvirt_barray_ptr * src_coef_arrays;
  jvirt_barray_ptr * dst_coef_arrays;
  /* We assume all-in-memory processing and can therefore use only a
   * single file pointer for sequential input and output operation. 
   */
  static FILE * fp = NULL;

  /* Set up parameters. */
  copyoption = (JCOPY_OPTION)crop_copyoption;
  transformoption.transform = (JXFORM_CODE)transform;
  transformoption.perfect = FALSE;
  transformoption.trim = FALSE;
  transformoption.force_grayscale = (boolean)force_grayscale;
  transformoption.crop = TRUE;
  transformoption.crop_xoffset = crop_xoffset;
  transformoption.crop_xoffset_set = JCROP_POS;
  transformoption.crop_yoffset = crop_yoffset;
  transformoption.crop_yoffset_set = JCROP_POS;
  transformoption.crop_width = crop_width;
  transformoption.crop_width_set = JCROP_POS;
  transformoption.crop_height = crop_height;
  transformoption.crop_height_set = JCROP_POS;

/* overide the error_exit handler */
  srcinfo.err = jpeg_std_error( &jsrcerr.pub );
  jsrcerr.pub.error_exit = win_jpeg_error_exit;
  dstinfo.err = jpeg_std_error( &jdsterr.pub );
  jdsterr.pub.error_exit = win_jpeg_error_exit;
/* if this is true some weird error has already occured */
  if ( setjmp(jsrcerr.setjmp_buffer) ) {
    jpeg_destroy_decompress( &srcinfo );
    jpeg_destroy_compress( &dstinfo );
    if (fp) fclose( fp );
    return 0;
  }
  if ( setjmp(jdsterr.setjmp_buffer) ) {
    jpeg_destroy_decompress( &srcinfo );
    jpeg_destroy_compress( &dstinfo );
    if (fp) fclose( fp );
    return 0;
  }

  jpeg_create_decompress(&srcinfo);
  jpeg_create_compress(&dstinfo);

  if (copyfiletime) {
      status = _stat(src, &statbuf);
  }

  /* Open the input file. */
  if ((fp = fopen(src, "rb")) == NULL) {
//      fprintf(stderr, "can't open %s\n", src);
      win_jpeg_error_exit((j_common_ptr)&srcinfo);
  }

  /* Specify data source for decompression */
  jpeg_stdio_src(&srcinfo, fp);

  /* Enable saving of extra markers that we want to copy */
  jcopy_markers_setup(&srcinfo, copyoption);

  /* Read file header */
  (void) jpeg_read_header(&srcinfo, TRUE);

  if (scale)
      srcinfo.scale_num = scale;

  /* Any space needed by a transform option must be requested before
   * jpeg_read_coefficients so that memory allocation will be done right.
   */
#if TRANSFORMS_SUPPORTED
  (void) jtransform_request_workspace(&srcinfo, &transformoption);
#endif

  /* Read source file as DCT coefficients */
  src_coef_arrays = jpeg_read_coefficients(&srcinfo);

  if (processing_mode == 0 && jsrcerr.pub.num_warnings) {
      win_jpeg_error_exit((j_common_ptr)&srcinfo);
  }

  /* Initialize destination compression parameters from source values */
  jpeg_copy_critical_parameters(&srcinfo, &dstinfo);

  /* Adjust destination parameters if required by transform options;
   * also find out which set of coefficient arrays will hold the output.
   */
#if TRANSFORMS_SUPPORTED
  dst_coef_arrays = jtransform_adjust_parameters(&srcinfo, &dstinfo,
                                                 src_coef_arrays,
                                                 &transformoption);
#else
  dst_coef_arrays = src_coef_arrays;
#endif

  if (optimize_coding == 2)
      dstinfo.arith_code = TRUE;
  else
      dstinfo.optimize_coding = (boolean)optimize_coding;

  if (progressive)
      jpeg_simple_progression(&dstinfo);

  /* Close input file.
   * Note: we assume that jpeg_read_coefficients consumed all input
   * until JPEG_REACHED_EOI, and that jpeg_finish_decompress will
   * only consume more while (! cinfo->inputctl->eoi_reached).
   * We cannot call jpeg_finish_decompress here since we still need the
   * virtual arrays allocated from the source object for processing.
   */
  fclose(fp);

  /* Open the output file. */
  if ((fp = fopen(des, "wb")) == NULL) {
//      fprintf(stderr, "can't open %s\n", des);
      win_jpeg_error_exit((j_common_ptr)&dstinfo);
  }

  /* Specify data destination for compression */
  jpeg_stdio_dest(&dstinfo, fp);

  /* Start compressor (note no image data is actually written here) */
  jpeg_write_coefficients(&dstinfo, dst_coef_arrays);

  /* Copy to the output file any extra markers that we want to preserve */
  jcopy_markers_execute(&srcinfo, &dstinfo, copyoption);

  /* Execute image transformation, if any */
#if TRANSFORMS_SUPPORTED
  jtransform_execute_transformation(&srcinfo, &dstinfo,
                                    src_coef_arrays,
                                    &transformoption);
#endif

  /* Finish compression and release memory */
  jpeg_finish_compress(&dstinfo);
  jpeg_destroy_compress(&dstinfo);
  (void) jpeg_finish_decompress(&srcinfo);
  jpeg_destroy_decompress(&srcinfo);

  /* Close output file */
  fclose(fp);

  if (copyfiletime) {
    if (status == 0) {
      timebuf.actime = statbuf.st_atime;
      timebuf.modtime = statbuf.st_mtime;
      _utime(des, &timebuf);
    }
  }

  /* All done. */
  if (processing_mode == 0 &&
      jsrcerr.pub.num_warnings + jdsterr.pub.num_warnings)
      return 0;
  return 1;
}
