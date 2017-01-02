#include <aes.h>
#include <vdi.h>
#include <errno.h>
#include <osbind.h>
#include <string.h>

#include "jpgdh.h"
#include "1stguide.h"
#include "util.h"

typedef struct guide
{
  int	handle, width, height, planes,
	lbc,			/* left-border-count fÅr Fenster.	*/
	tlc,			/* top-line-count fÅr Fenster.		*/
	bc,			/* border-count fÅr Fenster.		*/
	lc,			/* line-count fÅr Fenster.		*/
	xfac, yfac,		/* Scroll- und Rastereinheiten.		*/
	x, y, w, h,		/* Arbeitsbereich des Fensters.		*/
	hslide, vslide,		/* Puffer fÅr Sliderpositionen.		*/
	(*key)( struct guide *tg, int code, int ks );
  void	(*draw)( struct guide *tg, int *clip ),
	(*free)( struct guide *tg ),
	(*sclick)( struct guide *tg, int mx, int my ),
	(*dclick)( struct guide *tg, int mx, int my, int ks );
  char	path[128],		/* Pfad fÅr Datei îffnen.		*/
	name[128],		/* Informationspfad (Fenstertitel).	*/
	hist[256];		/* History-Buffer.			*/
  struct guide *next;		/* Zeiger auf nÑchste Fensterstruktur.	*/
  struct guide *prev;		/* Zeiger auf vorige Fensterstruktur.	*/
  BASPAG *actpd;		/* Zeiger auf Prozess-Descriptor.	*/

  MFDB	mfdb;
  int	x_flag, scale,
	rgb_list[8];		/* Farbpalette.				*/
}
GUIDE;

typedef struct
{
  int *colorindex;
  int *colorset;
  int *fserrors;
  int *dummy;
}
FSBUF;

FROM( image ) IMPORT unsigned char Pli2Vdi[256];
FROM( image ) IMPORT void setconv( void );
FROM( image ) IMPORT void draw_image();
FROM( image ) IMPORT int key_image();
FROM( image ) IMPORT void sclick_image();
FROM( image ) IMPORT void dummy_image();

unsigned char dithermatrix[] =
{
    0,192, 48,240, 12,204, 60,252,  3,195, 51,243, 15,207, 63,255,
  128, 64,176,112,140, 76,188,124,131, 67,179,115,143, 79,191,127,
   32,224, 16,208, 44,236, 28,220, 35,227, 19,211, 47,239, 31,223,
  160, 96,144, 80,172,108,156, 92,163, 99,147, 83,175,111,159, 95,
    8,200, 56,248,  4,196, 52,244, 11,203, 59,251,  7,199, 55,247,
  136, 72,184,120,132, 68,180,116,139, 75,187,123,135, 71,183,119,
   40,232, 24,216, 36,228, 20,212, 43,235, 27,219, 39,231, 23,215,
  168,104,152, 88,164,100,148, 84,171,107,155, 91,167,103,151, 87,
    2,194, 50,242, 14,206, 62,254,  1,193, 49,241, 13,205, 61,253,
  130, 66,178,114,142, 78,190,126,129, 65,177,113,141, 77,189,125,
   34,226, 18,210, 46,238, 30,222, 33,225, 17,209, 45,237, 29,221,
  162, 98,146, 82,174,110,158, 94,161, 97,145, 81,173,109,157, 93,
   10,202, 58,250,  6,198, 54,246,  9,201, 57,249,  5,197, 53,245,
  138, 74,186,122,134, 70,182,118,137, 73,185,121,133, 69,181,117,
   42,234, 26,218, 38,230, 22,214, 41,233, 25,217, 37,229, 21,213,
  170,106,154, 90,166,102,150, 86,169,105,153, 89,165,101,149, 85,

  32, 32+43, 32+2*43, 32+3*43, 32+4*43, 32+5*43
};

static int screen_set[3][256];
long screen_rgb[3][256];
long plane_size;
static void *image_ptr;
static char *fsp;
unsigned char *dith_row;
static int row_count, dest_planes;

void set_screen( void )
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
      rgb[ci] = ((unsigned)i * 200U) / 51U;
      vs_color( handle, 1, rgb );
      if (check.fd_nplanes < 24)
      {
	vq_color( handle, 1, 1, rgb );
	screen_set[ci][i] = ((unsigned)rgb[ci] * 51U + 100U) / 200U;
      }
      v_pmarker( handle, 1, pxy );
      vro_cpyfm( handle, S_ONLY, pxy, &screen, &check );
      ++(long *)check.fd_addr;
    }
    while (++i <= 255);
  }
  while (++ci <= 2);
  vro_cpyfm( handle, S_ONLY, pxy, &save, &screen );
  vs_color( handle, 1, pxy );
}

void set_color_map( int *get_map )
{
  int i, ci, r = 0, g = 0, b = 0, rgb[3];

  for (i = 0;;i++)
  {
    rgb[0] = r; rgb[1] = g; rgb[2] = b; ci = Pli2Vdi[32 + i];
    vs_color( handle, ci, rgb );
    if (get_map)
    {
      vq_color( handle, ci, 1, rgb ); ci = 2;
      do get_map[256*ci+i] = ((unsigned)rgb[ci] * 51U + 100U) / 200U;
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

char *fs_init( int ditherflag, long fs_data, long fs_width )
{
  int nci, nc, i, j, k, max_colors, blksize, limit, val;
  int *p, *colormap, mycolormap[8];
  FSBUF *fs;

  fs_width = (fs_width + 2) * 2;
  i = 1;
  if (ditherflag != i)
  {
    i = 3;
    if (ditherflag != i)
    {
      fs_width *= 3; fs_data *= 3;
  } }
  j = 2;
  if (par.dithtyp == 0 || (fs = Malloc(
      (sizeof(FSBUF) + 1024) * i + fs_data + fs_width )) == 0)
  {
    if (ditherflag > j) set_color_map( 0 ); return 0;
  }
  (char *)fs += fs_data;
  p = (int *)(fs + i);

  if (ditherflag > 0)
  {
    if (ditherflag > j)
    {
      static int *get_map = screen_set[0];
      set_color_map( get_map );
      get_map = 0; j = 6;
    }
    nc = i; k = 1; do k *= j; while (--i > 0);

    blksize = max_colors = k; nci = --j;

    do
    { blksize /= nci + 1;

      if (nci != 1) colormap = screen_set[i];
      else
      {
	colormap = mycolormap; val = j = 0;
	do { colormap[j] = val; (char)val = ~(char)val; }
	while ((j += blksize) < max_colors);
      }
      limit = 128; val = j = k = 0;
      do
      { while (k > limit) { limit += 255; val += blksize; }
	p[256] = colormap[val]; *p++ = val; k += nci;
      }
      while (++(char)j);
      fs[i].colorindex = p - 256;
      fs[i].colorset = p; p += 256;
    }
    while (++i < nc);
    if (ditherflag == 3)
    {
      long l = 0;
      do fs[0].colorindex[l] +=
	 fs[1].colorindex[l] +
	 fs[2].colorindex[l];
      while (++(char)l);
  } }    
  else
  {
    fs[0].colorset = screen_set[0];
    fs[1].colorset = screen_set[1];
    fs[2].colorset = screen_set[2];
  }
  memset( p, 0, fs_width );
  {
    FSBUF *fst = fs;
    do { fst->fserrors = p; fst++; p++; } while (--i > 0);
  }
  return ((char *)fs - fs_data);
}

int fs_dither( int n_comps, int n_rows, int n_cols,
	       char *fs, unsigned char *ptr )
{
  int comp, col, val, cur, belowerr, bpreverr;
  int *colorindex, *colorset, *fserrors;

  for (;;)
  {
    if ((comp = n_comps) < 0) { ptr += n_comps; comp = -comp; }
    do
    { colorindex = *((int **)fs)++;
      colorset = *((int **)fs)++;
      fserrors = *((int **)fs)++;
      col = n_cols; ((int **)fs)++;
      if ((val = n_comps) < 0)
	do
	{ (char *)fserrors += col;
	  (char *)fserrors += col;
	  fserrors++;
	  ptr += col;
	}
	while (++val < 0);
      cur = belowerr = bpreverr = 0;
      do
      { n_comps <<= 1;
        cur += *(int *)((char *)fserrors + n_comps);
        cur += 8; cur >>= 4;
	if ((cur += *ptr) < 0) cur = 0;
	else if (cur > 255) cur = 255;
	*ptr = colorindex[cur];
	cur -= colorset[cur];
	val = cur << 1;  cur += val;
	bpreverr += cur; cur += val;
	belowerr += cur; cur += val;
	*fserrors = bpreverr;
	bpreverr = belowerr;
	belowerr = val >> 1;
	(char *)fserrors += n_comps;
	n_comps >>= 1;
	ptr += n_comps;
      }
      while (--col > 0);
      *fserrors = bpreverr;
      if ((val = n_comps) > 0) do ptr -= n_cols; while (--val > 0);
      ptr++;
    }
    while (--comp > 0);
    n_comps = -n_comps;
    if (--n_rows <= 0) break;
    col = (n_cols + 15) & -16;
    if ((val = n_comps) < 0) { ptr += n_comps; val = -val; }
    do { ptr += col; --(FSBUF *)fs; } while (--val > 0);
  }
  return n_comps;
}

static JPGD_PTR fs_dither2( JPGD_PTR jpgd_ptr )
{
  char *fs;
  unsigned char *ptr;
  int n_comps, n_rows, n_cols;
  int comp, col, val, cur, belowerr, bpreverr;
  int *colorset, *fserrors;

  ptr = jpgd_ptr->OutTmpPointer; fs = fsp;
  n_cols = jpgd_ptr->MFDBStruct.fd_w;
  n_rows = jpgd_ptr->OutTmpHeight;
  n_comps = row_count;
  do
  { if ((comp = n_comps) < 0) { ptr += n_comps; comp = -comp; }
    do
    { col = n_cols; ((int **)fs)++;
      colorset = *((int **)fs)++;
      fserrors = *((int **)fs)++;
      ((int **)fs)++;
      if ((val = n_comps) < 0)
	do
	{ (char *)fserrors += col;
	  (char *)fserrors += col;
	  fserrors++;
	  ptr += col;
	}
	while (++val < 0);
      cur = belowerr = bpreverr = 0;
      do
      { n_comps <<= 1;
        cur += *(int *)((char *)fserrors + n_comps);
        cur += 8; cur >>= 4;
	if ((cur += *ptr) < 0) cur = 0;
	else if (cur > 255) cur = 255;
	*ptr = cur;
	cur -= colorset[cur];
	val = cur << 1;  cur += val;
	bpreverr += cur; cur += val;
	belowerr += cur; cur += val;
	*fserrors = bpreverr;
	bpreverr = belowerr;
	belowerr = val >> 1;
	(char *)fserrors += n_comps;
	n_comps >>= 1;
	ptr += n_comps;
      }
      while (--col > 0);
      *fserrors = bpreverr;
      if ((val = n_comps) > 0) do ptr -= n_cols; while (--val > 0);
      ptr++;
    }
    while (--comp > 0);
    n_comps = -n_comps; col = (n_cols + 15) & -16;
    if ((val = n_comps) < 0) { ptr += n_comps; val = -val; }
    do { ptr += col; --(FSBUF *)fs; } while (--val > 0);
  }
  while (--n_rows > 0);
  row_count = n_comps; return jpgd_ptr;
}

char *gray16_put_pixel_rows( char *raster_ptr, int num_cols,
			     int num_rows, unsigned char *ptr )
{
  long val, *red, *green, *blue;

  red   = screen_rgb[0];
  green = red + 256;
  blue  = green + 256;
  do
  { int col = num_cols;
    do
    { int i = 0; val = *ptr++;
      i |= *(int *)&red  [val];
      i |= *(int *)&green[val];
      i |= *(int *)&blue [val];
      *((int *)raster_ptr)++ = i;
    }
    while (--col > 0);
    col -= num_cols; col &= 15; ptr += col;
    col <<= 1; raster_ptr += col;
  }
  while (--num_rows > 0);
  return raster_ptr;
}

static short gray16( JPGD_PTR jpgd_ptr )
{
  image_ptr = gray16_put_pixel_rows( image_ptr,
				     jpgd_ptr->MFDBStruct.fd_w,
				     jpgd_ptr->OutTmpHeight,
				     jpgd_ptr->OutTmpPointer );
  return 0;
}

static short gray16fs( JPGD_PTR jpgd_ptr )
{
  return gray16( fs_dither2( jpgd_ptr ) );
}

static short rgbtrue16_put_pixel_rows( JPGD_PTR jpgd_ptr )
{
  unsigned char *ptr, *raster_ptr;
  long *red, *green, *blue;
  int i, num_cols, num_rows;

  num_cols = jpgd_ptr->MFDBStruct.fd_w;
  num_rows = jpgd_ptr->OutTmpHeight;
  ptr = jpgd_ptr->OutTmpPointer;
  raster_ptr = image_ptr;
  red   = screen_rgb[0];
  green = red + 256;
  blue  = green + 256;
  do
  { int col = num_cols;
    do
    { i  = *(int *)&red  [*ptr++];
      i |= *(int *)&green[*ptr++];
      i |= *(int *)&blue [*ptr++];
      *((int *)raster_ptr)++ = i;
    }
    while (--col > 0);
    col -= num_cols; col &= 15; ptr += col;
    col <<= 1; ptr += col; raster_ptr += col;
  }
  while (--num_rows > 0);
  image_ptr = raster_ptr; return 0;
}

static short rgb16fs( JPGD_PTR jpgd_ptr )
{
  return rgbtrue16_put_pixel_rows( fs_dither2( jpgd_ptr ) );
}

char *gray24_put_pixel_rows( char *raster_ptr, int num_cols,
			     int num_rows, unsigned char *ptr )
{
  long i, j, k, *red, *green, *blue;

  red   = screen_rgb[0];
  green = red + 256;
  blue  = green + 256;
  do
  { int col = num_cols;
    do
    { i = 0; j = *ptr++;
      i |= red  [j];
      i |= green[j];
      i |= blue [j];
      i >>= 8; j = i >> 8; k = j >> 8;
      *raster_ptr++ = k;
      *raster_ptr++ = j;
      *raster_ptr++ = i;
    }
    while (--col > 0);
    col -= num_cols; col &= 15;
    ptr += col; raster_ptr += col;
    col <<= 1; raster_ptr += col;
  }
  while (--num_rows > 0);
  return raster_ptr;
}

static short gray24( JPGD_PTR jpgd_ptr )
{
  image_ptr = gray24_put_pixel_rows( image_ptr,
				     jpgd_ptr->MFDBStruct.fd_w,
				     jpgd_ptr->OutTmpHeight,
				     jpgd_ptr->OutTmpPointer );
  return 0;
}

static short rgbtrue24_put_pixel_rows( JPGD_PTR jpgd_ptr )
{
  unsigned char *ptr, *raster_ptr;
  long i, j, k, *red, *green, *blue;
  int num_cols, num_rows;

  num_cols = jpgd_ptr->MFDBStruct.fd_w;
  num_rows = jpgd_ptr->OutTmpHeight;
  ptr = jpgd_ptr->OutTmpPointer;
  raster_ptr = image_ptr;
  red   = screen_rgb[0];
  green = red + 256;
  blue  = green + 256;
  do
  { int col = num_cols;
    do
    { i  = red  [*ptr++];
      i |= green[*ptr++];
      i |= blue [*ptr++];
      i >>= 8; j = i >> 8; k = j >> 8;
      *raster_ptr++ = k;
      *raster_ptr++ = j;
      *raster_ptr++ = i;
    }
    while (--col > 0);
    col -= num_cols; col &= 15;
    ptr += col; raster_ptr += col; col <<= 1;
    ptr += col; raster_ptr += col;
  }
  while (--num_rows > 0);
  image_ptr = raster_ptr; return 0;
}

char *gray32_put_pixel_rows( char *raster_ptr, int num_cols,
			     int num_rows, unsigned char *ptr )
{
  long i, val, *red, *green, *blue;

  red   = screen_rgb[0];
  green = red + 256;
  blue  = green + 256;
  do
  { int col = num_cols;
    do
    { i = 0; val = *ptr++;
      i |= red  [val];
      i |= green[val];
      i |= blue [val];
      *((long *)raster_ptr)++ = i;
    }
    while (--col > 0);
    col -= num_cols; col &= 15; ptr += col;
    col <<= 2; raster_ptr += col;
  }
  while (--num_rows > 0);
  return raster_ptr;
}

static short gray32( JPGD_PTR jpgd_ptr )
{
  image_ptr = gray32_put_pixel_rows( image_ptr,
				     jpgd_ptr->MFDBStruct.fd_w,
				     jpgd_ptr->OutTmpHeight,
				     jpgd_ptr->OutTmpPointer );
  return 0;
}

static short rgbtrue32_put_pixel_rows( JPGD_PTR jpgd_ptr )
{
  unsigned char *ptr, *raster_ptr;
  long i, *red, *green, *blue;
  int num_cols, num_rows;

  num_cols = jpgd_ptr->MFDBStruct.fd_w;
  num_rows = jpgd_ptr->OutTmpHeight;
  ptr = jpgd_ptr->OutTmpPointer;
  raster_ptr = image_ptr;
  red   = screen_rgb[0];
  green = red + 256;
  blue  = green + 256;
  do
  { int col = num_cols;
    do
    { i  = red  [*ptr++];
      i |= green[*ptr++];
      i |= blue [*ptr++];
      *((long *)raster_ptr)++ = i;
    }
    while (--col > 0);
    col -= num_cols; col &= 15;
    ptr += col; col <<= 1; ptr += col;
    col <<= 1; raster_ptr += col;
  }
  while (--num_rows > 0);
  image_ptr = raster_ptr; return 0;
}

static short grayscale_put_pixel_rows( JPGD_PTR jpgd_ptr )
{
  int col, num_cols, num_rows;
  unsigned mask, *raster_ptr;
  unsigned char *ptr;

  num_cols = jpgd_ptr->MFDBStruct.fd_w;
  num_rows = jpgd_ptr->OutTmpHeight;
  ptr = jpgd_ptr->OutTmpPointer;
  row_count = fs_dither( row_count, num_rows, num_cols, fsp, ptr );
  raster_ptr = image_ptr;
  do
  { col = num_cols;
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
    nextrow: raster_ptr++; ptr += (col -= num_cols) & 15;
  }
  while (--num_rows > 0);
  image_ptr = raster_ptr; return 0;
}

static short rgbclut3_put_pixel_rows( JPGD_PTR jpgd_ptr )
{
  long planesize;
  char planes, pix, *ptr;
  unsigned mask, *raster_ptr, *plane_ptr;
  int col, num_cols, num_rows;

  num_cols = jpgd_ptr->MFDBStruct.fd_w;
  num_rows = jpgd_ptr->OutTmpHeight;
  ptr = jpgd_ptr->OutTmpPointer;
  row_count = fs_dither( row_count, num_rows, num_cols,
			 fsp, (unsigned char *)ptr );
  raster_ptr = image_ptr; planesize = plane_size; planes = nplanes;
  do
  { col = num_cols;
    for (;;)
    {
      mask = 0x8000;
      do
      { pix  = *ptr++;
	pix += *ptr++;
	pix += *ptr++;
	if (pix -= 7)		/* nicht Weiss */
	{
	  plane_ptr = raster_ptr;
	  if ((pix += 7) == 0)	/* Schwarz */
	  {
	    pix = planes;
	    do { *plane_ptr |= mask; (char *)plane_ptr += planesize; }
	    while (--pix > 0);
	  }
	  else
	  {
	    if ((pix <<= 5) < 0) *plane_ptr |= mask;
	    (char *)plane_ptr += planesize;
	    if ((pix <<= 1) < 0) *plane_ptr |= mask;
	    if ((pix <<= 1) < 0)
	      *(unsigned *)((char *)plane_ptr + planesize) |= mask;
	} }
        if (--col <= 0) goto nextrow;
      }
      while (mask >>= 1);
      raster_ptr++;
    }
    nextrow: raster_ptr++; col -= num_cols; col &= 15;
    ptr += col; ptr += col; ptr += col;
  }
  while (--num_rows > 0);
  image_ptr = raster_ptr; return 0;
}

static short gray8_put_pixel_rows( JPGD_PTR jpgd_ptr )
{
  long planesize;
  char pix, *ptr;
  unsigned mask, *raster_ptr;
  int col, num_cols, num_rows;

  num_cols = jpgd_ptr->MFDBStruct.fd_w;
  num_rows = jpgd_ptr->OutTmpHeight;
  ptr = jpgd_ptr->OutTmpPointer;
  row_count = fs_dither( row_count, num_rows, num_cols,
			 fsp, (unsigned char *)ptr );
  raster_ptr = image_ptr; planesize = plane_size;
  do
  { col = num_cols;
    for (;;)
    {
      mask = 0x8000;
      do
      { (char *)raster_ptr += planesize << 3;
	pix = 32;
	(char *)raster_ptr -= planesize;
	if ((pix += *ptr++) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pix <<= 1) < 0) *raster_ptr |= mask;
	if (--col <= 0) goto nextrow;
      }
      while (mask >>= 1);
      raster_ptr++;
    }
    nextrow: raster_ptr++; ptr += (col -= num_cols) & 15;
  }
  while (--num_rows > 0);
  image_ptr = raster_ptr; return 0;
}

static short rgbclut8_put_pixel_rows( JPGD_PTR jpgd_ptr )
{
  long planesize;
  char pix, *ptr;
  unsigned mask, *raster_ptr;
  int col, num_cols, num_rows;

  num_cols = jpgd_ptr->MFDBStruct.fd_w;
  num_rows = jpgd_ptr->OutTmpHeight;
  ptr = jpgd_ptr->OutTmpPointer;
  row_count = fs_dither( row_count, num_rows, num_cols,
			 fsp, (unsigned char *)ptr );
  raster_ptr = image_ptr; planesize = plane_size;
  do
  { col = num_cols;
    for (;;)
    {
      mask = 0x8000;
      do
      { (char *)raster_ptr += planesize << 3;
	pix = 32; pix += *ptr++; pix += *ptr++;
	(char *)raster_ptr -= planesize;
	if ((pix += *ptr++) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pix <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pix <<= 1) < 0) *raster_ptr |= mask;
	if (--col <= 0) goto nextrow;
      }
      while (mask >>= 1);
      raster_ptr++;
    }
    nextrow: raster_ptr++; col -= num_cols; col &= 15;
    ptr += col; ptr += col; ptr += col;
  }
  while (--num_rows > 0);
  image_ptr = raster_ptr; return 0;
}

unsigned *color_quantize1( unsigned *raster_ptr, int num_cols,
			   int num_rows, unsigned char *ptr )
{
  int col;
  unsigned mask;
  unsigned char *dith_ptr, *dith_end;

  dith_ptr = dith_row; dith_end = dithermatrix + 256;
  do
  { col = num_cols;
    for (;;)
    {
      mask = 0x8000;
      do
      { if (*ptr++ <= *dith_ptr++) *raster_ptr |= mask;
	if (--col <= 0) goto nextrow;
      }
      while (mask >>= 1);
      dith_ptr -= 16; raster_ptr++;
    }
    nextrow: raster_ptr++; col -= num_cols; col &= 15;
    dith_ptr += col; ptr += col;
    if (dith_ptr == dith_end) dith_ptr -= 256;
  }
  while (--num_rows > 0);
  dith_row = dith_ptr;
  return raster_ptr;
}

static short gray1( JPGD_PTR jpgd_ptr )
{
  image_ptr = color_quantize1( image_ptr,
			       jpgd_ptr->MFDBStruct.fd_w,
			       jpgd_ptr->OutTmpHeight,
			       jpgd_ptr->OutTmpPointer );
  return 0;
}

static short color_quantize3( JPGD_PTR jpgd_ptr )
{
  char pixcode;
  long planesize;
  unsigned mask, *raster_ptr, *plane_ptr;
  unsigned char dith, *ptr, *dith_ptr, *dith_end;
  int col, num_cols, num_rows, planes;

  num_cols = jpgd_ptr->MFDBStruct.fd_w;
  num_rows = jpgd_ptr->OutTmpHeight;
  ptr = jpgd_ptr->OutTmpPointer;
  dith_ptr = dith_row; raster_ptr = image_ptr;
  planesize = plane_size; planes = nplanes;
  dith_end = dithermatrix + 256;
  do
  { col = num_cols;
    for (;;)
    {
      mask = 0x8000;
      do
      { dith = *dith_ptr++; pixcode = 0;
	if (dith < *ptr++) pixcode += 4;
	if (dith < *ptr++) pixcode += 2;
	if (dith < *ptr++) pixcode += 1;
	if (pixcode -= 7)		/* nicht Weiss */
	{
	  plane_ptr = raster_ptr;
	  if ((pixcode += 7) == 0)	/* Schwarz */
	  {
	    pixcode = planes;
	    do { *plane_ptr |= mask; (char *)plane_ptr += planesize; }
	    while (--pixcode > 0);
	  }
	  else
	  {
	    if ((pixcode <<= 5) < 0) *plane_ptr |= mask;
	    (char *)plane_ptr += planesize;
	    if ((pixcode <<= 1) < 0) *plane_ptr |= mask;
	    if ((pixcode <<= 1) < 0)
	      *(unsigned *)((char *)plane_ptr + planesize) |= mask;
        } }
	if (--col <= 0) goto nextrow;
      }
      while (mask >>= 1);
      dith_ptr -= 16; raster_ptr++;
    }
    nextrow: raster_ptr++; col -= num_cols; col &= 15;
    dith_ptr += col; ptr += col; ptr += col; ptr += col;
    if (dith_ptr == dith_end) dith_ptr -= 256;
  }
  while (--num_rows > 0);
  image_ptr = raster_ptr; dith_row = dith_ptr; return 0;
}

unsigned *gray_quantize8( unsigned *raster_ptr, int num_cols,
			  int num_rows, unsigned char *ptr )
{
  char pixcode;
  int val, col;
  unsigned mask;
  long planesize;
  unsigned char *dith_ptr, *dith_end;

  dith_ptr = dith_row; planesize = plane_size;
  dith_end = dithermatrix + 256;
  do
  { col = num_cols;
    for (;;)
    {
      mask = 0x8000;
      do
      { (char *)raster_ptr += planesize << 3;
	val = *ptr++; val += val << 2;
	if (*dith_ptr++ < (unsigned char)val) val += 256;
	val >>= 8;
	(char *)raster_ptr -= planesize;
	if ((pixcode = dith_end[val]) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pixcode <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pixcode <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pixcode <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pixcode <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pixcode <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pixcode <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pixcode <<= 1) < 0) *raster_ptr |= mask;
	if (--col <= 0) goto nextrow;
      }
      while (mask >>= 1);
      dith_ptr -= 16; raster_ptr++;
    }
    nextrow: raster_ptr++; col -= num_cols; col &= 15;
    dith_ptr += col; ptr += col;
    if (dith_ptr == dith_end) dith_ptr -= 256;
  }
  while (--num_rows > 0);
  dith_row = dith_ptr;
  return raster_ptr;
}

static short gray8( JPGD_PTR jpgd_ptr )
{
  image_ptr = gray_quantize8( image_ptr,
			      jpgd_ptr->MFDBStruct.fd_w,
			      jpgd_ptr->OutTmpHeight,
			      jpgd_ptr->OutTmpPointer );
  return 0;
}

static short color_quantize8( JPGD_PTR jpgd_ptr )
{
  char pixcode;
  long planesize;
  unsigned mask, *raster_ptr;
  unsigned char dith, *ptr, *dith_ptr, *dith_end;
  int val, col, num_cols, num_rows;

  num_cols = jpgd_ptr->MFDBStruct.fd_w;
  num_rows = jpgd_ptr->OutTmpHeight;
  ptr = jpgd_ptr->OutTmpPointer;
  dith_ptr = dith_row;
  raster_ptr = image_ptr;
  planesize = plane_size;
  dith_end = dithermatrix + 256;
  do
  { col = num_cols;
    for (;;)
    {
      mask = 0x8000;
      do
      { (char *)raster_ptr += planesize << 3;
	dith = *dith_ptr++; pixcode = 0;
	val = *ptr++; val += val << 2;
	if (dith < (unsigned char)val) ++pixcode;
	val >>= 8; pixcode += (char)val;
	pixcode += pixcode << 1; pixcode <<= 1;
	val = *ptr++; val += val << 2;
	if (dith < (unsigned char)val) ++pixcode;
	val >>= 8; pixcode += (char)val;
	pixcode += pixcode << 1; pixcode <<= 1;
	val = *ptr++; val += val << 2;
	if (dith < (unsigned char)val) ++pixcode;
	val >>= 8; pixcode += (char)val;
	(char *)raster_ptr -= planesize;
	if ((pixcode += 32) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pixcode <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pixcode <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pixcode <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pixcode <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pixcode <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pixcode <<= 1) < 0) *raster_ptr |= mask;
	(char *)raster_ptr -= planesize;
	if ((pixcode <<= 1) < 0) *raster_ptr |= mask;
	if (--col <= 0) goto nextrow;
      }
      while (mask >>= 1);
      dith_ptr -= 16; raster_ptr++;
    }
    nextrow: raster_ptr++; col -= num_cols; col &= 15;
    dith_ptr += col; ptr += col; ptr += col; ptr += col;
    if (dith_ptr == dith_end) dith_ptr -= 256;
  }
  while (--num_rows > 0);
  image_ptr = raster_ptr; dith_row = dith_ptr; return 0;
}

static short (*get_write( JPGD_PTR jpgd_ptr ))( JPGD_PTR jpgd_ptr )
{
  long fs_width = jpgd_ptr->MFDBStruct.fd_w;
  int ncmin1 = jpgd_ptr->OutComponents - 1;
  int planes = dest_planes;

  if (planes == 1)
  {
    if ((fsp = fs_init( 1, 0, fs_width )) != 0)
      return grayscale_put_pixel_rows;
    return gray1;
  }
  if (planes < 16)
  {
    if (par.dithcol == 0 || planes < 8)
    {
      if ((fsp = fs_init( 2, 0, fs_width )) != 0)
	return rgbclut3_put_pixel_rows;
      return color_quantize3;
    }
    if (ncmin1 == 0)
    {
      if ((fsp = fs_init( 3, 0, fs_width )) != 0)
        return gray8_put_pixel_rows;
      return gray8;
    }
    if ((fsp = fs_init( 4, 0, fs_width )) != 0)
      return rgbclut8_put_pixel_rows;
    return color_quantize8;
  }
  if (planes < 24)
  {
    if ((fsp = fs_init( -1, 0, fs_width )) != 0)
    {
      if (ncmin1 == 0) return gray16fs;
      return rgb16fs;
    }
    if (ncmin1 == 0) return gray16;
    return rgbtrue16_put_pixel_rows;
  }
  fsp = 0;
  if (planes < 32)
  {
    if (ncmin1 == 0) return gray24;
    return rgbtrue24_put_pixel_rows;
  }
  if (ncmin1 == 0) return gray32;
  return rgbtrue32_put_pixel_rows;
}

static short do_Create( JPGD_PTR jpgd_ptr )
{
  jpgd_ptr->Write = get_write( jpgd_ptr );
  if (fsp) row_count = jpgd_ptr->OutComponents;
  else dith_row = dithermatrix;
  return 0;
}

static short do_Close()
{
  if (fsp) { Mfree( fsp ); fsp = 0; }
  return 0;
}

GUIDE *load_drv_jpg( int fh, long len, JPGDDRV_PTR jpgddrv_ptr )
{
  JPGD_PTR jpgd_ptr;
  char *buf;
  GUIDE *rg;
  long rastsize;
  int m_wdwidth, m_w, m_h, m_nplanes;

  rg = 0;
  rastsize = -4; rastsize &= JPGDGetStructSize( jpgddrv_ptr ) + 3;
  if ((jpgd_ptr = Malloc( rastsize + len )) == 0)
    { Fclose( fh ); form_error( EINVMEM ); return rg; }
  memset( jpgd_ptr, 0, rastsize );
  buf = (char *)jpgd_ptr + rastsize;
  Fread( fh, len, buf ); Fclose( fh );
  if (JPGDOpenDriver( jpgddrv_ptr, jpgd_ptr )) form_error( EINVMEM );
  else
  {
    jpgd_ptr->InPointer = buf;
    jpgd_ptr->InSize = len;
    if (JPGDGetImageInfo( jpgddrv_ptr, jpgd_ptr )) form_error( ENOENT );
    else
    {
      m_nplanes = nplanes;
      jpgd_ptr->OutComponents = jpgd_ptr->OutPixelSize =
	m_nplanes < 3 ||
	par.dithmod == 0 || jpgd_ptr->InComponents == 1 ? 1 : 3;
      if (m_nplanes < 3 ||
	  jpgd_ptr->OutComponents == 1 &&
	  (par.dithcol == 0 || m_nplanes < 8))
	m_nplanes = 1;
      m_wdwidth = (jpgd_ptr->MFDBStruct.fd_w + 15) >> 4;
      m_w = (jpgd_ptr->MFDBStruct.fd_w + 7) & -8;
      m_h = (jpgd_ptr->MFDBStruct.fd_h + 7) & -8;
      plane_size = 2L * m_wdwidth * m_h;
      rastsize = sizeof(GUIDE) + plane_size * m_nplanes;
      if ((rg = Malloc( rastsize )) == 0) form_error( EINVMEM );
      else
      {
	memset( rg, 0, rastsize ); setconv();
	if (m_nplanes < 16) ++rg->mfdb.fd_stand; else set_screen();
	rg->planes = jpgd_ptr->InComponents << 3;
	rg->width = jpgd_ptr->MFDBStruct.fd_w;
	rg->height = jpgd_ptr->MFDBStruct.fd_h;
	rg->x_flag += 8; image_ptr = rg->mfdb.fd_addr = rg->rgb_list;
	rg->mfdb.fd_nplanes = m_nplanes;
	rg->mfdb.fd_wdwidth = m_wdwidth;
	rg->w = rg->mfdb.fd_w = m_w; rg->bc = rg->w >> 3;
	rg->h = rg->mfdb.fd_h = m_h; rg->lc = rg->h >> 3;
	rg->xfac += 8; rg->yfac += 8; rg->draw = draw_image;
	rg->sclick = sclick_image; rg->dclick = dummy_image;
	rg->free = dummy_image; rg->key = key_image;

	if (JPGDGetImageSize( jpgddrv_ptr, jpgd_ptr ))
	{
	  Mfree( rg ); rg = 0; form_error( EINVMEM );
	}
	else
	{
	  dest_planes = m_nplanes;
	  jpgd_ptr->OutFlag--;
	  jpgd_ptr->Create = do_Create;
	  jpgd_ptr->Close =
	  jpgd_ptr->SigTerm = do_Close;
	  JPGDDecodeImage( jpgddrv_ptr, jpgd_ptr );
    } } }
    JPGDCloseDriver( jpgddrv_ptr, jpgd_ptr );
  }
  Mfree( jpgd_ptr ); return rg;
}

#include <setjmp.h>

#define PNG_INTERNAL

#include "png.h"

typedef struct
{
  long	r_y[257],
	g_y[257],
	b_y[257];
}
RGB_Y_TAB;

extern RGB_Y_TAB rgb_y_tab;

void rgb_y_init(void);

typedef struct
{
  png_struct png_ptr;
  png_info info_ptr;
  JPGD_STRUCT jpgd_ptr;
  png_byte *row_ptr;
  jmp_buf jmpbuf;
  GUIDE *rg;
  int handle, err_num;
  long buf_size, length;
  png_byte *input_buffer, fbuf[512];
}
MY_PNG;

static void my_error(MY_PNG *fp, char *message)
{
  longjmp(fp->jmpbuf, ENOENT);
}

static void my_warning(MY_PNG *fp, char *message)
{
}

static void *my_malloc(MY_PNG *fp, png_uint_32 size)
{
  void *ret = Malloc(size);
  if (ret) return ret;
  longjmp(fp->jmpbuf, EINVMEM);
}

static void my_free(MY_PNG *fp, void *ptr)
{
  if (ptr) Mfree(ptr);
}

static void my_read_data(MY_PNG *fp)
{
  long count;

  if ((count = Fread(fp->handle, fp->buf_size, fp->input_buffer)) > 0)
  {
    fp->length -= count;
    fp->png_ptr.zbuf_size += count;
    fp->png_ptr.zbuf = fp->input_buffer;
  }
  else longjmp(fp->jmpbuf, ENOENT);
}

void try_buf( MY_PNG *fp )
{
  long new_size = fp->length;
  unsigned char *new_buf;

  for (;;)
  {
    if (new_size <= fp->buf_size) return;
    if ((new_buf = Malloc( new_size )) != 0)
    {
      fp->input_buffer = new_buf;
      fp->buf_size = new_size; return;
    }
    new_size = (Mavail() - 16L*1024) & -512L;
} }

static void rgb_y_convert( unsigned char *sp, int i )
{
  long y, *r_y_tab, *g_y_tab, *b_y_tab;
  unsigned char *dp;

  r_y_tab = rgb_y_tab.r_y;
  g_y_tab = rgb_y_tab.g_y;
  b_y_tab = rgb_y_tab.b_y;
  dp = sp;
  do
  { y  = r_y_tab[*sp++];
    y += g_y_tab[*sp++];
    y += b_y_tab[*sp++];
    y >>= 16; *dp++ = (unsigned char)y;
  }
  while (--i);
}

GUIDE *load_png( int fh, long len )
{
  static MY_PNG my_read;
  int m_wdwidth, m_w, m_h, m_nplanes;
  long rastsize, expand_offs;
  png_byte chunk_start[8];
  png_uint_32 length;
  int more_passes;

  memset( &my_read, 0, sizeof(my_read) );
  my_read.handle = fh;
  my_read.length = len;
  my_read.buf_size = sizeof(my_read.fbuf);
  my_read.input_buffer = my_read.fbuf;
  my_read.png_ptr.zstream.zalloc = (zalloc_func)my_malloc;
  my_read.png_ptr.zstream.zfree = (zfree_func)my_free;
  my_read.png_ptr.png_error = (void (*)(png_struct *, char *))my_error;
  my_read.png_ptr.png_warning = (void (*)(png_struct *, char *))my_warning;
  my_read.png_ptr.png_readwrite = (void (*)(png_struct *))my_read_data;

  if ((my_read.err_num = setjmp( my_read.jmpbuf )) != 0) goto finish;

  png_read_data(&my_read.png_ptr, chunk_start, 8);
  if (memcmp(chunk_start, png_sig, 8))
    my_error(&my_read, "Not a Png File");

  for (;;)
  {
    length = png_read_uint_32(&my_read.png_ptr);
    png_reset_crc(&my_read.png_ptr);
    png_crc_read(&my_read.png_ptr, chunk_start, 4);
    if (!memcmp(chunk_start, png_IHDR, 4))
    {
      if (my_read.png_ptr.mode != PNG_BEFORE_IHDR)
	my_error(&my_read, "Out of Place IHDR");

      png_handle_IHDR(&my_read.png_ptr, &my_read.info_ptr, length);
      my_read.png_ptr.mode = PNG_HAVE_IHDR;
    }
    else if (!memcmp(chunk_start, png_PLTE, 4))
    {
      if (my_read.png_ptr.mode != PNG_HAVE_IHDR)
	my_error(&my_read, "Missing IHDR");

      if (my_read.info_ptr.color_type == PNG_COLOR_TYPE_PALETTE)
      {
	png_handle_PLTE(&my_read.png_ptr, &my_read.info_ptr, length);
	my_read.png_ptr.mode = PNG_HAVE_PLTE;
      }
      else png_crc_skip(&my_read.png_ptr, length);
    }
    else if (!memcmp(chunk_start, png_IDAT, 4))
    {
      my_read.png_ptr.idat_size = length;
      my_read.png_ptr.mode = PNG_HAVE_IDAT;
      break;
    }
    else if (!memcmp(chunk_start, png_IEND, 4))
    {
      my_error(&my_read, "No Image in File");
    }
    else
    {
      if (isupper(chunk_start[0]))
	my_error(&my_read, "Unknown Critical Chunk");

      png_crc_skip(&my_read.png_ptr, length);
    }
    if (~png_read_uint_32(&my_read.png_ptr) != my_read.png_ptr.crc)
      my_error(&my_read, "Bad CRC value");
  }

  my_read.jpgd_ptr.MFDBStruct.fd_w = (int)my_read.info_ptr.width;
  my_read.jpgd_ptr.MFDBStruct.fd_h = (int)my_read.info_ptr.height;

  m_nplanes = nplanes;
  my_read.jpgd_ptr.OutComponents = my_read.jpgd_ptr.OutPixelSize =
    m_nplanes < 3 ||
    par.dithmod == 0 ||
    (my_read.info_ptr.color_type & PNG_COLOR_MASK_COLOR) == 0 ? 1 : 3;
  if (m_nplanes < 3 ||
    my_read.jpgd_ptr.OutComponents == 1 &&
    (par.dithcol == 0 || m_nplanes < 8))
    m_nplanes = 1;
  m_wdwidth = (my_read.jpgd_ptr.MFDBStruct.fd_w + 15) >> 4;
  m_w = (my_read.jpgd_ptr.MFDBStruct.fd_w + 7) & -8;
  m_h = (my_read.jpgd_ptr.MFDBStruct.fd_h + 7) & -8;
  plane_size = 2L * m_wdwidth * m_h;
  rastsize = sizeof(GUIDE) + plane_size * m_nplanes;
  my_read.rg = my_malloc( &my_read, rastsize );
  memset( my_read.rg, 0, rastsize ); setconv();
  if (m_nplanes < 16) ++my_read.rg->mfdb.fd_stand; else set_screen();
  {
    GUIDE *rg = my_read.rg;
    rg->planes = my_read.info_ptr.channels * my_read.info_ptr.bit_depth;
    rg->width = my_read.jpgd_ptr.MFDBStruct.fd_w;
    rg->height = my_read.jpgd_ptr.MFDBStruct.fd_h;
    rg->x_flag += 8; image_ptr = rg->mfdb.fd_addr = rg->rgb_list;
    rg->mfdb.fd_nplanes = m_nplanes;
    rg->mfdb.fd_wdwidth = m_wdwidth;
    rg->w = rg->mfdb.fd_w = m_w; rg->bc = rg->w >> 3;
    rg->h = rg->mfdb.fd_h = m_h; rg->lc = rg->h >> 3;
    rg->xfac += 8; rg->yfac += 8; rg->draw = draw_image;
    rg->sclick = sclick_image; rg->dclick = dummy_image;
    rg->free = dummy_image; rg->key = key_image;
  }

  if (my_read.jpgd_ptr.OutComponents == 1 &&
      (my_read.info_ptr.color_type & PNG_COLOR_MASK_COLOR))
  {
    rgb_y_init();
    if (my_read.info_ptr.color_type == PNG_COLOR_TYPE_PALETTE)
      rgb_y_convert( my_read.info_ptr.palette,
		     my_read.info_ptr.num_palette );
  }

  more_passes = png_set_interlace_handling(&my_read.png_ptr);
  png_read_start_row(&my_read.png_ptr);

  expand_offs = rastsize = my_read.info_ptr.rowbytes;
  if (my_read.info_ptr.bit_depth < 8)
    expand_offs = my_read.info_ptr.width;
  if (my_read.info_ptr.color_type == PNG_COLOR_TYPE_PALETTE &&
      my_read.jpgd_ptr.OutComponents == 3)
    expand_offs = my_read.info_ptr.width * 3;
  expand_offs -= rastsize;
  if (--more_passes) rastsize *= my_read.info_ptr.height;
  my_read.row_ptr = my_malloc( &my_read, rastsize + expand_offs );
  my_read.jpgd_ptr.OutTmpHeight++;
  dest_planes = m_nplanes;
  do_Create( &my_read.jpgd_ptr );
  try_buf( &my_read );

  if (more_passes)
    do
    { length = my_read.info_ptr.height;
      my_read.jpgd_ptr.OutTmpPointer = my_read.row_ptr + expand_offs;
      do
      { png_read_row(&my_read.png_ptr, my_read.jpgd_ptr.OutTmpPointer, 0);
	(png_byte *)my_read.jpgd_ptr.OutTmpPointer +=
	  my_read.info_ptr.rowbytes;
      }
      while (--length);
    }
    while (--more_passes);

  length = my_read.info_ptr.height;
  my_read.jpgd_ptr.OutTmpPointer = my_read.row_ptr;
  do
  { png_read_row(&my_read.png_ptr,
		 (png_byte *)my_read.jpgd_ptr.OutTmpPointer + expand_offs,
		 0);
    if (my_read.info_ptr.color_type == PNG_COLOR_TYPE_PALETTE)
    {
      char bit_num;
      if ((bit_num = 8 - my_read.info_ptr.bit_depth) != 0)
      {
	png_byte bit_mask = (1 << my_read.info_ptr.bit_depth) - 1;
	png_byte *sp = (png_byte *)my_read.jpgd_ptr.OutTmpPointer
			  + expand_offs;
	png_byte *dp = sp + my_read.info_ptr.rowbytes
			  - my_read.info_ptr.width;
	for (;;)
	{
	  *dp = (*sp >> bit_num) & bit_mask;
	  if (dp == sp) break;
	  dp++;
	  if ((bit_num -= my_read.info_ptr.bit_depth) < 0)
	  {
	    bit_num += 8; sp++;
      } } }
      {
	png_byte *dp = my_read.jpgd_ptr.OutTmpPointer, *sp = dp;
	png_byte *tp = my_read.info_ptr.palette;
	if (my_read.jpgd_ptr.OutComponents == 3)
	{
	  sp += my_read.info_ptr.width * 2;
	  do
	  { long i = *sp++; i += i << 1;
	    *dp++ = tp[i];
	    *dp++ = tp[i + 1];
	    *dp++ = tp[i + 2];
	  }
	  while (sp != dp);
	}
	else
	{
	  long i = my_read.info_ptr.width;
	  do *dp++ = tp[*sp++];
	  while (--i);
    } } }
    else
    {
      png_byte *dp = my_read.jpgd_ptr.OutTmpPointer, *sp = dp;
      if (my_read.info_ptr.bit_depth == 16)
      {
	long i = my_read.info_ptr.rowbytes;
	do { *dp++ = *sp++; sp++; } while (i -= 2);
	sp = dp = my_read.jpgd_ptr.OutTmpPointer;
      }
      if (my_read.info_ptr.channels == 1)
      {
	char bit_num;
	if ((bit_num = 8 - my_read.info_ptr.bit_depth) > 0)
	{
	  sp += expand_offs;
	  for (;;)
	  {
	    png_byte shift;
	    *dp = (*sp >> bit_num) &
		    ((1 << my_read.info_ptr.bit_depth) - 1);
	    shift = 8;
	    while (shift -= my_read.info_ptr.bit_depth)
	       *dp |= *dp << shift;
	    if (dp == sp) break;
	    dp++;
	    if ((bit_num -= my_read.info_ptr.bit_depth) < 0)
	    {
	      bit_num += 8; sp++;
      } } } }
      else if (my_read.info_ptr.channels == 2)
      {
	long i = my_read.info_ptr.width;
	do { *dp++ = *sp++; sp++; } while (--i);
      }
      else
      {
	if (my_read.info_ptr.channels == 4)
	{
	  long i = my_read.info_ptr.width;
	  do { *dp++ = *sp++; *dp++ = *sp++; *dp++ = *sp++; sp++; }
	  while (--i);
	}
	if (my_read.jpgd_ptr.OutComponents == 1)
	  rgb_y_convert( my_read.jpgd_ptr.OutTmpPointer,
			 my_read.jpgd_ptr.MFDBStruct.fd_w );
    } }
    (*my_read.jpgd_ptr.Write)( &my_read.jpgd_ptr );
    if (my_read.info_ptr.interlace_type)
      (png_byte *)my_read.jpgd_ptr.OutTmpPointer +=
	my_read.info_ptr.rowbytes;
  }
  while (--length);

  finish:
  if (my_read.input_buffer != my_read.fbuf)
    Mfree( my_read.input_buffer );
  do_Close();
  if (my_read.row_ptr) Mfree( my_read.row_ptr );
  else
  {
    if (my_read.rg) { Mfree( my_read.rg ); my_read.rg = 0; }
    form_error( my_read.err_num );
  }
  png_read_destroy(&my_read.png_ptr, &my_read.info_ptr, 0);
  Fclose( my_read.handle );
  return my_read.rg;
}

typedef struct
{
  short version;
  unsigned short headlen;
  short planes,
	pat_run,
	pix_width,
	pix_height,
	sl_width,
	sl_height;
}
IMG_HEADER;

typedef struct
{
  IMG_HEADER ih;
  long magic;
  short comps,
	red,
	green,
	blue;
}
TIMG_HEADER;

FROM( util ) IMPORT jmp_buf setjmp_buffer;

#define FTESTC(fp) if (--(fp)->bytes_left < 0) Fbufread( fp );
#define FFETCHC(fp) *(fp)->pbuf++

typedef struct ibuf
{
  char *pbuf;
  long bytes_left;
  void (*putline)(struct ibuf *ip);
  short lines_left;
  char vrc, pad;
  short comps, bits[3];
  char *src_end[3];
  char *des_end;
  JPGD_STRUCT *jpgd_ptr;
  char *line_adr;
  long length;
  long linelen;
  char pat_buf[0];
}
IBUF;

#define ISKIPC(ip)   \
  MAKESTMT( (ip)->pbuf++; \
	    if (--(ip)->bytes_left == 0) (*(ip)->putline)(ip); )

#define IPUTC(ip, ch)   \
  MAKESTMT( *(ip)->pbuf++ = ch; \
	    if (--(ip)->bytes_left == 0) (*(ip)->putline)(ip); )

#define FICOPYC(src, des)   \
  MAKESTMT( if (--(src)->bytes_left < 0) Fbufread(src); \
	    *(des)->pbuf++ = *(src)->pbuf++; \
	    if (--(des)->bytes_left == 0) (*(des)->putline)(ip); )

static void goto_finish( void )
{
  longjmp( setjmp_buffer, 2 );
}

static void put_true( IBUF *ip )
{
  short comp, num_bits, bits;
  char *src, *des, *sp;
  unsigned char mask;
  long linelen;
  int val;

  linelen = ip->linelen;
  do
  { comp = 0;
    do
    { des = (char *)ip->jpgd_ptr->OutTmpPointer + comp;
      src = ip->src_end[comp];
      num_bits = ip->bits[comp];
      mask = 0x80;
      do
      { val = 0;
	if ((bits = num_bits) != 0)
	{
	  sp = src;
	  do
	  { sp -= linelen; val <<= 1;
	    if (*sp & mask) val++;
	  }
	  while (--bits);
	  switch (num_bits) /* Upscaling */
	  {
	    case 1: val |= val << 1;  /* fall through! */
	    case 2: val |= val << 2;  /* fall through! */
	    case 4: val |= val << 4; break;
	    case 5: val <<= 3; val |= val >> 5; break;
	    case 3: val |= val << 3;  /* fall through! */
	    case 6: val <<= 2; val |= val >> 6; break;
	    case 7: val <<= 1; val |= val >> 7; break;
	} }
	*des = (char)val; des += ip->comps;
	if ((mask >>= 1) == 0)
	{
	  mask = 0x80; src++;
      } }
      while (des < ip->des_end);
    }
    while (++comp < ip->comps);
    if (comp != ip->jpgd_ptr->OutComponents)
      rgb_y_convert( ip->jpgd_ptr->OutTmpPointer,
		     ip->jpgd_ptr->MFDBStruct.fd_w );
    (*ip->jpgd_ptr->Write)( ip->jpgd_ptr );
    if (--ip->lines_left <= 0) goto_finish();
  }
  while (--ip->vrc);
  ip->vrc++;
  ip->pbuf = ip->line_adr; ip->bytes_left = ip->length;
}

GUIDE *load_timg( int fh, TIMG_HEADER *th, long len )
{
  static JPGD_STRUCT jpgd_ptr;
  int m_wdwidth, m_w, m_h, m_nplanes, i;
  long rastsize, linelen, outlen;
  char cdata, *pat_ptr;
  GUIDE *rg;
  IBUF *ip;
  FBUF f;

  f.handle = fh; f.buf_size = len;
  memset( &jpgd_ptr, 0, sizeof(jpgd_ptr) );

  jpgd_ptr.MFDBStruct.fd_w = th->ih.sl_width;
  jpgd_ptr.MFDBStruct.fd_h = th->ih.sl_height;

  m_nplanes = nplanes;
  jpgd_ptr.OutComponents = jpgd_ptr.OutPixelSize =
    m_nplanes < 3 || par.dithmod == 0 || th->comps == 1 ? 1 : 3;
  if (m_nplanes < 3 ||
    jpgd_ptr.OutComponents == 1 &&
    (par.dithcol == 0 || m_nplanes < 8))
    m_nplanes = 1;
  m_wdwidth = (jpgd_ptr.MFDBStruct.fd_w + 15) >> 4;
  m_w = (jpgd_ptr.MFDBStruct.fd_w + 7) & -8;
  m_h = (jpgd_ptr.MFDBStruct.fd_h + 7) & -8;
  plane_size = 2L * m_wdwidth * m_h;
  rastsize = sizeof(GUIDE) + plane_size * m_nplanes;
  if ((rg = Malloc( rastsize )) == 0)
  {
    Fclose( f.handle ); form_error( EINVMEM ); return 0;
  }
  memset( rg, 0, rastsize ); setconv();
  if (m_nplanes < 16) ++rg->mfdb.fd_stand; else set_screen();
  rg->planes = th->ih.planes;
  rg->width = jpgd_ptr.MFDBStruct.fd_w;
  rg->height = jpgd_ptr.MFDBStruct.fd_h;
  rg->x_flag += 8; image_ptr = rg->mfdb.fd_addr = rg->rgb_list;
  rg->mfdb.fd_nplanes = m_nplanes;
  rg->mfdb.fd_wdwidth = m_wdwidth;
  rg->w = rg->mfdb.fd_w = m_w; rg->bc = rg->w >> 3;
  rg->h = rg->mfdb.fd_h = m_h; rg->lc = rg->h >> 3;
  rg->xfac += 8; rg->yfac += 8; rg->draw = draw_image;
  rg->sclick = sclick_image; rg->dclick = dummy_image;
  rg->free = dummy_image; rg->key = key_image;

  linelen = (th->ih.sl_width + 7) >> 3;
  rastsize = linelen * th->ih.planes;
  outlen = (long)th->comps * th->ih.sl_width;
  ip = Malloc( sizeof(IBUF) + rastsize + th->ih.pat_run + outlen );
  if (ip == 0)
  {
    Mfree( rg ); Fclose( f.handle ); form_error( EINVMEM ); return 0;
  }
  ip->line_adr = ip->pbuf = ip->pat_buf + th->ih.pat_run;
  ip->length = ip->bytes_left = rastsize;
  ip->linelen = linelen;
  ip->putline = put_true;
  ip->lines_left = th->ih.sl_height;
  ip->comps = th->comps;
  if (th->comps != jpgd_ptr.OutComponents) rgb_y_init();
  ip->bits[0] = th->red < 8 ? th->red : 8;
  ip->bits[1] = th->green < 8 ? th->green : 8;
  ip->bits[2] = th->blue < 8 ? th->blue : 8;
  ip->src_end[0] = ip->pbuf + linelen * th->red;
  ip->src_end[1] = ip->src_end[0] + linelen * th->green;
  ip->src_end[2] = ip->src_end[1] + linelen * th->blue;
  ip->vrc = 1;
  ip->jpgd_ptr = &jpgd_ptr;

  jpgd_ptr.OutTmpPointer = ip->pbuf + rastsize;
  ip->des_end = (char *)jpgd_ptr.OutTmpPointer + outlen;
  jpgd_ptr.OutTmpHeight++;
  dest_planes = m_nplanes;
  do_Create( &jpgd_ptr );
  Fbufopen( &f );
  if (setjmp( setjmp_buffer )) goto finish;
  if (th->ih.headlen)
    do { FSKIPC(&f); FSKIPC(&f); } while (--th->ih.headlen);
  for (;;)
  {
    FGETC(&f, cdata);
    if (((char)i = cdata) != 0)
    {
      if (cdata <<= 1)			/* solid run */
      {
	(char)i >>= 7; do IPUTC(ip, i); while (cdata -= 2);
      }
      else				/* byte string */
      {
	FGETC(&f, cdata);
	do FICOPYC(&f, ip); while (--cdata);
    } }
    else
    {
      FTESTC(&f);
      if ((cdata = FFETCHC(&f)) != 0)	/* pattern run */
      {
	pat_ptr = ip->pat_buf;
	for (i = th->ih.pat_run; --i >= 0;)
	  FGETC(&f, *pat_ptr++);
	do
	{ pat_ptr = ip->pat_buf;
	  for (i = th->ih.pat_run; --i >= 0;)
	    IPUTC(ip, *pat_ptr++);
	}
	while (--cdata);
      }
      else				/* vertical replication */
      {
	FGETC(&f, cdata);
	if (++cdata) do ISKIPC(ip); while (--cdata);
	else FGETC(&f, ip->vrc);
  } } }
  finish: Fbufclose( &f );
  do_Close();
  Mfree( ip );
  return rg;
}
