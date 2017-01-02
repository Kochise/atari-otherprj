#include <osbind.h>
#include <string.h>
#include <scancode.h>
#include <errno.h>
#include <aes.h>
#include <vdi.h>

#include "video.h"
#include "1stguide.h"
#include "util.h"

typedef struct node
{
  struct node *next;
}
NODE;

typedef struct guide
{
  int	handle, s1, s2,
	input;			/* File handle to incoming data.	*/
  int	lbc,			/* left-border-count fr Fenster.	*/
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

  struct guide *v_next;		/* Zeiger auf n„chste Videostruktur.	*/
  NODE base;
  void *old_adr;
  MFDB mfdb;
  long plane_size, image_size;
  void (*mraster)( unsigned *raster_ptr, int num_cols,
		   int num_rows, unsigned char *ptr );
  void (*craster)( unsigned *raster_ptr, int num_cols,
		   int num_rows, unsigned char **ptr );
  VidStream vid_stream;
  unsigned char buf_start[];
}
GUIDE;

FROM( image ) IMPORT void setconv( void );
FROM( 1stguide ) IMPORT GUIDE *vg;
FROM( jpegddrv ) IMPORT unsigned char dithermatrix[256];
FROM( jpegddrv ) IMPORT long screen_rgb[3][256];
FROM( jpegddrv ) IMPORT long plane_size;
FROM( jpegddrv ) IMPORT unsigned char *dith_row;
FROM( jpegddrv ) IMPORT void set_screen( void );
FROM( jpegddrv ) IMPORT void set_color_map( int *get_map );
void color_quantize1( unsigned *raster_ptr, int num_cols,
		      int num_rows, unsigned char *ptr );
void gray_quantize8( unsigned *raster_ptr, int num_cols,
		     int num_rows, unsigned char *ptr );
void gray16_put_pixel_rows( unsigned *raster_ptr, int num_cols,
			    int num_rows, unsigned char *ptr );
void gray24_put_pixel_rows( unsigned *raster_ptr, int num_cols,
			    int num_rows, unsigned char *ptr );
void gray32_put_pixel_rows( unsigned *raster_ptr, int num_cols,
			    int num_rows, unsigned char *ptr );

static char string2[] = { 0, 0, 0, 0 },
	    string3[] = "XXXXX *XXXXX Pixel";

#pragma warn -rpt
static OBJECT popup[] =
{
  0,  1,  7, G_BOX,    NONE,   SHADOWED, 0xFF1100L,		0, 0, 19,5,
  5, -1, -1, G_STRING, NONE,   DISABLED, string3,		0, 0, 19,1,
  3, -1, -1, G_BUTTON, NONE,   NORMAL,   "-- ^\004",		0, 1,  7,1,
  4, -1, -1, G_BUTTON, NONE,   DISABLED, string2,		7, 1,  5,1,
  6, -1, -1, G_BUTTON, NONE,   NORMAL,   "++ ^\003",		12,1,  7,1,
  2, -1, -1, G_STRING, NONE,   NORMAL,   "  Step          ^S",	0, 2, 19,1,
  7, -1, -1, G_STRING, NONE,   NORMAL,   "  Pause         ^E",	0, 3, 19,1,
  0, -1, -1, G_STRING, LASTOB, NORMAL,   "  Restart       ^T",	0, 4, 19,1,
};
#pragma warn +rpt

#ifdef VIDEOOPT

void rgb16_put_pixel_rows( unsigned *raster_ptr, int num_cols,
			   int num_rows, unsigned char **ptr );

void rgb24_put_pixel_rows( unsigned *raster_ptr, int num_cols,
			   int num_rows, unsigned char **ptr );

void rgb32_put_pixel_rows( unsigned *raster_ptr, int num_cols,
			   int num_rows, unsigned char **ptr );

void rgb3_put_pixel_rows( unsigned *raster_ptr, int num_cols,
			  int num_rows, unsigned char **ptr );

void rgb8_put_pixel_rows( unsigned *raster_ptr, int num_cols,
			  int num_rows, unsigned char **ptr );

#else

static void rgb16_put_pixel_rows( unsigned *raster_ptr, int num_cols,
				  int num_rows, unsigned char **ptr )
{
  unsigned char *ptr0, *ptr1, *ptr2;

  ptr0 = *ptr++; ptr1 = *ptr++; ptr2 = *ptr;
  do
  { int col = num_cols;
    do *raster_ptr++ =
	 *(unsigned *)&screen_rgb[0][*ptr0++] |
	 *(unsigned *)&screen_rgb[1][*ptr1++] |
	 *(unsigned *)&screen_rgb[2][*ptr2++];
    while (--col > 0);
    col -= num_cols; col &= 15;
    ptr0 += col; ptr1 += col; ptr2 += col;
    col <<= 1; (char *)raster_ptr += col;
  }
  while (--num_rows > 0);
}

static void rgb24_put_pixel_rows( unsigned *raster_ptr, int num_cols,
				  int num_rows, unsigned char **ptr )
{
  unsigned char *ptr0, *ptr1, *ptr2;

  ptr0 = *ptr++; ptr1 = *ptr++; ptr2 = *ptr;
  do
  { int col = num_cols;
    do
    { long i, j, k;
      i  = screen_rgb[0][*ptr0++];
      i |= screen_rgb[1][*ptr1++];
      i |= screen_rgb[2][*ptr2++];
      i >>= 8; j = i >> 8; k = j >> 8;
      *((char *)raster_ptr)++ = k;
      *((char *)raster_ptr)++ = j;
      *((char *)raster_ptr)++ = i;
    }
    while (--col > 0);
    col -= num_cols; col &= 15;
    ptr0 += col; ptr1 += col; ptr2 += col;
    (char *)raster_ptr += col; col <<= 1;
    (char *)raster_ptr += col;
  }
  while (--num_rows > 0);
}

static void rgb32_put_pixel_rows( unsigned *raster_ptr, int num_cols,
				  int num_rows, unsigned char **ptr )
{
  unsigned char *ptr0, *ptr1, *ptr2;

  ptr0 = *ptr++; ptr1 = *ptr++; ptr2 = *ptr;
  do
  { int col = num_cols;
    do *((long *)raster_ptr)++ =
	 screen_rgb[0][*ptr0++] |
	 screen_rgb[1][*ptr1++] |
	 screen_rgb[2][*ptr2++];
    while (--col > 0);
    col -= num_cols; col &= 15;
    ptr0 += col; ptr1 += col; ptr2 += col;
    col <<= 2; (char *)raster_ptr += col;
  }
  while (--num_rows > 0);
}

static void rgb3_put_pixel_rows( unsigned *raster_ptr, int num_cols,
				 int num_rows, unsigned char **ptr )
{
  char pixcode;
  long planesize;
  int col, planes;
  unsigned mask, *plane_ptr;
  unsigned char dith, *dith_ptr;
  unsigned char *ptr0, *ptr1, *ptr2;

  planesize = plane_size; planes = nplanes;
  dith_ptr = dithermatrix;
  ptr0 = *ptr++; ptr1 = *ptr++; ptr2 = *ptr;
  do
  { col = num_cols;
    for (;;)
    {
      mask = 0x8000;
      do
      { dith = *dith_ptr++; pixcode = 0;
	if (dith < *ptr0++) pixcode += 4;
	if (dith < *ptr1++) pixcode += 2;
	if (dith < *ptr2++) pixcode += 1;
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
    nextrow: raster_ptr++; col -= num_cols;
    {
      long i = 15; (int)i &= col;
      dith_ptr += i; ptr0 += i; ptr1 += i; ptr2 += i;
    }
    if (dith_ptr == dithermatrix + 256) dith_ptr -= 256;
  }
  while (--num_rows > 0);
}

static void rgb8_put_pixel_rows( unsigned *raster_ptr, int num_cols,
				 int num_rows, unsigned char **ptr )
{
  char pixcode;
  int val, col;
  unsigned mask;
  long planesize;
  unsigned char dith, *dith_ptr;
  unsigned char *ptr0, *ptr1, *ptr2;

  planesize = plane_size;
  dith_ptr = dithermatrix;
  ptr0 = *ptr++; ptr1 = *ptr++; ptr2 = *ptr;
  do
  { col = num_cols;
    for (;;)
    {
      mask = 0x8000;
      do
      { (char *)raster_ptr += planesize << 3;
	dith = *dith_ptr++; pixcode = 0;
	val = *ptr0++; val += val << 2;
	if (dith < (unsigned char)val) ++pixcode;
	val >>= 8; pixcode += (char)val;
	pixcode += pixcode << 1; pixcode <<= 1;
	val = *ptr1++; val += val << 2;
	if (dith < (unsigned char)val) ++pixcode;
	val >>= 8; pixcode += (char)val;
	pixcode += pixcode << 1; pixcode <<= 1;
	val = *ptr2++; val += val << 2;
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
    dith_ptr += col; ptr0 += col; ptr1 += col; ptr2 += col;
    if (dith_ptr == dithermatrix + 256) dith_ptr -= 256;
  }
  while (--num_rows > 0);
}

#endif

static void draw_video( GUIDE *tg, int *clip )
{
  static int index[2] = { 1, 0 };
  int  pxy[8];
  MFDB d;

  pxy[0] = (tg->lbc << 3) + clip[0] - tg->x;
  pxy[1] = (tg->tlc << 3) + clip[1] - tg->y;
  pxy[2] = pxy[0] + clip[2] - clip[0];
  pxy[3] = pxy[1] + clip[3] - clip[1];
  if (tg->mfdb.fd_w - 1 < pxy[2]) pxy[2] = tg->mfdb.fd_w - 1;
  if (tg->mfdb.fd_h - 1 < pxy[3]) pxy[3] = tg->mfdb.fd_h - 1;
  *(long *)(pxy + 4) = *(long *)clip;
  pxy[6] = clip[0] + pxy[2] - pxy[0];
  pxy[7] = clip[1] + pxy[3] - pxy[1];
  if (pxy[6] < clip[2] || pxy[7] < clip[3])
    if (pxy[6] < clip[2] && pxy[7] < clip[3] &&
	pxy[6] >= clip[0] && pxy[7] >= clip[1])
    {
      clip[0] = pxy[6] + 1; pxy[5] = clip[3]; clip[3] = pxy[7];
      vr_recfl( handle, clip );
      clip[0] = pxy[4];
      clip[3] = pxy[5]; pxy[5] = clip[1]; clip[1] = pxy[7] + 1;
      vr_recfl( handle, clip );
    }
    else
    {
      if (pxy[7] >= clip[3])
	if (pxy[6] + 1 > clip[0]) clip[0] = pxy[6] + 1;
      if (pxy[6] >= clip[2])
	if (pxy[7] + 1 > clip[1]) clip[1] = pxy[7] + 1;
      vr_recfl( handle, clip );
      if (pxy[6] < pxy[4] || pxy[7] < pxy[5]) return;
    }
  tg->mfdb.fd_stand = 0; d.fd_addr = 0;
  if (tg->mfdb.fd_nplanes == 1)
    vrt_cpyfm( handle, MD_REPLACE, pxy, &tg->mfdb, &d, index );
  else vro_cpyfm( handle, S_ONLY, pxy, &tg->mfdb, &d );
}

static void transform( MFDB *des, long size )
{
  MFDB src;

  src = *des;
  src.fd_stand++;
  (char *)des->fd_addr -= size;
  vr_trnfm( handle, &src, des );
}

static void do_raster( GUIDE *tg )
{
  if (tg->mfdb.fd_nplanes < 16)
  {
    (char *)tg->mfdb.fd_addr += tg->image_size;
    memset( tg->mfdb.fd_addr, 0, tg->image_size );
    plane_size = tg->plane_size; dith_row = dithermatrix;
  }
  if (tg->vid_stream.red)
    (*tg->vid_stream.y_scale)( tg->vid_stream.current->luminance,
			       tg->vid_stream.red,
			       tg->vid_stream.mb_width << 4,
			       tg->vid_stream.mb_height << 4,
			       tg->mfdb.fd_wdwidth << 4 );
  if (tg->vid_stream.color == 0)
    (*tg->mraster)( tg->mfdb.fd_addr,
		    tg->vid_stream.dest_width,
		    tg->vid_stream.dest_height,
		    tg->vid_stream.red
		      ? tg->vid_stream.red
		      : tg->vid_stream.current->luminance );
  else
  {
    (*tg->vid_stream.c_scale)( tg->vid_stream.current->Cr,
			       tg->vid_stream.green,
			       tg->vid_stream.mb_width << 3,
			       tg->vid_stream.mb_height << 3,
			       tg->mfdb.fd_wdwidth << 4 );
    (*tg->vid_stream.c_scale)( tg->vid_stream.current->Cb,
			       tg->vid_stream.blue,
			       tg->vid_stream.mb_width << 3,
			       tg->vid_stream.mb_height << 3,
			       tg->mfdb.fd_wdwidth << 4 );
    (*tg->vid_stream.c_convert)( &tg->vid_stream );
    (*tg->craster)( tg->mfdb.fd_addr,
		    tg->vid_stream.dest_width,
		    tg->vid_stream.dest_height,
		    &tg->vid_stream.red );
  }
  if (tg->mfdb.fd_nplanes < 16)
    transform( &tg->mfdb, tg->image_size );
}

static void restart( GUIDE *tg )
{
  if (Fseek( 0, tg->input, 0 ) == 0)
    tg->vid_stream.init_flag = 0;
}

static void step( GUIDE *tg )
{
  if (tg->mfdb.fd_stand == 0)
  {
    if (mpegVidRsrc( &tg->vid_stream, 0 ))
    {
      restart( tg );
      mpegVidRsrc( &tg->vid_stream, 0 );
    }
    do_raster( tg );
    tg->mfdb.fd_stand++;
  }
  full_redraw();
}

#pragma warn -par
static void dclick_video( GUIDE *tg, int mx, int my, int ks )
{
  if (ks += 3) return;
  vg = vg->v_next;
  if (tg->handle) step( tg );
}

static void put_message( const char *msg, ... ) { }
#pragma warn +par

static void *alloc( VidStream *vs, unsigned long size )
{
  NODE *b, *p;

  b = &((GUIDE *)((char *)vs - (int)&((GUIDE *)0)->vid_stream))->base;
  p = Malloc( size + sizeof(NODE) );
  if (p)
  {
    p->next = b->next; b->next = p; p++;
  }
  return p;
}

static int free( VidStream *vs, void *ptr )
{
  NODE *b, *p;

  b = &((GUIDE *)((char *)vs - (int)&((GUIDE *)0)->vid_stream))->base;
  p = (NODE *)ptr - 1;
  while (b->next)
  {
    if (b->next == p)
    {
      b->next = p->next; Mfree( p ); return 0;
    }
    b = b->next;
  }
  return 1;
}

static void free_all( GUIDE *tg )
{
  NODE *new, *old = tg->base.next;

  while (old) { new = old; old = new->next; Mfree( new ); }
}

static void pause( GUIDE *tg )
{
  GUIDE *lg;

  struplo( tg->path ); set_name();
  lg = vg;
  if (lg)
  {
    do
      if (lg->v_next == tg)
      {
	if (tg == lg) vg = 0;
	else
	{
	  vg = tg->v_next;
	  lg->v_next = vg;
	}
	return;
      }
    while ((lg = lg->v_next) != vg);
    tg->v_next = lg->v_next; lg->v_next = tg;
  }
  else { tg->v_next = tg; ekind |= MU_TIMER; }
  vg = tg;
}

static void change_scale( GUIDE *tg, int mx, int my, int delta )
{
  int oldscale, dx, dy, d[3];

  if (tg->mfdb.fd_addr != tg->old_adr)
  {
    free( &tg->vid_stream, tg->mfdb.fd_addr );
    tg->mfdb.fd_addr = tg->old_adr;
  }
  delta += oldscale = tg->vid_stream.scale;
  start:
  mpegNewScale( &tg->vid_stream, delta );
  tg->mfdb.fd_w = (tg->vid_stream.dest_width + 7) & -8;
  tg->mfdb.fd_h = (tg->vid_stream.dest_height + 7) & -8;
  tg->mfdb.fd_wdwidth = (tg->vid_stream.dest_width + 15) >> 4;
  tg->plane_size = 2L * tg->mfdb.fd_wdwidth * tg->mfdb.fd_h;
  tg->image_size = tg->plane_size * tg->mfdb.fd_nplanes;
  if ((delta = tg->vid_stream.scale) > 0)
  {
    void *p = alloc( &tg->vid_stream,
      tg->mfdb.fd_nplanes < 16 ? tg->image_size << 1 : tg->image_size );
    if (p == 0) { delta--; goto start; }
    tg->mfdb.fd_addr = p;
  }
  memset( tg->mfdb.fd_addr, 0, tg->image_size ); do_raster( tg );
  if ((delta -= oldscale) == 0) return;
  if (mx < 0) { graf_mkstate( d + 1, d + 2, d, d ); mx = d[1]; my = d[2]; }
  tg->bc = (tg->mfdb.fd_w - tg->w) >> 3;
  tg->lc = (tg->mfdb.fd_h - tg->h) >> 3;
  dx = tg->lbc; mx -= tg->x; mx += 4; mx >>= 3; dx += mx;
  dy = tg->tlc; my -= tg->y; my += 4; my >>= 3; dy += my;
  if (delta > 0) { dx <<= delta; dy <<= delta; }
  else { delta = -delta; dx >>= delta; dy >>= delta; }
  dx -= mx; if (tg->bc < dx) dx = tg->bc; if (dx < 0) dx = 0;
  dy -= my; if (tg->lc < dy) dy = tg->lc; if (dy < 0) dy = 0;
  tg->lbc = dx; tg->tlc = dy; set_allslider(); full_redraw();
}

#pragma warn -par
static int key_video( GUIDE *tg, int code, int ks )
{
  switch (code)
  {
    case CNTRL_CL:
      if (tg->vid_stream.scale >= -2) change_scale(tg,-1,0,-1); return 0;
    case CNTRL_CR:
      if (tg->vid_stream.scale <=  2) change_scale(tg,-1,0, 1); return 0;
    case CNTRL_T: restart( tg );
    case CNTRL_S: step( tg ); return 0;
    case CNTRL_E: pause( tg ); return 0;
  }
  return 1;
}
#pragma warn +par

static void sclick_video( GUIDE *tg, int mx, int my )
{
  popup[4].ob_state = tg->vid_stream.scale <=  2 ? NORMAL : DISABLED;
  popup[2].ob_state = tg->vid_stream.scale >= -2 ? NORMAL : DISABLED;
  {
    char v = ' ', z = '0', *p = string2;
    if (tg->vid_stream.scale < 0) { v = '-'; z -= tg->vid_stream.scale; }
    if (tg->vid_stream.scale > 0) { v = '+'; z += tg->vid_stream.scale; }
    *p++ = v; *p++ = z; *p++ = v;
  }
  itostring( tg->vid_stream.h_size, string3 + 5, 6 );
  itostring( tg->vid_stream.v_size, string3 + 12, 6 );
  switch (popup_menu( popup, 3, mx, my, objc_draw ))
  {
    case 2: change_scale( tg, mx, my, -1 ); break;
    case 4: change_scale( tg, mx, my,  1 ); break;
    case 7: restart( tg );
    case 5: step( tg ); break;
    case 6: pause( tg );
} }

static long get_more_data( VidStream *vs, unsigned char **buf_ptr )
{
  GUIDE *tg = (GUIDE *)((char *)vs - (int)&((GUIDE *)0)->vid_stream);

  *buf_ptr = tg->buf_start;
  return Fread( tg->input, BUF_LENGTH, tg->buf_start );
}

static void free_video( GUIDE *tg )
{
  GUIDE *lg;

  free_all( tg ); Fclose( tg->input );
  lg = vg;
  if (lg)
    do
      if (lg->v_next == tg)
      {
	if (tg == lg) vg = 0;
	else
	{
	  vg = tg->v_next;
	  lg->v_next = vg;
	}
	return;
      }
    while ((lg = lg->v_next) != vg);
}

GUIDE *load_mpg( int fh )
{
  GUIDE *rg;
  int i;

  if ((rg = Malloc( sizeof(GUIDE) + BUF_LENGTH )) == 0)
    { i = EINVMEM; form_error( i ); Fclose( fh ); return 0; }
  memset( rg, 0, sizeof(GUIDE) ); rg->input = fh;
  rg->vid_stream.get_more_data = get_more_data;
  rg->vid_stream.put_message = put_message;
  rg->vid_stream.alloc = alloc;
  rg->vid_stream.free = free;
  rg->vid_stream.No_P_Flag =
  rg->vid_stream.No_B_Flag = ks & 1;
  rg->mfdb.fd_nplanes = nplanes;
  if (rg->mfdb.fd_nplanes >= 3)
    rg->vid_stream.color = par.dithmod;
  if ((i = mpegVidRsrc( &rg->vid_stream, 0 )) != 0)
  {
    if (i == 4) i = EINVMEM; else i = ENOENT;
    free_all( rg ); Mfree( rg );
    form_error( i ); Fclose( fh ); return 0;
  }
  if (rg->mfdb.fd_nplanes < 3 || rg->vid_stream.color == 0 &&
      (par.dithcol == 0 || rg->mfdb.fd_nplanes < 8))
  {
    rg->mfdb.fd_nplanes = 1;
    rg->mraster = color_quantize1;
    rg->craster = rgb3_put_pixel_rows;
  }
  else if (rg->mfdb.fd_nplanes < 16)
  {
    if (par.dithcol == 0 || rg->mfdb.fd_nplanes < 8)
    {
      rg->mraster = color_quantize1;
      rg->craster = rgb3_put_pixel_rows;
    }
    else
    {
      rg->mraster = gray_quantize8;
      rg->craster = rgb8_put_pixel_rows;
      setconv(); set_color_map( 0 );
  } }
  else
  {
    if (rg->mfdb.fd_nplanes < 24)
    {
      rg->mraster = gray16_put_pixel_rows;
      rg->craster = rgb16_put_pixel_rows;
    }
    else if (rg->mfdb.fd_nplanes < 32)
    {
      rg->mraster = gray24_put_pixel_rows;
      rg->craster = rgb24_put_pixel_rows;
    }
    else
    {
      rg->mraster = gray32_put_pixel_rows;
      rg->craster = rgb32_put_pixel_rows;
    }
    set_screen();
  }
  rg->mfdb.fd_wdwidth = rg->vid_stream.mb_width;
  rg->w = rg->mfdb.fd_w = (rg->vid_stream.h_size + 7) & -8;
  rg->bc = rg->w >> 3;
  rg->h = rg->mfdb.fd_h = (rg->vid_stream.v_size + 7) & -8;
  rg->lc = rg->h >> 3;
  rg->plane_size = 16L * rg->mfdb.fd_wdwidth * rg->lc;
  rg->image_size = rg->plane_size * rg->mfdb.fd_nplanes;
  rg->old_adr = alloc( &rg->vid_stream,
    rg->mfdb.fd_nplanes < 16 ? rg->image_size << 1 : rg->image_size );
  if ((rg->mfdb.fd_addr = rg->old_adr) == 0)
  {
    i = EINVMEM;
    free_all( rg ); Mfree( rg );
    form_error( i ); Fclose( fh ); return 0;
  }
  memset( rg->mfdb.fd_addr, 0, rg->image_size );
  if (popup->ob_next == 0) { --popup->ob_next; fix_tree( popup, 7 ); }
  rg->xfac += 8; rg->yfac += 8;
  rg->draw = draw_video; rg->free = free_video; rg->key = key_video;
  rg->sclick = sclick_video; rg->dclick = dclick_video;
  if (vg) { rg->v_next = vg->v_next; vg->v_next = rg; }
  else { rg->v_next = rg; ekind |= MU_TIMER; }
  vg = rg; return rg;
}
