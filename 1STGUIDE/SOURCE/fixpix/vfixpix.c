/*
 * vfixpix.c
 *
 * Sample implementation of FixPix routines for the Virtual Device
 * Interface (VDI) under GEM (Graphic environment manager).
 * Currently leaned on the 1stGuide context, but should be easy to
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
 * The method is to draw points in the upper left corner on the
 * screen with the specified shades of primary colors and then
 * get the corresponding pixel representation.
 * Thus we can get away with any Bit-order/Byte-order dependencies.
 *
 * The routine uses the global phys_handle variable (returned from
 * graf_handle()). Adapt this to your application as necessary.
 * I've not passed it in as parameter, since for other platforms
 * it may be different (see xfixpix.c), and so the screen_init()
 * interface is unique.
 */

void screen_init(void)
{
  static int init_flag = 0;
  static unsigned long pix_buf = 0;
  static int pxy[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  static MFDB screen = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  static MFDB check = { screen_rgb, 1, 1, 1, 0, 0, 0, 0, 0 };
  static MFDB save = { &pix_buf, 1, 1, 1, 0, 0, 0, 0, 0 };
  int work_in[11], work_out[57], rgb[3];
  int check_handle, ci, i;

  if (init_flag) return;
  init_flag = 1;

  check_handle = phys_handle;
  for (i = 0; i < 10; i++) work_in[i] = 1;
  work_in[10] = 2;
  v_opnvwk(work_in, &check_handle, work_out);
  vq_extnd(check_handle, 1, work_out);
  save.fd_nplanes = check.fd_nplanes = work_out[4];
  vro_cpyfm(check_handle, S_ONLY, pxy, &screen, &save);
  for (ci = 0; ci < 3; ci++) {
    for (i = 0; i < 256; i++) {
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 0;
      /* Do proper upscaling from unsigned 8 bit (image data values)
	 to the 0...1000 range (VDI color representation).
	 This would be scaling *1000/255 = *200/51, what can be done
	 in 16 bit unsigned integer arithmetic. */
      rgb[ci] = ((unsigned)i * 200U) / 51U;
      vs_color(check_handle, 1, rgb);
      if (check.fd_nplanes < 24) {
	vq_color(check_handle, 1, 1, rgb); /* get exact representation */
	screen_set[ci][i] = ((unsigned)rgb[ci] * 51U + 100U) / 200U;
      }
      v_pmarker(check_handle, 1, pxy);
      vro_cpyfm(check_handle, S_ONLY, pxy, &screen, &check);
      ++(unsigned long *)check.fd_addr;
    }
  }
  vro_cpyfm(check_handle, S_ONLY, pxy, &save, &screen);
  v_clsvwk(check_handle);
}
