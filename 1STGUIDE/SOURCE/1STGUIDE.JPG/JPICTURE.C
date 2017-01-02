#include <aes.h>
#include <vdi.h>
#include <errno.h>
#include <setjmp.h>
#include <osbind.h>

#include "jinclude.h"
#include "1stguide.h"
#include "util.h"

typedef struct guide
{
  int	handle, width, height, planes,
	lbc,			/* left-border-count fr Fenster.	*/
	tlc,			/* top-line-count fr Fenster.		*/
	bc,			/* border-count fr Fenster.		*/
	lc,			/* line-count fr Fenster.		*/
	xfac, yfac,		/* Scroll- und Rastereinheiten.		*/
	x, y, w, h,		/* Arbeitsbereich des Fensters.		*/
	hslide, vslide,		/* Puffer fr Sliderpositionen.		*/
	(*key)( struct guide *tg, int code, int ks );
  void	(*draw)( struct guide *tg, int *clip ),
	(*free)( struct guide *tg ),
	(*sclick)( struct guide *tg, int mx, int my ),
	(*dclick)( struct guide *tg, int mx, int my, int ks );
  char	path[128],		/* Pfad fr Datei ”ffnen.		*/
	name[128],		/* Informationspfad (Fenstertitel).	*/
	hist[256];		/* History-Buffer.			*/
  struct guide *next;		/* Zeiger auf n„chste Fensterstruktur.	*/
  struct guide *prev;		/* Zeiger auf vorige Fensterstruktur.	*/
  BASPAG *actpd;		/* Zeiger auf Prozess-Descriptor.	*/

  MFDB	mfdb;
  int	x_flag, scale,
	rgb_list[8];		/* Farbpalette.				*/
}
GUIDE;

typedef struct
{
  struct Decompress_info_struct cinfo;
  struct External_methods_struct e_methods;
  GUIDE *rg;
  int handle, EOF_flag;
  long buf_size, length;
  unsigned char *input_buffer;
  unsigned char fbuf[512];
}
MY_JPGD;

FROM( util ) IMPORT jmp_buf setjmp_buffer;

FROM( image ) IMPORT unsigned char Pli2Vdi[256];
FROM( image ) IMPORT void setconv( void );
FROM( image ) IMPORT void draw_image();
FROM( image ) IMPORT int key_image();
FROM( image ) IMPORT void sclick_image();
FROM( image ) IMPORT void dummy_image();

static MFORM achtel[] =
{
{ 8, 8, 1, 0, 1,
  0x07E0, 0x1FF8, 0x3FFC, 0x7FFE, 0x7FFE, 0xFFFF, 0xFFFF, 0xFFFF,
  0xFFFF, 0xFFFF, 0xFFFF, 0x7FFE, 0x7FFE, 0x3FFC, 0x1FF8, 0x07E0,
  0x0700, 0x18E0, 0x20F8, 0x40FA, 0x40F2, 0x80E1, 0x80C1, 0x8081,
  0x8001, 0x8001, 0x8001, 0x4002, 0x4002, 0x2004, 0x1818, 0x07E0
},
{ 8, 8, 1, 0, 1,
  0x07E0, 0x1FF8, 0x3FFC, 0x7FFE, 0x7FFE, 0xFFFF, 0xFFFF, 0xFFFF,
  0xFFFF, 0xFFFF, 0xFFFF, 0x7FFE, 0x7FFE, 0x3FFC, 0x1FF8, 0x07E0,
  0x0700, 0x18E0, 0x20F8, 0x40FC, 0x40FC, 0x80FE, 0x80FE, 0x80FE,
  0x8001, 0x8001, 0x8001, 0x4002, 0x4002, 0x2004, 0x1818, 0x07E0
},
{ 8, 8, 1, 0, 1,
  0x07E0, 0x1FF8, 0x3FFC, 0x7FFE, 0x7FFE, 0xFFFF, 0xFFFF, 0xFFFF,
  0xFFFF, 0xFFFF, 0xFFFF, 0x7FFE, 0x7FFE, 0x3FFC, 0x1FF8, 0x07E0,
  0x0700, 0x18E0, 0x20F8, 0x40FC, 0x40FC, 0x80FE, 0x80FE, 0x80FE,
  0x80FE, 0x807E, 0x803E, 0x401C, 0x400C, 0x2000, 0x1818, 0x07E0
},
{ 8, 8, 1, 0, 1,
  0x07E0, 0x1FF8, 0x3FFC, 0x7FFE, 0x7FFE, 0xFFFF, 0xFFFF, 0xFFFF,
  0xFFFF, 0xFFFF, 0xFFFF, 0x7FFE, 0x7FFE, 0x3FFC, 0x1FF8, 0x07E0,
  0x0700, 0x18E0, 0x20F8, 0x40FC, 0x40FC, 0x80FE, 0x80FE, 0x80FE,
  0x80FE, 0x80FE, 0x80FE, 0x40FC, 0x40FC, 0x20F8, 0x18E0, 0x0700
},
{ 8, 8, 1, 0, 1,
  0x07E0, 0x1FF8, 0x3FFC, 0x7FFE, 0x7FFE, 0xFFFF, 0xFFFF, 0xFFFF,
  0xFFFF, 0xFFFF, 0xFFFF, 0x7FFE, 0x7FFE, 0x3FFC, 0x1FF8, 0x07E0,
  0x0700, 0x18E0, 0x20F8, 0x40FC, 0x40FC, 0x80FE, 0x80FE, 0x80FE,
  0x81FE, 0x83FE, 0x87FE, 0x4FFC, 0x5FFC, 0x1FF8, 0x07E0, 0x0000
},
{ 8, 8, 1, 0, 1,
  0x07E0, 0x1FF8, 0x3FFC, 0x7FFE, 0x7FFE, 0xFFFF, 0xFFFF, 0xFFFF,
  0xFFFF, 0xFFFF, 0xFFFF, 0x7FFE, 0x7FFE, 0x3FFC, 0x1FF8, 0x07E0,
  0x0700, 0x18E0, 0x20F8, 0x40FC, 0x40FC, 0x80FE, 0x80FE, 0x80FE,
  0x7FFE, 0x7FFE, 0x7FFE, 0x3FFC, 0x3FFC, 0x1FF8, 0x07E0, 0x0000
},
{ 8, 8, 1, 0, 1,
  0x07E0, 0x1FF8, 0x3FFC, 0x7FFE, 0x7FFE, 0xFFFF, 0xFFFF, 0xFFFF,
  0xFFFF, 0xFFFF, 0xFFFF, 0x7FFE, 0x7FFE, 0x3FFC, 0x1FF8, 0x07E0,
  0x0700, 0x18E0, 0x00F8, 0x30FC, 0x38FC, 0x7CFE, 0x7EFE, 0x7FFE,
  0x7FFE, 0x7FFE, 0x7FFE, 0x3FFC, 0x3FFC, 0x1FF8, 0x07E0, 0x0000
}
};

static JSAMPLE screen_set[3][MAXJSAMPLE+1];
static long screen_rgb[3][MAXJSAMPLE+1];
static void *image_ptr;
static long plane_size;

static void set_screen( void )
{
  int i, ci, rgb[3];
  static int pxy[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  static long pix_buf = 0;
  static MFDB screen = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	      check = { screen_rgb, 1, 1, 1, 0, 0, 0, 0, 0 },
	      save = { &pix_buf, 1, 1, 1, 0, 0, 0, 0, 0 };

  if (check.fd_nplanes) return;
  save.fd_nplanes = check.fd_nplanes = nplanes;
  vro_cpyfm( handle, S_ONLY, pxy, &screen, &save );
  ci = 0;
  do
  { i = 0;
    do
    { *(long *)rgb = 0; rgb[2] = 0;
#if MAXJSAMPLE == 256
      rgb[ci] = ((unsigned)i * 125U) >> 5;
#else
      rgb[ci] = (int)(((long)i * 1000) / MAXJSAMPLE);
#endif
      vs_color( handle, 1, rgb );
      if (check.fd_nplanes < 24)
      {
	vq_color( handle, 1, 1, rgb );
	screen_set[ci][i] =
#if MAXJSAMPLE == 256
	  (JSAMPLE)((((unsigned)rgb[ci] << 6) + 125U) / 250U);
#else
	  (JSAMPLE)(((long)rgb[ci] * MAXJSAMPLE + 500) / 1000);
#endif
      }
      v_pmarker( handle, 1, pxy );
      vro_cpyfm( handle, S_ONLY, pxy, &screen, &check );
      ++(long *)check.fd_addr;
    }
    while (++i <= MAXJSAMPLE);
  }
  while (++ci <= 2);
  vro_cpyfm( handle, S_ONLY, pxy, &save, &screen );
  vs_color( handle, 1, pxy );
}

static void set_color_map( JSAMPROW get_map )
{
  int i, ci, r = 0, g = 0, b = 0, rgb[3];

  for (i = 0;;i++)
  {
    rgb[0] = r; rgb[1] = g; rgb[2] = b; ci = Pli2Vdi[32 + i];
    vs_color( handle, ci, rgb );
    if (get_map)
    {
      vq_color( handle, ci, 1, rgb ); ci = 2;
      do get_map[(MAXJSAMPLE+1)*ci+i] =
#if MAXJSAMPLE == 256
	  (JSAMPLE)((((unsigned)rgb[ci] << 6) + 125U) / 250U);
#else
	  (JSAMPLE)(((long)rgb[ci] * MAXJSAMPLE + 500) / 1000);
#endif
      while (--ci >= 0);
    }
    if ((b += 200) > 1000)
    {
      b = 0;
      if ((g += 200) > 1000)
      {
	g = 0;
	if ((r += 200) > 1000) break;
} } } }

static void gray16_put_pixel_rows( decompress_info_ptr cinfo,
				int num_rows, JSAMPIMAGE pixel_data )
{
  long col, width = cinfo->image_width;
  JSAMPROW ptr, *pix_arr = *pixel_data;
  int i, val, *raster_ptr;

  raster_ptr = image_ptr;
  do
  { ptr = *pix_arr++; col = width;
    do
    { i = 0; val = *ptr++;
      i |= *(int *)&screen_rgb[0][val];
      i |= *(int *)&screen_rgb[1][val];
      i |= *(int *)&screen_rgb[2][val];
      *raster_ptr++ = i;
    }
    while (--col > 0);
    raster_ptr += (col -= width) & 15;
  }
  while (--num_rows > 0);
  image_ptr = raster_ptr;
}

static void rgbtrue16_put_pixel_rows( decompress_info_ptr cinfo,
				int num_rows, JSAMPIMAGE pixel_data )
{
  long row, col, width = cinfo->image_width;
  JSAMPROW ptr0, ptr1, ptr2;
  int *raster_ptr;

  raster_ptr = image_ptr; row = 0;
  do
  { ptr0 = pixel_data[0][row];
    ptr1 = pixel_data[1][row];
    ptr2 = pixel_data[2][row]; col = width;
    do *raster_ptr++ =
	 *(int *)&screen_rgb[0][*ptr0++] |
	 *(int *)&screen_rgb[1][*ptr1++] |
	 *(int *)&screen_rgb[2][*ptr2++];
    while (--col > 0);
    raster_ptr += (col -= width) & 15;
  }
  while (++(int)row < num_rows);
  image_ptr = raster_ptr;
}

static void gray24_put_pixel_rows( decompress_info_ptr cinfo,
				int num_rows, JSAMPIMAGE pixel_data )
{
  long i, j, col, width = cinfo->image_width;
  JSAMPROW ptr, *pix_arr = *pixel_data;
  char *raster_ptr;

  raster_ptr = image_ptr;
  do
  { ptr = *pix_arr++; col = width;
    do
    { int k;
      i = 0; k = *ptr++;
      i |= screen_rgb[0][k];
      i |= screen_rgb[1][k];
      i |= screen_rgb[2][k];
      i >>= 8; j = i >> 8; k = (int)j >> 8;
      *raster_ptr++ = k;
      *raster_ptr++ = j;
      *raster_ptr++ = i;
    }
    while (--col > 0);
    i = (col -= width) & 15;
    raster_ptr += i;
    raster_ptr += i;
    raster_ptr += i;
  }
  while (--num_rows > 0);
  image_ptr = raster_ptr;
}

static void rgbtrue24_put_pixel_rows( decompress_info_ptr cinfo,
				int num_rows, JSAMPIMAGE pixel_data )
{
  long i, j, k, col, row, width = cinfo->image_width;
  JSAMPROW ptr0, ptr1, ptr2;
  char *raster_ptr;

  raster_ptr = image_ptr; row = 0;
  do
  { ptr0 = pixel_data[0][row];
    ptr1 = pixel_data[1][row];
    ptr2 = pixel_data[2][row]; col = width;
    do
    { i  = screen_rgb[0][*ptr0++];
      i |= screen_rgb[1][*ptr1++];
      i |= screen_rgb[2][*ptr2++];
      i >>= 8; j = i >> 8; k = j >> 8;
      *raster_ptr++ = k;
      *raster_ptr++ = j;
      *raster_ptr++ = i;
    }
    while (--col > 0);
    i = (col -= width) & 15;
    raster_ptr += i;
    raster_ptr += i;
    raster_ptr += i;
  }
  while (++(int)row < num_rows);
  image_ptr = raster_ptr;
}

static void gray32_put_pixel_rows( decompress_info_ptr cinfo,
				int num_rows, JSAMPIMAGE pixel_data )
{
  long col, width = cinfo->image_width;
  JSAMPROW ptr, *pix_arr = *pixel_data;
  long i, *raster_ptr;

  raster_ptr = image_ptr;
  do
  { ptr = *pix_arr++; col = width;
    do
    { int val;
      i = 0; val = *ptr++;
      i |= screen_rgb[0][val];
      i |= screen_rgb[1][val];
      i |= screen_rgb[2][val];
      *raster_ptr++ = i;
    }
    while (--col > 0);
    raster_ptr += (col -= width) & 15;
  }
  while (--num_rows > 0);
  image_ptr = raster_ptr;
}

static void rgbtrue32_put_pixel_rows( decompress_info_ptr cinfo,
				int num_rows, JSAMPIMAGE pixel_data )
{
  long col, row, width = cinfo->image_width;
  JSAMPROW ptr0, ptr1, ptr2;
  long *raster_ptr;

  raster_ptr = image_ptr; row = 0;
  do
  { ptr0 = pixel_data[0][row];
    ptr1 = pixel_data[1][row];
    ptr2 = pixel_data[2][row]; col = width;
    do *raster_ptr++ =
	 screen_rgb[0][*ptr0++] |
	 screen_rgb[1][*ptr1++] |
	 screen_rgb[2][*ptr2++];
    while (--col > 0);
    raster_ptr += (col -= width) & 15;
  }
  while (++(int)row < num_rows);
  image_ptr = raster_ptr;
}

static void grayscale_put_pixel_rows( decompress_info_ptr cinfo,
				int num_rows, JSAMPIMAGE pixel_data )
{
  long col, width = cinfo->image_width;
  JSAMPROW ptr, *pix_arr = *pixel_data;
  unsigned mask, *raster_ptr;

  raster_ptr = image_ptr;
  do
  { ptr = *pix_arr++; col = width;
    for (;;)
    {
      mask = 0x8000;
      do
      { if (*ptr++ == 0) *raster_ptr |= mask;
	if (--col <= 0) goto nextrow;
      }
      while (mask >>= 1);
      raster_ptr++;
    }
    nextrow: raster_ptr++;
  }
  while (--num_rows > 0);
  image_ptr = raster_ptr;
}

static void rgbclut3_put_pixel_rows( decompress_info_ptr cinfo,
				int num_rows, JSAMPIMAGE pixel_data )
{
  long planesize, col, width = cinfo->image_width;
  JSAMPLE pix, *ptr, **pix_arr = *pixel_data;
  unsigned mask, *raster_ptr, *plane_ptr;
  int planes;

  raster_ptr = image_ptr; planesize = plane_size; planes = nplanes;
  do
  { ptr = *pix_arr++; col = width;
    for (;;)
    {
      mask = 0x8000;
      do
      { pix = *ptr++;
	if (pix -= 7)			/* nicht Weiss */
	{
	  plane_ptr = raster_ptr;
	  if ((pix += 7) == 0)		/* Schwarz */
	  {
	    plane_ptr = raster_ptr; pix = (JSAMPLE)planes;
	    do { *plane_ptr |= mask; (char *)plane_ptr += planesize; }
	    while (--pix > 0);
	  }
	  else
	  {
	    if (((char)pix <<= 5) < 0) *plane_ptr |= mask;
	    (char *)plane_ptr += planesize;
	    if (((char)pix <<= 1) < 0) *plane_ptr |= mask;
	    if (((char)pix <<= 1) < 0)
	      *(unsigned *)((char *)plane_ptr + planesize) |= mask;
	} }
	if (--col <= 0) goto nextrow;
      }
      while (mask >>= 1);
      raster_ptr++;
    }
    nextrow: raster_ptr++;
  }
  while (--num_rows > 0);
  image_ptr = raster_ptr;
}

static void rgbclut8_put_pixel_rows( decompress_info_ptr cinfo,
				int num_rows, JSAMPIMAGE pixel_data )
{
  long planesize, col, width = cinfo->image_width;
  JSAMPLE pix, *ptr, **pix_arr = *pixel_data;
  unsigned mask, *raster_ptr;

  raster_ptr = image_ptr; planesize = plane_size;
  do
  { ptr = *pix_arr++; col = width;
    for (;;)
    {
      mask = 0x8000;
      do
      { (char *)raster_ptr += planesize << 3;
	pix = *ptr++;
	(char *)raster_ptr -= planesize;
	if (((char)pix += 32) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= mask;
	if (--col <= 0) goto nextrow;
      }
      while (mask >>= 1);
      raster_ptr++;
    }
    nextrow: raster_ptr++;
  }
  while (--num_rows > 0);
  image_ptr = raster_ptr;
}

static void put_color_map( decompress_info_ptr cinfo,
			int num_colors, JSAMPARRAY colormap )
{
  if (num_colors > 0) return;
  *colormap++ = screen_set[0];
  if (cinfo->color_out_comps == 3)
  {
    *colormap++ = screen_set[1];
    *colormap   = screen_set[2];
} }

static void rgbclut8_put_color_map( decompress_info_ptr cinfo,
				int num_colors, JSAMPARRAY colormap )
{
  static JSAMPROW get_map = screen_set[0];

  *colormap++ = screen_set[0];
  if (cinfo->color_out_comps == 3)
  {
    *colormap++ = screen_set[1];
    *colormap   = screen_set[2];
  }
  else
  {
    JSAMPROW colorindex = colormap[-5];
    num_colors = MAXJSAMPLE;
    do *colorindex++ *= 43;
    while (--num_colors >= 0);
  }
  set_color_map( get_map );
  get_map = 0;
}

static void rgbclut8_output_term() { set_color_map( 0 ); }

static void output_init( MY_JPGD *fp )
{
  int m_wdwidth, m_w, m_h, m_nplanes;
  long rastsize;
  GUIDE *rg;

  m_wdwidth = ((int)fp->cinfo.image_width + 15) >> 4;
  m_w = ((int)fp->cinfo.image_width + 7) & -8;
  m_h = ((int)fp->cinfo.image_height + 7) & -8;
  m_nplanes = fp->cinfo.desired_number_of_colors == 2 ? 1 : nplanes;
  plane_size = 2L * m_wdwidth * m_h;
  rastsize = sizeof(GUIDE) + plane_size * m_nplanes;
  rg = Malloc( rastsize );
  if ((fp->rg = rg) == 0) longjmp( setjmp_buffer, EINVMEM );
  memset( rg, 0, rastsize ); setconv();
  if (m_nplanes < 16) ++rg->mfdb.fd_stand; else set_screen();
  rg->planes = fp->cinfo.data_precision * fp->cinfo.num_components;
  rg->width = (int)fp->cinfo.image_width;
  rg->height = (int)fp->cinfo.image_height;
  rg->x_flag += 8; image_ptr = rg->mfdb.fd_addr = rg->rgb_list;
  rg->mfdb.fd_nplanes = m_nplanes;
  rg->mfdb.fd_wdwidth = m_wdwidth;
  rg->w = rg->mfdb.fd_w = m_w; rg->bc = rg->w >> 3;
  rg->h = rg->mfdb.fd_h = m_h; rg->lc = rg->h >> 3;
  rg->xfac += 8; rg->yfac += 8; rg->draw = draw_image;
  rg->sclick = sclick_image; rg->dclick = dummy_image;
  rg->free = dummy_image; rg->key = key_image;
}

static void d_ui_method_selection( decompress_info_ptr cinfo )
{
  cinfo->output_init = (void (*)(decompress_info_ptr))output_init;
  cinfo->put_color_map = put_color_map;
  cinfo->output_term = dummy_image;
  cinfo->use_dithering = par.dithtyp;

  cinfo->out_color_space =
    nplanes < 3 ||
    par.dithmod == 0 || cinfo->jpeg_color_space == CS_GRAYSCALE
    ? CS_GRAYSCALE : CS_RGB;
  if (nplanes < 3 ||
      cinfo->out_color_space == CS_GRAYSCALE &&
      (par.dithcol == 0 || nplanes < 8))
  {
    cinfo->put_pixel_rows = grayscale_put_pixel_rows;
    cinfo->desired_number_of_colors = 2;
    cinfo->quantize_colors++; return;
  }
  if (nplanes < 16)
  {
    if (par.dithcol == 0 || nplanes < 8)
    {
      cinfo->put_pixel_rows = rgbclut3_put_pixel_rows;
      cinfo->desired_number_of_colors = 2 * 2 * 2;
    }
    else
    {
      if (cinfo->use_dithering)
	cinfo->put_color_map = rgbclut8_put_color_map;
      else
	cinfo->output_term = rgbclut8_output_term;
      cinfo->put_pixel_rows = rgbclut8_put_pixel_rows;
      cinfo->desired_number_of_colors =
	cinfo->out_color_space == CS_GRAYSCALE ? 6 : 6 * 6 * 6;
    }
    cinfo->quantize_colors++; return;
  }
  if (nplanes < 24)
  {
    cinfo->quantize_colors = cinfo->use_dithering;
    cinfo->put_pixel_rows = cinfo->out_color_space == CS_GRAYSCALE
      ? gray16_put_pixel_rows : rgbtrue16_put_pixel_rows;
  }
  else if (nplanes < 32)
  {
    cinfo->put_pixel_rows = cinfo->out_color_space == CS_GRAYSCALE
      ? gray24_put_pixel_rows : rgbtrue24_put_pixel_rows;
  }
  else
    cinfo->put_pixel_rows = cinfo->out_color_space == CS_GRAYSCALE
      ? gray32_put_pixel_rows : rgbtrue32_put_pixel_rows;
}

static void progress_monitor( MY_JPGD *fp )
{
  static long nextlimit;
  static MFORM *nextmform;
  long loopcounter = fp->cinfo.MCU_row_counter;
  long looplimit = fp->cinfo.MCU_rows_in_scan;

  if ((loopcounter <<= 3) == 0)
  {
    long new_size = fp->length;
    unsigned char *new_buf;

    nextlimit = looplimit;
    nextmform = achtel;

    if (fp->input_buffer != fp->fbuf) return;
    for (;;)
    {
      if (new_size <= fp->buf_size) return;
      if ((new_buf = Malloc( new_size )) != 0)
      {
	fp->input_buffer = new_buf;
	fp->buf_size = new_size; return;
      }
      new_size = Mavail() & -sizeof(fp->fbuf);
  } }
  while (loopcounter >= nextlimit)
  {
    nextlimit += looplimit;
    graf_mouse( USER_DEF, nextmform++ );
} }

static void read_jpeg_data( MY_JPGD *fp )
{
  long num_read;

  if (fp->EOF_flag) goto eof;

  if ((num_read = Fread( fp->handle,
			 fp->buf_size,
			 fp->input_buffer )) > 0)
    fp->length -= num_read;
  else
  {
    WARNMS(fp->cinfo.emethods, "Premature EOF in JPEG file");
    fp->EOF_flag = 1;
    fp->input_buffer[0] = (char) 0xFF;
    fp->input_buffer[1] = (char) 0xD9; /* EOI marker */
    eof:
    num_read = 2;
  }
  fp->cinfo.bytes_in_buffer += num_read;
  fp->cinfo.next_input_byte = fp->input_buffer;
}

static void error_exit() { longjmp( setjmp_buffer, ENOENT ); }

GUIDE *load_jpg( int fh, long len )
{
  static MY_JPGD my_read;

  memset( &my_read, 0, sizeof(my_read) );
  my_read.handle = fh;
  my_read.length = len;
  my_read.buf_size = sizeof(my_read.fbuf);
  my_read.input_buffer = my_read.fbuf;

  my_read.cinfo.d_ui_method_selection = d_ui_method_selection;
  my_read.cinfo.progress_monitor =
    (void (*)(decompress_info_ptr))progress_monitor;
  my_read.cinfo.read_jpeg_data =
    (void (*)(decompress_info_ptr))read_jpeg_data;

  my_read.e_methods.error_exit = error_exit;
  my_read.e_methods.trace_message = dummy_image;
  my_read.e_methods.alloc = (void *(*)(size_t))Malloc;
  my_read.e_methods.free = (void (*)(void *))Mfree;
  my_read.e_methods.more_warning_level = 3;

  my_read.cinfo.emethods = &my_read.e_methods;

  jselmemmgr( &my_read.e_methods );
  jselrjfif( &my_read.cinfo );

  /* Default to RGB output */
  /* UI can override by changing out_color_space */
  /* my_read.cinfo.out_color_space = CS_RGB; */
  /* my_read.cinfo.jpeg_color_space = CS_UNKNOWN; */
  /* Setting any other value in jpeg_color_space overrides heuristics */
  /* in jrdjfif.c. That might be useful when reading non-JFIF JPEG */
  /* files, but ordinarily the UI shouldn't change it. */
  
  if ((fh = setjmp( setjmp_buffer )) != 0)
  {
    if (my_read.cinfo.MCU_row_counter == 0)
    {
      form_error( fh );
      if (my_read.rg) { Mfree( my_read.rg ); my_read.rg = 0; }
    }
    (*my_read.e_methods.free_all)();
  }
  else jpeg_decompress( &my_read.cinfo );

  if (my_read.input_buffer != my_read.fbuf)
    Mfree( my_read.input_buffer );
  Fclose( my_read.handle );
  return my_read.rg;
}
