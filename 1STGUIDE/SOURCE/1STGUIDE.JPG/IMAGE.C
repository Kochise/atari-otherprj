#include <aes.h>
#include <vdi.h>
#include <osbind.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <scancode.h>
#include <errno.h>

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

#include "1stguide.h"
#include "util.h"

FROM( util ) IMPORT jmp_buf setjmp_buffer;

#ifdef __TOS__
#define X_MAGIC  'XIMG'
#define T_MAGIC  'TIMG'
#define FM_MAGIC 'FORM'
#define FM_IMAGE 'ILBM'
#define BM_MAGIC 'BMHD'
#define CM_MAGIC 'CMAP'
#define BO_MAGIC 'BODY'
#else
#define X_MAGIC  'X'+((long)'I'<<8)+((long)'M'<<16)+((long)'G'<<24)
#define T_MAGIC  'T'+((long)'I'<<8)+((long)'M'<<16)+((long)'G'<<24)
#define FM_MAGIC 'F'+((long)'O'<<8)+((long)'R'<<16)+((long)'M'<<24)
#define FM_IMAGE 'I'+((long)'L'<<8)+((long)'B'<<16)+((long)'M'<<24)
#define BM_MAGIC 'B'+((long)'M'<<8)+((long)'H'<<16)+((long)'D'<<24)
#define CM_MAGIC 'C'+((long)'M'<<8)+((long)'A'<<16)+((long)'P'<<24)
#define BO_MAGIC 'B'+((long)'O'<<8)+((long)'D'<<16)+((long)'Y'<<24)
#endif

typedef struct
{
  int version;
  unsigned headlen;
  int planes, pat_run, pix_width, pix_height, sl_width, sl_height;
}
IMG_HEADER;

typedef struct
{
  long magic, length;
}
CHUNK_HEADER;

typedef struct
{
  long fm_magic, fm_length, fm_type;
}
FORMCHUNK;

typedef struct
{
  long bm_magic, bm_length;
  int  bm_width, bm_height, bm_x, bm_y;
  char bm_nPlanes, bm_Masking, bm_Compression, bm_pad1;
  int  bm_transparentColor;
  char bm_xAspect, bm_yAspect;
  int  bm_pageWidth, bm_pageHeight;
}
BMHDCHUNK;

static long four[256];
static int  doub[256], maxconv;
static char half[512];
unsigned char Pli2Vdi[256], Vdi2Pli[256];
static char string2[] = { 0, 0, 0, 0 },
	    string3[] = "XXXXX *XXXXX Pixel",
	    string4[] = "---- XX Ebenen ----",
	    string5[] = " Sichern (XIMG) ^X",
	    ostring[] = "  Original-Pal. ^E",
	    sstring[] = "  Standard-Pal. ^S";
char dith64matrix[64] =
     {
	17, 61, 27, 51, 18, 63, 25, 49,
	41,  5, 38, 14, 42,  7, 37, 13,
	31, 55, 20, 56, 28, 52, 22, 59,
	34, 10, 44,  0, 32,  8, 46,  3,
	19, 62, 24, 48, 16, 60, 26, 50,
	43,  6, 36, 12, 40,  4, 39, 15,
	29, 53, 23, 58, 30, 54, 21, 57,
	33,  9, 47,  2, 35, 11, 45,  1
     };
static int defcols[16][3] =
	   {
		1000, 1000, 1000,
		1000,    0,    0,
		   0, 1000,    0,
		1000, 1000,    0, 
		   0,    0, 1000,
		1000,    0, 1000,
		   0, 1000, 1000,
		 666,  666,  666,
		 333,  333,  333,
		 666,    0,    0,
		   0,  666,    0,
		 666,  666,    0,
		   0,    0,  666,
		 666,    0,  666,
		   0,  666,  666,
		   0,    0,    0
	   };
#pragma warn -rpt
static OBJECT popup[] =
{
  0,  1,  9, G_BOX,    NONE,   SHADOWED, 0xFF1100L,		0, 0, 19,7,
  5, -1, -1, G_STRING, NONE,   DISABLED, string3,		0, 0, 19,1,
  3, -1, -1, G_BUTTON, NONE,   NORMAL,   "-- ^\004",		0, 1,  7,1,
  4, -1, -1, G_BUTTON, NONE,   DISABLED, string2,		7, 1,  5,1,
  6, -1, -1, G_BUTTON, NONE,   NORMAL,   "++ ^\003",		12,1,  7,1,
  2, -1, -1, G_STRING, NONE,   NORMAL,   ostring,		0, 2, 19,1,
  7, -1, -1, G_STRING, NONE,   DISABLED, string4,		0, 3, 19,1,
  8, -1, -1, G_STRING, NONE,   DISABLED, "  Transparent   ^K",	0, 4, 19,1,
  9, -1, -1, G_STRING, NONE,   DISABLED, "--- Farbpalette ---", 0, 5, 19,1,
  0, -1, -1, G_STRING, LASTOB, NORMAL,   string5,		0, 6, 19,1
};
#pragma warn +rpt

static long gray64( int *rgb )
{
  return (299L * 2L * rgb[0] +
	  587L * 2L * rgb[1] +
	  114L * 2L * rgb[2] + 15625) / 31250;
}

static long gray255( int *rgb )
{
  return (299L * 51L * rgb[0] +
	  587L * 51L * rgb[1] +
	  114L * 51L * rgb[2] + 100000L) / 200000L;
}

static int get_pli( int vdi )
{
  int pel[2];

  vsm_color( handle, vdi );
  *(long *)pel = 0; v_pmarker( handle, 1, pel );
  v_get_pixel( handle, 0, 0, pel, pel + 1 );
  return *pel;
}

static void get_true( void *buf, int *rgb )
{
  static int pxy[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  static MFDB screen = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	      check  = { 0, 1, 1, 1, 0, 0, 0, 0, 0 };

  check.fd_addr = buf;
  check.fd_nplanes = nplanes;
  vs_color( handle, 1, rgb );
  v_pmarker( handle, 1, pxy );
  vro_cpyfm( handle, S_ONLY, pxy, &screen, &check );
}

static void get_gray( void *buf, int *rgb )
{
  int lum[3];

  lum[0] = lum[1] = lum[2] = (int)
    ((299L * rgb[0] + 587L * rgb[1] + 114L * rgb[2] + 500) / 1000);
  get_true( buf, lum );
}

static int pli2vdi( int pli, int plimax )
{
  if (ks & 1) return pli;
  if (pli == plimax) return 1;
  if (pli > maxconv) return pli;
  return Pli2Vdi[pli];
}

static int vdi2pli( int vdi, int plimax )
{
  if ((ks & 1) == 0) return vdi;
  if (vdi == 1) return plimax;
  if (vdi > maxconv) return vdi;
  return Vdi2Pli[vdi];
}

void setconv( void )
{
  static char vdi2pli[] = { 0,15,1,2,4,6,3,5,7,8,9,10,12,14,11,13 };
  int i, k;

  if (popup->ob_next) return;
  --popup->ob_next; fix_tree( popup, 9 );
  i = 255;
  do
  { char c = (1 << 7) & i;
    c += ((1 << 5) & i) << 1;
    c += ((1 << 3) & i) << 2;
    c += ((1 << 1) & i) << 3; half[i] = c;
    (unsigned char)c >>= 4; half[256 + i] = c;
    k = 0; if ((c = i) < 0) k += (1 << 15) + (1 << 14);
    if ((c <<= 1) < 0) k += (1 << 13) + (1 << 12);
    if ((c <<= 1) < 0) k += (1 << 11) + (1 << 10);
    if ((c <<= 1) < 0) k += (1 <<  9) + (1 <<  8);
    if ((c <<= 1) < 0) k += (1 <<  7) + (1 <<  6);
    if ((c <<= 1) < 0) k += (1 <<  5) + (1 <<  4);
    if ((c <<= 1) < 0) k += (1 <<  3) + (1 <<  2);
    if ((c <<= 1) < 0) k += 3; doub[i] = k;
  }
  while (--i >= 0);
  i = 255;
  do
  { k = doub[i];
    ((int *)&four[i])[0] = doub[(unsigned)k >> 8];
    ((int *)&four[i])[1] = doub[(unsigned char)k];
  }
  while (--i >= 0);
  k = nplanes; i = 15;
  if (k > 4 && k < 16) i = ((1 << k) - 1) & 255;
  maxconv = i;
  do
  { if (maxconv <= 15) k = vdi2pli[i];
    else
    {
      k = get_pli( i );
      if ((unsigned)maxconv < (unsigned)k) k = maxconv;
    }
    Vdi2Pli[i] = k;
    Pli2Vdi[k] = i;
  }
  while (--i >= 0);
}

static void switch_colors( GUIDE *tg )
{
  int colors, k, *rgb_list, rgb_get[3];

  if ((tg->x_flag & 1) == 0) return;
  rgb_list = tg->rgb_list; if (*rgb_list++) return;
  colors = 1 << min( tg->planes, nplanes );
  for (k = 0; k < colors; ++k)
  {
    vq_color( handle, k, 0, rgb_get );
    vs_color( handle, k, rgb_list );
    *rgb_list++ = rgb_get[0];
    *rgb_list++ = rgb_get[1];
    *rgb_list++ = rgb_get[2];
  }
  tg->x_flag ^= 4;
}

static void write_palette( int xflag, int fh, int count, int *rgb_list )
{
  int pli;
  unsigned *rgb_ptr;
  unsigned char c[4];

  count = count / 3 - 1; pli = 0;
  do
  { rgb_ptr = (unsigned *)rgb_list + 3 * pli2vdi( pli, count );
    if (xflag) Fwrite( fh, 6, rgb_ptr );
    else
    {
      c[0] = (unsigned char)((*rgb_ptr++ * 51U + 100U) / 200U);
      c[1] = (unsigned char)((*rgb_ptr++ * 51U + 100U) / 200U);
      c[2] = (unsigned char)((*rgb_ptr++ * 51U + 100U) / 200U);
      Fwrite( fh, 3, c );
  } }
  while (++pli <= count);
}

static void save_palette( GUIDE *tg )
{
  long count, k, j;
  int  fh, i, *rgb_list;
  char *buf;

  rgb_list = tg->rgb_list; *rgb_list++ = 0;
  i = 1 << min( tg->planes, nplanes ); fh = 0;
  do { vq_color( handle, fh, 0, rgb_list ); rgb_list += 3; }
  while (++fh < i);
  graf_mouse( BUSYBEE, 0 );
  if ((fh = Fopen( tg->path, 2 )) < 0)
  {
    graf_mouse( ARROW, 0 ); form_error( -31 - fh ); return;
  }
  rgb_list = tg->rgb_list + 1; i = (1 << tg->planes) * 3;
  if (tg->x_flag & 1)
  {
    if (tg->x_flag & 2)
    {
      CHUNK_HEADER ch;

      ch.length = sizeof(FORMCHUNK);
      do
      { Fseek( (ch.length + 1) & -2, fh, 1 );
	if (Fread( fh, 8, &ch ) - 8)
	{
	  Fclose( fh ); graf_mouse( ARROW, 0 );
	  form_error( ENOENT ); return;
	}
#ifndef __TOS__
	fliplongs( (int *)&ch.length, 1, 1 );
#endif
      }
      while (ch.magic != CM_MAGIC);
      write_palette( 0, fh, min( (int)ch.length, i ), rgb_list );
    }
    else
    {
      Fseek( sizeof(IMG_HEADER) + 4, fh, 0 );
#ifndef __TOS__
      flipwords( (char *)(rgb_list - 1), (i + 1) << 1 );
#endif
      Fwrite( fh, 2, rgb_list - 1 ); write_palette( 1, fh, i, rgb_list );
#ifndef __TOS__
      flipwords( (char *)(rgb_list - 1), (i + 1) << 1 );
#endif
  } }
  else
  {
    count = Fseek( 0, fh, 2 );
    if ((buf = Malloc( count )) == 0)
    {
      Fclose( fh ); graf_mouse( ARROW, 0 ); form_error( EINVMEM ); return;
    }
    Fseek( 0, fh, 0 ); count = Fread( fh, count, buf ); Fseek( 0, fh, 0 );
    if (tg->x_flag & 2)
    {
      k = 12;
      do
      {	if ((j = k) > count)
	{
	  Mfree( buf ); Fclose( fh ); graf_mouse( ARROW, 0 );
	  form_error( ENOENT ); return;
	}
#ifndef __TOS__
	fliplongs( (int *)(buf + j + 4), 1, 1 );
#endif
	k += (*(long *)(buf + j + 4) + 9) & -2;
#ifndef __TOS__
	fliplongs( (int *)(buf + j + 4), 1, 1 );
#endif
      }
      while (*(long *)(buf + j) != BM_MAGIC);
#ifndef __TOS__
      fliplongs( (int *)(buf + 4), 1, 1 );
#endif
      *(long *)(buf + 4) += i + 8;
#ifndef __TOS__
      fliplongs( (int *)(buf + 4), 1, 1 );
#endif
      Fwrite( fh, k, buf ); *(long *)buf = i;
#ifndef __TOS__
      fliplongs( (int *)buf, 1, 1 );
#endif
      Fwrite( fh, 4, "CMAP" ); Fwrite( fh, 4, buf );
      write_palette( 0, fh, i, rgb_list );
    }
    else
    {
#ifndef __TOS__
      flipwords( buf, sizeof(IMG_HEADER) );
#endif
      k = ((IMG_HEADER *)buf)->headlen << 1;
      ((IMG_HEADER *)buf)->headlen = 11 + i;
#ifndef __TOS__
      flipwords( buf, sizeof(IMG_HEADER) );
      flipwords( (char *)(rgb_list - 1), (i + 1) << 1 );
#endif
      Fwrite( fh, sizeof(IMG_HEADER), buf );
      Fwrite( fh, 4, "XIMG" );
      Fwrite( fh, 2, rgb_list - 1 ); write_palette( 1, fh, i, rgb_list );
#ifndef __TOS__
      flipwords( (char *)(rgb_list - 1), (i + 1) << 1 );
#endif
    }
    Fwrite( fh, count - k, buf + k ); Mfree( buf ); tg->x_flag |= 1;
  }
  Fclose( fh ); graf_mouse( ARROW, 0 );
}

static void transform( MFDB *src, MFDB *des )
{
  long size;

  *src = *des; size = 2L * src->fd_wdwidth * src->fd_h * src->fd_nplanes;
  if ((src->fd_addr = Malloc( size )) == 0)
  {
    if (src->fd_nplanes > 1) return;
    src->fd_addr = des->fd_addr;
  }
  else memcpy( src->fd_addr, des->fd_addr, size );
#ifndef __TOS__
  if (src->fd_stand) flipwords( src->fd_addr, size );
#endif
  des->fd_stand ^= 1; vr_trnfm( handle, src, des );
#ifndef __TOS__
  if (des->fd_stand) flipwords( des->fd_addr, size );
#endif
  if (src->fd_addr != des->fd_addr) Mfree( src->fd_addr );
}

void draw_image( GUIDE *tg, int *clip )
{
  static int index[2] = { 1, 0 };
  char  *des;
  long  size1, size2, lxy[4];
  int   i, k, pxy[8];
  GUIDE *hg;
  MFDB  d, s;

  lxy[0] = ((long)tg->lbc << 3) + (clip[0] - tg->x);
  lxy[1] = ((long)tg->tlc << 3) + (clip[1] - tg->y);
  lxy[2] = lxy[0] + (clip[2] - clip[0]);
  lxy[3] = lxy[1] + (clip[3] - clip[1]);
  size1 = tg->mfdb.fd_w; size2 = tg->mfdb.fd_h;
  if ((i = tg->scale) > 0) { size1 <<= i; size2 <<= i; }
  if (i < 0) { size1 >>= -i; size2 >>= -i; }
  if (size1 <= lxy[2]) lxy[2] = --size1;
  if (size2 <= lxy[3]) lxy[3] = --size2;
  pxy[0] = (int)lxy[0]; pxy[1] = (int)lxy[1];
  pxy[2] = (int)lxy[2]; pxy[3] = (int)lxy[3];
  *(long *)(pxy + 4) = *(long *)clip;
  pxy[6] = clip[0] + pxy[2] - pxy[0];
  pxy[7] = clip[1] + pxy[3] - pxy[1];
  if (pxy[6] < clip[2] || pxy[7] < clip[3])
  {
    if (pxy[7] >= clip[3])
      if (pxy[6] + 1 > clip[0]) clip[0] = pxy[6] + 1;
    if (pxy[6] >= clip[2])
      if (pxy[7] + 1 > clip[1]) clip[1] = pxy[7] + 1;
    vr_recfl( handle, clip );
    if (pxy[6] < pxy[4] || pxy[7] < pxy[5]) return;
  }
  if (i == 0)
  {
    if (tg->mfdb.fd_stand) transform( &s, &tg->mfdb );
    if (tg->mfdb.fd_stand == 0)
    {
      d.fd_addr = 0;
      if (tg->mfdb.fd_nplanes == 1)
      {
	if (nplanes >= 16) switch_colors( tg );
	vrt_cpyfm( handle, MD_REPLACE, pxy, &tg->mfdb, &d, index );
	if (nplanes >= 16) switch_colors( tg );
      }
      else vro_cpyfm( handle, S_ONLY, pxy, &tg->mfdb, &d );
      return;
  } }
  if (tg->mfdb.fd_stand == 0) transform( &s, &tg->mfdb );
  d = tg->mfdb;
  if (i <= 0)
  {
    pxy[3] -= pxy[1]; pxy[1] = 0; d.fd_h = pxy[3] + 1;
    if (i < 0)
      { d.fd_w = ((pxy[2] >> 3) - (pxy[0] >> 3) + 1) << 3; k = 7; }
    else
      { d.fd_w = ((pxy[2] >> 4) - (pxy[0] >> 4) + 1) << 4; k = 15; }
  }
  else
  {
    d.fd_h = ((pxy[3] >> i) - (pxy[1] >> i) + 1) << i;
    k = (1 << i) - 1; i += 3; pxy[3] -= pxy[1] & ~k; pxy[1] &= k;
    d.fd_w = ((pxy[2] >> i) - (pxy[0] >> i) + 1) << i;
    k = (1 << i) - 1; i -= 3;
  }
  pxy[2] -= pxy[0] & ~k; pxy[0] &= k; d.fd_wdwidth = (d.fd_w + 15) >> 4;
  if (d.fd_stand == 0 ||
      (des = Malloc( 4L * d.fd_wdwidth * d.fd_h * d.fd_nplanes )) == 0)
  {
    vsf_interior( handle, FIS_USER );
    vr_recfl( handle, pxy + 4 );
    vsf_interior( handle, FIS_HOLLOW ); return;
  }
  size1 = tg->mfdb.fd_wdwidth << 1; size2 = size1 * tg->mfdb.fd_h;
  s = d; d.fd_addr = des; hg = tg;
  if (i < 0)
  {
    i = -i; size1 <<= i;
    (char *)s.fd_addr += size1 * lxy[1] + ((lxy[0] >> 3) << i);
    if (--i == 0)
    {
      char *phalf1, *phalf2; long c;
      phalf1 = half; phalf2 = phalf1 + 256; c = 0;
      do
      { tg = s.fd_addr; (char *)s.fd_addr += size2; i = d.fd_h;
	do
	{ clip = (int *)tg; (char *)tg += size1; k = d.fd_wdwidth;
	  do
	  {
#ifdef __TOS__
	    char e;
	    (char)c = *((char *)clip)++; e = phalf1[c];
	    (char)c = *((char *)clip)++; e |= phalf2[c]; *des++ = e;
	    (char)c = *((char *)clip)++; e = phalf1[c];
	    (char)c = *((char *)clip)++; e |= phalf2[c]; *des++ = e;
#else
	    int e;
	    (char)c = *((char *)clip)++; (char)e = phalf1[c];
	    (char)c = *((char *)clip)++; (char)e |= phalf2[c]; e <<= 8;
	    (char)c = *((char *)clip)++; (char)e = phalf1[c];
	    (char)c = *((char *)clip)++; (char)e |= phalf2[c];
	    *((int *)des)++ = e;
#endif
	  }
	  while (--k);
	}
	while (--i);
      }
      while (--d.fd_nplanes);
    }
    else if (--i == 0)
    {
      char *phalf1, *phalf2; long c, e;
      phalf1 = half; phalf2 = phalf1 + 256; e = c = 0;
      do
      { tg = s.fd_addr; (char *)s.fd_addr += size2; i = d.fd_h;
	do
	{ clip = (int *)tg; (char *)tg += size1; k = d.fd_wdwidth;
	  do
	  { (char)c = *((char *)clip)++; (char)e = phalf1[c];
	    (char)c = *((char *)clip)++; (char)e |= phalf2[c];
	    *des = phalf1[e];
	    (char)c = *((char *)clip)++; (char)e = phalf1[c];
	    (char)c = *((char *)clip)++; (char)e |= phalf2[c];
	    (char)e = phalf2[e]; *des++ |= e;
	    (char)c = *((char *)clip)++; (char)e = phalf1[c];
	    (char)c = *((char *)clip)++; (char)e |= phalf2[c];
	    *des = phalf1[e];
	    (char)c = *((char *)clip)++; (char)e = phalf1[c];
	    (char)c = *((char *)clip)++; (char)e |= phalf2[c];
	    (char)e = phalf2[e]; *des++ |= e;
	  }
	  while (--k);
	}
	while (--i);
      }
      while (--d.fd_nplanes);
#ifndef __TOS__
      flipwords( d.fd_addr, des - d.fd_addr );
#endif
    }
    else
      do
      { tg = s.fd_addr; (char *)s.fd_addr += size2; i = d.fd_h;
	do
	{ clip = (int *)tg; (char *)tg += size1; k = d.fd_wdwidth;
	  do
	  { int e = 0;
	    if (*((char *)clip)++ < 0) e += 1 << 15;
	    if (*((char *)clip)++ < 0) e += 1 << 14;
	    if (*((char *)clip)++ < 0) e += 1 << 13;
	    if (*((char *)clip)++ < 0) e += 1 << 12;
	    if (*((char *)clip)++ < 0) e += 1 << 11;
	    if (*((char *)clip)++ < 0) e += 1 << 10;
	    if (*((char *)clip)++ < 0) e += 1 <<  9;
	    if (*((char *)clip)++ < 0) e += 1 <<  8;
	    if (*((char *)clip)++ < 0) e += 1 <<  7;
	    if (*((char *)clip)++ < 0) e += 1 <<  6;
	    if (*((char *)clip)++ < 0) e += 1 <<  5;
	    if (*((char *)clip)++ < 0) e += 1 <<  4;
	    if (*((char *)clip)++ < 0) e += 1 <<  3;
	    if (*((char *)clip)++ < 0) e += 1 <<  2;
	    if (*((char *)clip)++ < 0) e += 1 <<  1;
	    if (*((char *)clip)++ < 0) e += 1;
	    *((int *)des)++ = e;
	  }
	  while (--k);
	}
	while (--i);
      }
      while (--d.fd_nplanes);
  }
  else if (i == 0)
  {
    (char *)s.fd_addr += size1 * lxy[1] + ((lxy[0] >> 4) << 1);
    do
    { tg = s.fd_addr; (char *)s.fd_addr += size2; i = d.fd_h;
      do
      { clip = (int *)tg; (char *)tg += size1; k = d.fd_wdwidth;
	do *((int *)des)++ = *clip++; while (--k);
      }
      while (--i);
    }
    while (--d.fd_nplanes);
#ifndef __TOS__
    flipwords( d.fd_addr, des - d.fd_addr );
#endif
  }
  else
  {
    (char *)s.fd_addr += size1 * (lxy[1] >> i) + ((lxy[0] >> 3) >> i);
    if (--i == 0)
    {
      int *pdoub = doub;
      do
      { tg = s.fd_addr; (char *)s.fd_addr += size2; i = d.fd_h;
	do
	{ unsigned char *src = (unsigned char *)tg; (char *)tg += size1;
	  k = d.fd_wdwidth; k <<= 1; (char *)clip = des + k;
	  do
	  { *(int *)des = pdoub[*src++]; *clip++ = *((int *)des)++;
	  }
	  while (k -= 2);
	  des = (char *)clip;
	}
	while (i -= 2);
      }
      while (--d.fd_nplanes);
    }
    else if (--i == 0)
    {
      long *pfour = four;
      do
      { tg = s.fd_addr; (char *)s.fd_addr += size2; i = d.fd_h;
	do
	{ unsigned char *src = (unsigned char *)tg; (char *)tg += size1;
	  clip = (int *)des; k = d.fd_wdwidth;
	  do *((long *)des)++ = pfour[*src++]; while (k -= 2);
	  k = d.fd_wdwidth;
	  do
	  { *((long *)des)++ = *((long *)clip)++;
	    *((long *)des)++ = *((long *)clip)++;
	    *((long *)des)++ = *((long *)clip)++;
	  }
	  while (k -= 2);
	}
	while (i -= 4);
      }
      while (--d.fd_nplanes);
    }
    else
    {
      char c = -1;
      do
      { tg = s.fd_addr; (char *)s.fd_addr += size2; i = d.fd_h;
	do
	{ char *src = (char *)tg; (char *)tg += size1;
	  clip = (int *)des; k = d.fd_wdwidth;
	  do
	  { char e; if ((e = *src++) < 0) *des++ = c; else *des++ = 0;
	    if ((e <<= 1) < 0) *des++ = c; else *des++ = 0;
	    if ((e <<= 1) < 0) *des++ = c; else *des++ = 0;
	    if ((e <<= 1) < 0) *des++ = c; else *des++ = 0;
	    if ((e <<= 1) < 0) *des++ = c; else *des++ = 0;
	    if ((e <<= 1) < 0) *des++ = c; else *des++ = 0;
	    if ((e <<= 1) < 0) *des++ = c; else *des++ = 0;
	    if ((e <<= 1) < 0) *des++ = c; else *des++ = 0;
	  }
	  while (k -= 4);
	  k = d.fd_wdwidth;
	  do
	  { *((long *)des)++ = *((long *)clip)++;
	    *((long *)des)++ = *((long *)clip)++;
	    *((long *)des)++ = *((long *)clip)++;
	    *((long *)des)++ = *((long *)clip)++;
	    *((long *)des)++ = *((long *)clip)++;
	    *((long *)des)++ = *((long *)clip)++;
	    *((long *)des)++ = *((long *)clip)++;
	  }
	  while (k -= 2);
	}
	while (i -= 8);
      }
      while (--d.fd_nplanes);
#ifndef __TOS__
      flipwords( d.fd_addr, des - d.fd_addr );
#endif
  } }
  tg = hg; d.fd_nplanes = s.fd_nplanes;
  s.fd_addr = des; des = d.fd_addr;
  s.fd_stand = 0; vr_trnfm( handle, &d, &s );
  d.fd_addr = 0;
  if (s.fd_nplanes == 1)
  {
    if (nplanes >= 16) switch_colors( tg );
    vrt_cpyfm( handle, MD_REPLACE, pxy, &s, &d, index );
    if (nplanes >= 16) switch_colors( tg );
  }
  else vro_cpyfm( handle, S_ONLY, pxy, &s, &d );
  Mfree( des );
}

static void change_scale( GUIDE *tg, int mx, int my, int delta )
{
  int dx, dy, d[3];

  if (mx < 0) { graf_mkstate( d + 1, d + 2, d, d ); mx = d[1]; my = d[2]; }
  dx = tg->width; dy = tg->height;
  switch (tg->scale += delta)
  {
    case -3: ++dx; dx >>= 1; ++dy; dy >>= 1;
    case -2: ++dx; dx >>= 1; ++dy; dy >>= 1;
    case -1: ++dx; dx >>= 1; ++dy; dy >>= 1;
    case  0: ++dx; dx >>= 1; ++dy; dy >>= 1;
    case  1: ++dx; dx >>= 1; ++dy; dy >>= 1;
    case  2: ++dx; dx >>= 1; ++dy; dy >>= 1;
  }
  dx -= tg->w >> 3; tg->bc = dx; dy -= tg->h >> 3; tg->lc = dy;
  dx = tg->lbc; mx -= tg->x; mx += 3; mx >>= 3; dx += mx;
  dy = tg->tlc; my -= tg->y; my += 3; my >>= 3; dy += my;
  if (delta > 0) { dx <<= 1; dy <<= 1; } else { dx >>= 1; dy >>= 1; }
  dx -= mx; if (tg->bc < dx) dx = tg->bc; if (dx < 0) dx = 0;
  dy -= my; if (tg->lc < dy) dy = tg->lc; if (dy < 0) dy = 0;
  tg->lbc = dx; tg->tlc = dy; set_allslider(); full_redraw();
}

#pragma warn -par
int key_image( GUIDE *tg, int code, int ks )
{
  switch (code)
  {
    case CNTRL_CL: if (tg->scale >= -2) change_scale(tg,-1,0,-1); return 0;
    case CNTRL_CR: if (tg->scale <=  2) change_scale(tg,-1,0, 1); return 0;
    case CNTRL_E: if ((tg->x_flag & 4) == 0) switch_colors( tg ); return 0;
    case CNTRL_S: if (tg->x_flag & 4) switch_colors( tg ); return 0;
    case CNTRL_X: if ((tg->x_flag & 8) == 0) save_palette( tg ); return 0;
  }
  return 1;
}
#pragma warn +par

void sclick_image( GUIDE *tg, int mx, int my )
{
  popup[9].ob_state = tg->x_flag & 8 ? DISABLED : NORMAL;
  popup[5].ob_state = tg->x_flag & 1 ? NORMAL : DISABLED;
  popup[5].ob_spec.free_string = tg->x_flag & 4 ? sstring : ostring;
  popup[4].ob_state = tg->scale <=  2 ? NORMAL : DISABLED;
  popup[2].ob_state = tg->scale >= -2 ? NORMAL : DISABLED;
  {
    char v = ' ', z = '0', *p = string2;
    if (tg->scale < 0) { v = '-'; z -= tg->scale; }
    if (tg->scale > 0) { v = '+'; z += tg->scale; }
    *p++ = v; *p++ = z; *p++ = v;
  }
  itostring( tg->width, string3 + 5, 6 );
  itostring( tg->height, string3 + 12, 6 );
  itostring( tg->planes, string4 + 7, 3 );
  strncpy( string5 + 10, tg->x_flag & 2 ? "CMAP" : "XIMG", 4 );
  switch (popup_menu( popup, 3, mx, my, objc_draw ))
  {
    case 2: change_scale( tg, mx, my, -1 ); break;
    case 4: change_scale( tg, mx, my,  1 ); break;
    case 5: switch_colors( tg ); break;
    case 9: save_palette( tg );
} }

void dummy_image() { }

FROM( jpegddrv ) IMPORT char *fs_init( int ditherflag,
					long fs_data, long fs_width );
FROM( jpegddrv ) IMPORT int fs_dither( int n_comps,int n_rows,int n_cols,
					char *fs, char *ptr );

#define FTESTC(fp) if (--(fp)->bytes_left < 0) Fbufread( fp );
#define FFETCHC(fp) *(fp)->pbuf++

typedef struct ibuf
{
  char *pbuf;
  long bytes_left;
  void (*putline)(struct ibuf *ip);
  int  line;
  char vrc, dummy;
  char *line_adr;
  long length;
  long linelen;
  char *dith_ptr;
  int  width, height;
  int  srcplanes, desplanes;
  void *raster_ptr;
  long planesize;
  char pat_buf[4];
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

void goto_finish( void )
{
  longjmp( setjmp_buffer, 2 );
}

static void put_m6m5( IBUF *ip )
{
  char cdata, *line_ptr, *line_buf, *endl_buf;
  long i, linelen, *raster_ptr;
  int pix, idata;

  raster_ptr = ip->raster_ptr; endl_buf = ip->pbuf;
  linelen = ip->linelen;
  do
  { i = linelen; line_buf = ip->line_adr;
    do
    { cdata = 0x80;
      do
      { pix = 0; idata = 4; line_ptr = line_buf;
	do
	{ if (*line_ptr & cdata) pix |= idata;
	  idata <<= 1;
	}
	while ((line_ptr += linelen) < endl_buf);
	*raster_ptr++ = *(long *)(endl_buf + pix);
      }
      while ((unsigned char)cdata >>= 1);
      line_buf++;
    }
    while (--i);
    if (--ip->height <= 0) goto_finish();
    (char *)raster_ptr += (linelen & 1L) << 5;
  }
  while (--ip->vrc);
  ip->vrc++; ip->raster_ptr = raster_ptr;
  ip->pbuf = ip->line_adr; ip->bytes_left = ip->length;
}

static void put_m4m3( IBUF *ip )
{
  char cdata, *line_ptr, *line_buf, *endl_buf;
  char *raster_ptr;
  long i, linelen;
  int pix, idata;

  raster_ptr = ip->raster_ptr; endl_buf = ip->pbuf;
  linelen = ip->linelen;
  do
  { i = linelen; line_buf = ip->line_adr;
    do
    { cdata = 0x80;
      do
      { pix = 0; idata = 4; line_ptr = line_buf;
	do
	{ if (*line_ptr & cdata) pix |= idata;
	  idata <<= 1;
	}
	while ((line_ptr += linelen) < endl_buf);
	line_ptr = endl_buf + pix;
	*raster_ptr++ = *line_ptr++;
	*raster_ptr++ = *line_ptr++;
	*raster_ptr++ = *line_ptr++;
      }
      while ((unsigned char)cdata >>= 1);
      line_buf++;
    }
    while (--i);
    if (--ip->height <= 0) goto_finish();
    i = 1L; i &= linelen; i <<= 3;
    raster_ptr += i; i <<= 1;
    raster_ptr += i;
  }
  while (--ip->vrc);
  ip->vrc++; ip->raster_ptr = raster_ptr;
  ip->pbuf = ip->line_adr; ip->bytes_left = ip->length;
}

static void put_m2m1( IBUF *ip )
{
  char cdata, *line_ptr, *line_buf, *endl_buf;
  int pix, idata, *raster_ptr;
  long i, linelen;

  raster_ptr = ip->raster_ptr; endl_buf = ip->pbuf;
  linelen = ip->linelen;
  do
  { i = linelen; line_buf = ip->line_adr;
    do
    { cdata = 0x80;
      do
      { pix = 0; idata = 2; line_ptr = line_buf;
	do
	{ if (*line_ptr & cdata) pix |= idata;
	  idata <<= 1;
	}
	while ((line_ptr += linelen) < endl_buf);
	*raster_ptr++ = *(int *)(endl_buf + pix);
      }
      while ((unsigned char)cdata >>= 1);
      line_buf++;
    }
    while (--i);
    if (--ip->height <= 0) goto_finish();
    (char *)raster_ptr += (linelen & 1L) << 4;
  }
  while (--ip->vrc);
  ip->vrc++; ip->raster_ptr = raster_ptr;
  ip->pbuf = ip->line_adr; ip->bytes_left = ip->length;
}

static void put_0( IBUF *ip )
{
  char *raster_ptr, *line_ptr, *line_buf, *endl_buf;
  int mask, idata, srcplanes, desplanes;
  long i, planesize;

  raster_ptr = ip->raster_ptr; endl_buf = ip->pbuf;
  srcplanes = ip->srcplanes; desplanes = ip->desplanes;
  planesize = ip->planesize;
  do
  { i = ip->linelen; line_buf = ip->line_adr;
    loop: line_ptr = raster_ptr; idata = desplanes;
    for (;;)
    {
      if (i & 1L)
	do *line_ptr++ |= *line_buf++; while (--i);
      else
	do *((int *)line_ptr)++ |= *((int *)line_buf)++;
	while (i -= 2);
      i = ip->linelen;
      if (line_buf >= endl_buf) break;
      if (--idata <= 0) goto loop;
      line_ptr -= i; line_ptr += planesize;
    }
    i += i & 1L;
    if (srcplanes >= desplanes) raster_ptr += i;
    else
      do
      { idata = 1;
	line_ptr = raster_ptr; mask = *((int *)raster_ptr)++;
	do
	{ line_ptr += planesize;
	  if (idata < srcplanes) mask &= *(int *)line_ptr;
	  else *(int *)line_ptr = mask;
	}
	while (++idata < desplanes);
      }
      while (i -= 2);
    if (--ip->height <= 0) goto_finish();
  }
  while (--ip->vrc);
  ip->vrc++; ip->raster_ptr = raster_ptr;
  ip->pbuf = ip->line_adr; ip->bytes_left = ip->length;
}

#ifdef IMAGEOPT

void put_1( IBUF *ip );
void put_2( IBUF *ip );
void put_3( IBUF *ip );
void put_4( IBUF *ip );

#else

static void put_1( IBUF *ip )
{
  char cdata, *dith_ptr, *line_ptr, *line_buf, *endl_buf;
  char *raster_ptr;
  long i, linelen;
  int pix, idata;

  raster_ptr = ip->raster_ptr; dith_ptr = ip->dith_ptr;
  endl_buf = ip->pbuf; linelen = ip->linelen;
  do
  { i = linelen; line_buf = ip->line_adr;
    do
    { cdata = 0x80;
      do
      { pix = 0; idata = 1; line_ptr = line_buf;
	do
	{ if (*line_ptr & cdata) pix |= idata;
	  idata <<= 1;
	}
	while ((line_ptr += linelen) < endl_buf);
	if (endl_buf[pix] <= *dith_ptr++) *raster_ptr |= cdata;
      }
      while ((unsigned char)cdata >>= 1);
      dith_ptr -= 8; raster_ptr++; line_buf++;
    }
    while (--i);
    if (--ip->height <= 0) goto_finish();
    raster_ptr += linelen & 1L; dith_ptr += 8;
    if (dith_ptr == dith64matrix + 64) dith_ptr -= 64;
  }
  while (--ip->vrc);
  ip->vrc++;
  ip->dith_ptr = dith_ptr; ip->raster_ptr = raster_ptr;
  ip->pbuf = ip->line_adr; ip->bytes_left = ip->length;
}

static void put_2( IBUF *ip )
{
  char cdata, *dith_ptr, *line_ptr, *line_buf, *endl_buf;
  long i, linelen, planesize;
  char *raster_ptr;
  int pix, idata;

  raster_ptr = ip->raster_ptr; dith_ptr = ip->dith_ptr;
  endl_buf = ip->pbuf; linelen = ip->linelen;
  planesize = ip->planesize;
  do
  { i = linelen; line_buf = ip->line_adr;
    do
    { cdata = 0x80;
      do
      { pix = 0; idata = 4; line_ptr = line_buf;
	do
	{ if (*line_ptr & cdata) pix |= idata;
	  idata <<= 1;
	}
	while ((line_ptr += linelen) < endl_buf);
	line_ptr = endl_buf + pix;
	(char)idata = *dith_ptr++; pix = 0;
	if (*line_ptr++ > (char)idata) pix += 4; /* Rot  */
	if (*line_ptr++ > (char)idata) pix += 2; /* GrÅn */
	if (*line_ptr++ > (char)idata) pix += 1; /* Blau */
	if (pix -= 7)			/* nicht Weiss */
	{
	  line_ptr = raster_ptr;
	  if ((pix += 7) == 0)		/* Schwarz */
	  {
	    idata = ip->desplanes;
	    do { *line_ptr |= cdata; line_ptr += planesize; }
	    while (--idata > 0);
	  }
	  else
	  {
	    if (((char)pix <<= 5) < 0) *line_ptr |= cdata;
	    line_ptr += planesize;
	    if (((char)pix <<= 1) < 0) *line_ptr |= cdata;
	    if (((char)pix <<= 1) < 0)
	      line_ptr[planesize] |= cdata;
      } } }
      while ((unsigned char)cdata >>= 1);
      dith_ptr -= 8; raster_ptr++; line_buf++;
    }
    while (--i);
    if (--ip->height <= 0) goto_finish();
    raster_ptr += linelen & 1L; dith_ptr += 8;
    if (dith_ptr == dith64matrix + 64) dith_ptr -= 64;
  }
  while (--ip->vrc);
  ip->vrc++;
  ip->dith_ptr = dith_ptr; ip->raster_ptr = raster_ptr;
  ip->pbuf = ip->line_adr; ip->bytes_left = ip->length;
}

static void put_3( IBUF *ip )
{
  char cdata, *dith_ptr, *line_ptr, *line_buf, *endl_buf;
  long i, linelen, planesize;
  char *raster_ptr;
  int pix, idata;

  raster_ptr = ip->raster_ptr; dith_ptr = ip->dith_ptr;
  endl_buf = ip->pbuf; linelen = ip->linelen;
  planesize = ip->planesize;
  do
  { i = linelen; line_buf = ip->line_adr;
    do
    { cdata = 0x80;
      do
      { static const char idx[] =
	  { 32,32+43,32+2*43,32+3*43,32+4*43,32+5*43 };
	pix = 0; idata = 1; line_ptr = line_buf;
	do
	{ if (*line_ptr & cdata) pix |= idata;
	  idata <<= 1;
	}
	while ((line_ptr += linelen) < endl_buf);
	raster_ptr += planesize << 3; idata = 0;
	(char)idata = endl_buf[pix]; idata += idata << 2;
	if ((char)(idata & 63) > *dith_ptr++) idata += 64;
	idata >>= 6;
	raster_ptr -= planesize;
	if (((char)pix = idx[idata]) < 0) *raster_ptr |= cdata;
	raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= cdata;
	raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= cdata;
	raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= cdata;
	raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= cdata;
	raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= cdata;
	raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= cdata;
	raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= cdata;
      }
      while ((unsigned char)cdata >>= 1);
      dith_ptr -= 8; raster_ptr++; line_buf++;
    }
    while (--i);
    if (--ip->height <= 0) goto_finish();
    raster_ptr += linelen & 1L; dith_ptr += 8;
    if (dith_ptr == dith64matrix + 64) dith_ptr -= 64;
  }
  while (--ip->vrc);
  ip->vrc++;
  ip->dith_ptr = dith_ptr; ip->raster_ptr = raster_ptr;
  ip->pbuf = ip->line_adr; ip->bytes_left = ip->length;
}

static void put_4( IBUF *ip )
{
  char cdata, *dith_ptr, *line_ptr, *line_buf, *endl_buf;
  long i, linelen, planesize;
  char *raster_ptr;
  int pix, idata;

  raster_ptr = ip->raster_ptr; dith_ptr = ip->dith_ptr;
  endl_buf = ip->pbuf; linelen = ip->linelen;
  planesize = ip->planesize;
  do
  { i = linelen; line_buf = ip->line_adr;
    do
    { cdata = 0x80;
      do
      { pix = 0; idata = 4; line_ptr = line_buf;
	do
	{ if (*line_ptr & cdata) pix |= idata;
	  idata <<= 1;
	}
	while ((line_ptr += linelen) < endl_buf);
	line_ptr = endl_buf + pix; pix = 0; idata = 0;
	(char)idata = *line_ptr++; idata += idata << 2;
	if ((char)(idata & 63) > *dith_ptr) ++pix;
	idata >>= 6; pix += idata;
	pix += pix << 1; pix <<= 1;
	(char)idata = *line_ptr++; idata += idata << 2;
	if ((char)(idata & 63) > *dith_ptr) ++pix;
	idata >>= 6; pix += idata;
	pix += pix << 1; pix <<= 1;
	(char)idata = *line_ptr++; idata += idata << 2;
	if ((char)(idata & 63) > *dith_ptr++) ++pix;
	idata >>= 6; pix += idata;
	raster_ptr += planesize << 3;
	raster_ptr -= planesize;
	if (((char)pix += 32) < 0) *raster_ptr |= cdata;
	raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= cdata;
	raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= cdata;
	raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= cdata;
	raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= cdata;
	raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= cdata;
	raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= cdata;
	raster_ptr -= planesize;
	if (((char)pix <<= 1) < 0) *raster_ptr |= cdata;
      }
      while ((unsigned char)cdata >>= 1);
      dith_ptr -= 8; raster_ptr++; line_buf++;
    }
    while (--i);
    if (--ip->height <= 0) goto_finish();
    raster_ptr += linelen & 1L; dith_ptr += 8;
    if (dith_ptr == dith64matrix + 64) dith_ptr -= 64;
  }
  while (--ip->vrc);
  ip->vrc++;
  ip->dith_ptr = dith_ptr; ip->raster_ptr = raster_ptr;
  ip->pbuf = ip->line_adr; ip->bytes_left = ip->length;
}

#endif

static void put_5( IBUF *ip )
{
  char cdata, *dith_ptr, *line_ptr, *line_buf, *endl_buf;
  long i, linelen;
  int pix, idata;

  endl_buf = ip->pbuf; linelen = ip->linelen;
  do
  { dith_ptr = ip->dith_ptr;
    i = linelen; line_buf = ip->line_adr;
    do
    { cdata = 0x80;
      do
      { pix = 0; idata = 1; line_ptr = line_buf;
	do
	{ if (*line_ptr & cdata) pix |= idata;
	  idata <<= 1;
	}
	while ((line_ptr += linelen) < endl_buf);
	*dith_ptr++ = endl_buf[pix];
      }
      while ((unsigned char)cdata >>= 1);
      line_buf++;
    }
    while (--i);
    ip->line =
      fs_dither( ip->line, 1, ip->width, dith_ptr, ip->dith_ptr );
    dith_ptr = ip->dith_ptr;
    pix = ip->width; line_buf = ip->raster_ptr;
    for (;;)
    {
      idata = 0x8000;
      do
      { if (*dith_ptr++ == 0) *(int *)line_buf |= idata;
	if (--pix <= 0) goto nextline;
      }
      while ((unsigned)idata >>= 1);
      line_buf += 2;
    }
    nextline: line_buf += 2;
    ip->raster_ptr = line_buf;
    if (--ip->height <= 0) goto_finish();
  }
  while (--ip->vrc);
  ip->vrc++;
  ip->pbuf = ip->line_adr; ip->bytes_left = ip->length;
}

static void put_6( IBUF *ip )
{
  char cdata, *dith_ptr, *line_ptr, *line_buf, *endl_buf;
  long i, linelen, planesize;
  int pix, idata;

  endl_buf = ip->pbuf;
  linelen = ip->linelen; planesize = ip->planesize;
  do
  { dith_ptr = ip->dith_ptr;
    i = linelen; line_buf = ip->line_adr;
    do
    { cdata = 0x80;
      do
      { pix = 0; idata = 4; line_ptr = line_buf;
	do
	{ if (*line_ptr & cdata) pix |= idata;
	  idata <<= 1;
	}
	while ((line_ptr += linelen) < endl_buf);
	line_ptr = endl_buf + pix;
	*dith_ptr++ = *line_ptr++;
	*dith_ptr++ = *line_ptr++;
	*dith_ptr++ = *line_ptr++;
      }
      while ((unsigned char)cdata >>= 1);
      line_buf++;
    }
    while (--i);
    ip->line =
      fs_dither( ip->line, 1, ip->width, dith_ptr, ip->dith_ptr );
    dith_ptr = ip->dith_ptr;
    pix = ip->width; line_buf = ip->raster_ptr;
    for (;;)
    {
      idata = 0x8000;
      do
      {   cdata  = *dith_ptr++;
	  cdata += *dith_ptr++;
	  cdata += *dith_ptr++;
	if (cdata -= 7)				/* nicht Weiss */
	{
	  line_ptr = line_buf;
	  if ((cdata += 7) == 0)		/* Schwarz */
	  {
	    cdata = ip->desplanes;
	    do
	    { *(int *)line_ptr |= idata;
	      line_ptr += planesize;
	    }
	    while (--cdata > 0);
	  }
	  else
	  {
	    if ((cdata <<= 5) < 0) *(int *)line_ptr |= idata;
	    line_ptr += planesize;
	    if ((cdata <<= 1) < 0) *(int *)line_ptr |= idata;
	    if ((cdata <<= 1) < 0)
	      *(int *)(line_ptr + planesize) |= idata;
	} }
	if (--pix <= 0) goto nextline;
      }
      while ((unsigned)idata >>= 1);
      line_buf += 2;
    }
    nextline: line_buf += 2;
    ip->raster_ptr = line_buf;
    if (--ip->height <= 0) goto_finish();
  }
  while (--ip->vrc);
  ip->vrc++;
  ip->pbuf = ip->line_adr; ip->bytes_left = ip->length;
}

static void put_7( IBUF *ip )
{
  char cdata, *dith_ptr, *line_ptr, *line_buf, *endl_buf;
  long i, linelen, planesize;
  int pix, idata;

  endl_buf = ip->pbuf;
  linelen = ip->linelen; planesize = ip->planesize;
  do
  { dith_ptr = ip->dith_ptr;
    i = linelen; line_buf = ip->line_adr;
    do
    { cdata = 0x80;
      do
      { pix = 0; idata = 1; line_ptr = line_buf;
	do
	{ if (*line_ptr & cdata) pix |= idata;
	  idata <<= 1;
	}
	while ((line_ptr += linelen) < endl_buf);
	*dith_ptr++ = endl_buf[pix];
      }
      while ((unsigned char)cdata >>= 1);
      line_buf++;
    }
    while (--i);
    ip->line =
      fs_dither( ip->line, 1, ip->width, dith_ptr, ip->dith_ptr );
    dith_ptr = ip->dith_ptr;
    pix = ip->width; line_buf = ip->raster_ptr;
    for (;;)
    {
      idata = 0x8000;
      do
      { line_buf += planesize << 3;
	cdata = 32;
	line_buf -= planesize;
	if ((cdata += *dith_ptr++) < 0)
	  *(int *)line_buf |= idata;
	line_buf -= planesize;
	if ((cdata <<= 1) < 0) *(int *)line_buf |= idata;
	line_buf -= planesize;
	if ((cdata <<= 1) < 0) *(int *)line_buf |= idata;
	line_buf -= planesize;
	if ((cdata <<= 1) < 0) *(int *)line_buf |= idata;
	line_buf -= planesize;
	if ((cdata <<= 1) < 0) *(int *)line_buf |= idata;
	line_buf -= planesize;
	if ((cdata <<= 1) < 0) *(int *)line_buf |= idata;
	line_buf -= planesize;
	if ((cdata <<= 1) < 0) *(int *)line_buf |= idata;
	line_buf -= planesize;
	if ((cdata <<= 1) < 0) *(int *)line_buf |= idata;
	if (--pix <= 0) goto nextline;
      }
      while ((unsigned)idata >>= 1);
      line_buf += 2;
    }
    nextline: line_buf += 2;
    ip->raster_ptr = line_buf;
    if (--ip->height <= 0) goto_finish();
  }
  while (--ip->vrc);
  ip->vrc++;
  ip->pbuf = ip->line_adr; ip->bytes_left = ip->length;
}

static void put_8( IBUF *ip )
{
  char cdata, *dith_ptr, *line_ptr, *line_buf, *endl_buf;
  long i, linelen, planesize;
  int pix, idata;

  endl_buf = ip->pbuf;
  linelen = ip->linelen; planesize = ip->planesize;
  do
  { dith_ptr = ip->dith_ptr;
    i = linelen; line_buf = ip->line_adr;
    do
    { cdata = 0x80;
      do
      { pix = 0; idata = 4; line_ptr = line_buf;
	do
	{ if (*line_ptr & cdata) pix |= idata;
	  idata <<= 1;
	}
	while ((line_ptr += linelen) < endl_buf);
	line_ptr = endl_buf + pix;
	*dith_ptr++ = *line_ptr++;
	*dith_ptr++ = *line_ptr++;
	*dith_ptr++ = *line_ptr++;
      }
      while ((unsigned char)cdata >>= 1);
      line_buf++;
    }
    while (--i);
    ip->line =
      fs_dither( ip->line, 1, ip->width, dith_ptr, ip->dith_ptr );
    dith_ptr = ip->dith_ptr;
    pix = ip->width; line_buf = ip->raster_ptr;
    for (;;)
    {
      idata = 0x8000;
      do
      { line_buf += planesize << 3;
	cdata = 32; cdata += *dith_ptr++; cdata += *dith_ptr++;
	line_buf -= planesize;
	if ((cdata += *dith_ptr++) < 0)
	  *(int *)line_buf |= idata;
	line_buf -= planesize;
	if ((cdata <<= 1) < 0) *(int *)line_buf |= idata;
	line_buf -= planesize;
	if ((cdata <<= 1) < 0) *(int *)line_buf |= idata;
	line_buf -= planesize;
	if ((cdata <<= 1) < 0) *(int *)line_buf |= idata;
	line_buf -= planesize;
	if ((cdata <<= 1) < 0) *(int *)line_buf |= idata;
	line_buf -= planesize;
	if ((cdata <<= 1) < 0) *(int *)line_buf |= idata;
	line_buf -= planesize;
	if ((cdata <<= 1) < 0) *(int *)line_buf |= idata;
	line_buf -= planesize;
	if ((cdata <<= 1) < 0) *(int *)line_buf |= idata;
	if (--pix <= 0) goto nextline;
      }
      while ((unsigned)idata >>= 1);
      line_buf += 2;
    }
    nextline: line_buf += 2;
    ip->raster_ptr = line_buf;
    if (--ip->height <= 0) goto_finish();
  }
  while (--ip->vrc);
  ip->vrc++;
  ip->pbuf = ip->line_adr; ip->bytes_left = ip->length;
}

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

static GUIDE *load_image( int fh, TIMG_HEADER *th, long len )
{
  GUIDE *rg;
  IBUF  *ip;
  char  cdata, *endl_buf, *line_ptr;
  int	idata, i, pal, ditherflag, patrun;
  long	linelen, planesize, rastsize;
  CHUNK_HEADER ch;
  struct { int fd_w;
	   int fd_h;
	   int fd_wdwidth;
	   int fd_nplanes;
	 } s;
  FBUF  f;

  f.handle = fh; f.buf_size = len;
  i = 0;
  if (~th->ih.headlen)	/* IMG */
  {
    ch.length = (long)th->ih.headlen << 1;
    th->ih.headlen = 0;
    if (th->magic == X_MAGIC) i = 1;
  }
  else			/* IFF */
    for (;;)
    {
      if (Fread( f.handle, 8, &ch ) - 8)
      {
	Fclose( f.handle ); form_error( ENOENT ); return 0;
      }
#ifndef __TOS__
      fliplongs( (int *)&ch.length, 1, 1 );
#endif
      f.buf_size -= 8; ch.length += ch.length & 1L;
      if (ch.magic == BO_MAGIC) break;
      if (ch.magic == CM_MAGIC)	{ i = 1; break; }
      Fseek( ch.length, f.handle, 1 ); f.buf_size -= ch.length;
    }
  pal = i;
  s.fd_wdwidth = (th->ih.sl_width + 15) >> 4;
  s.fd_w = (th->ih.sl_width + 7) & -8;
  s.fd_h = (th->ih.sl_height + 7) & -8;
  idata = nplanes; s.fd_nplanes = th->ih.planes > 1 ? idata : 1;
  if ((i = 1 - (s.fd_nplanes >> 3)) < 0)
  {
    i <<= 1; i += par.dithmod;
  }
  else if ((i = par.usedith) != 0)
    if (--i && pal && idata < 16 || idata < th->ih.planes)
    {
      i = 0;
      if (idata >= 3)
      {
	if (idata >= 8) i = par.dithcol << 1;
	i += par.dithmod;
      }
      if (i == 0) idata = 1;
      s.fd_nplanes = idata; i++;
    }
    else i = 0;
  planesize = 2L * s.fd_wdwidth * s.fd_h; idata = 1 << th->ih.planes;
  rastsize = sizeof(GUIDE) + 2 + idata * 6;
  rastsize += planesize * s.fd_nplanes;
  if ((rg = Malloc( rastsize )) == 0)
    { Fclose( f.handle ); form_error( EINVMEM ); return 0; }
  line_ptr = (char *)rg->rgb_list;
  memset( rg, 0, rastsize );
  if ((ditherflag = i) != 0)
  {
    int k = i; i = idata;
    if (k != 1 && k != 3)
    {
      i <<= 1; if (k != -2 && k != -1) i <<= 1;
  } }
  if (th->ih.headlen == 0) linelen = s.fd_w >> 3;
  else linelen = s.fd_wdwidth << 1;
  rastsize = linelen * th->ih.planes;
  if (th->ih.headlen == 0) rastsize += th->ih.pat_run;
  if ((ip = Malloc( sizeof(IBUF) + rastsize + i )) == 0)
    { Mfree( rg ); Fclose( f.handle ); form_error( EINVMEM ); return 0; }
  ip->raster_ptr = line_ptr + idata * 6 + 2;
  endl_buf = ip->pat_buf + 4;
  endl_buf -= rastsize & 3;
  if (th->ih.headlen == 0)
  {
    endl_buf += th->ih.pat_run;
    rastsize -= th->ih.pat_run;
  }
  ip->line_adr = endl_buf; endl_buf += rastsize;
  ip->linelen = linelen;
  ip->planesize = planesize;
  ip->length = rastsize;
  memset( endl_buf, 0, i );
  setconv();
  if (ditherflag > 0)
    if ((ip->dith_ptr = fs_init( ditherflag, linelen << 3,
				 th->ih.sl_width )) != 0)
    {
      ip->line = (ditherflag & 1) ? 1 : 3;
      ditherflag += 4;
    }
    else ip->dith_ptr = dith64matrix;
  Fbufopen( &f ); idata--; i = 0;
  if (setjmp( setjmp_buffer )) goto finish;
  if (pal)
  {
    if (th->ih.headlen == 0)
    {
      ch.length -= 2;
#ifdef __TOS__
      FGETC(&f, line_ptr[0]);
      FGETC(&f, line_ptr[1]);
#else
      FGETC(&f, line_ptr[1]);
      FGETC(&f, line_ptr[0]);
#endif
    }
    ip->pbuf = endl_buf;
    do
    { line_ptr = (char *)rg->rgb_list + 6 * pli2vdi( i, idata ) + 2;
      if (th->ih.headlen == 0)
      {
	ch.length -= 6;
#ifdef __TOS__
	FGETC(&f, *line_ptr++);
	FGETC(&f, *line_ptr++);
	FGETC(&f, *line_ptr++);
	FGETC(&f, *line_ptr++);
	FGETC(&f, *line_ptr++);
	FGETC(&f, *line_ptr++);
	line_ptr -= 6;
#else
	FGETC(&f, line_ptr[1]);
	FGETC(&f, line_ptr[0]);
	FGETC(&f, line_ptr[3]);
	FGETC(&f, line_ptr[2]);
	FGETC(&f, line_ptr[5]);
	FGETC(&f, line_ptr[4]);
#endif
      }
      else
      {
	ch.length -= 3;
	FTESTC(&f);
	*((unsigned *)line_ptr)++ = (FFETCHC(&f) * 200U) / 51U;
	FTESTC(&f);
	*((unsigned *)line_ptr)++ = (FFETCHC(&f) * 200U) / 51U;
	FTESTC(&f);
	*((unsigned *)line_ptr)++ = (FFETCHC(&f) * 200U) / 51U;
	line_ptr -= 6;
      }
      switch (ditherflag)
      {
	case -6:
	case -4:
	  get_gray( endl_buf + (vdi2pli( i, idata ) << 2),
		    (int *)line_ptr );
	  break;
	case -5:
	case -3:
	  get_true( endl_buf + (vdi2pli( i, idata ) << 2),
		    (int *)line_ptr );
	  break;
	case -2:
	  get_gray( endl_buf + (vdi2pli( i, idata ) << 1),
		    (int *)line_ptr );
	  break;
	case -1:
	  get_true( endl_buf + (vdi2pli( i, idata ) << 1),
		    (int *)line_ptr );
	case 0:
	  break;
	case 1:
	case 3:
	  endl_buf[vdi2pli( i, idata )] = gray64( (int *)line_ptr );
	  break;
	case 2:
	case 4:
	  endl_buf = ip->pbuf + (vdi2pli( i, idata ) << 2);
	  *endl_buf++ = ((*((unsigned *)line_ptr)++ << 4) + 125U) / 250U;
	  *endl_buf++ = ((*((unsigned *)line_ptr)++ << 4) + 125U) / 250U;
	  *endl_buf++ = ((*((unsigned *)line_ptr)++ << 4) + 125U) / 250U;
	  break;
	case 5:
	case 7:
	  endl_buf[vdi2pli( i, idata )] = gray255( (int *)line_ptr );
	  break;
	case 6:
	case 8:
	  endl_buf = ip->pbuf + (vdi2pli( i, idata ) << 2);
	  *endl_buf++ = (*((unsigned *)line_ptr)++ * 51U + 100U) / 200U;
	  *endl_buf++ = (*((unsigned *)line_ptr)++ * 51U + 100U) / 200U;
	  *endl_buf++ = (*((unsigned *)line_ptr)++ * 51U + 100U) / 200U;
    } }
    while (ch.length > 0 && ++i <= idata);
    if (th->ih.headlen)
      do
      { while (--ch.length >= 0) FSKIPC(&f);
	line_ptr = (char *)&ch;
	FGETC(&f, *line_ptr++);
	FGETC(&f, *line_ptr++);
	FGETC(&f, *line_ptr++);
	FGETC(&f, *line_ptr++);
	FGETC(&f, *line_ptr++);
	FGETC(&f, *line_ptr++);
	FGETC(&f, *line_ptr++);
	FGETC(&f, *line_ptr++);
#ifndef __TOS__
	fliplongs( (int *)&ch.length, 1, 1 );
#endif
	ch.length += ch.length & 1L;
      }
      while (ch.magic != BO_MAGIC);
  }
  else
    switch (ditherflag)
    {
      case -6:
      case -4:
	do { get_gray( endl_buf, defcols[i & 15] ); endl_buf += 4; }
	while (++i < idata);
	get_gray( endl_buf, defcols[15] );
	break;
      case -5:
      case -3:
	do { get_true( endl_buf, defcols[i & 15] ); endl_buf += 4; }
	while (++i < idata);
	get_true( endl_buf, defcols[15] );
	break;
      case -2:
	do { get_gray( endl_buf, defcols[i & 15] ); endl_buf += 2; }
	while (++i < idata);
	get_gray( endl_buf, defcols[15] );
	break;
      case -1:
	do { get_true( endl_buf, defcols[i & 15] ); endl_buf += 2; }
	while (++i < idata);
	get_true( endl_buf, defcols[15] );
      case 0:
	break;
      case 1:
      case 3:
	do *endl_buf++ = gray64( defcols[i & 15] );
	while (++i < idata);
	break;
      case 2:
      case 4:
	do
	{ (int *)line_ptr = defcols[i & 15];
	  *endl_buf++ = ((*((unsigned *)line_ptr)++ << 4) + 125U) / 250U;
	  *endl_buf++ = ((*((unsigned *)line_ptr)++ << 4) + 125U) / 250U;
	  *endl_buf++ = ((*((unsigned *)line_ptr)++ << 4) + 125U) / 250U;
	  endl_buf++;
	}
	while (++i < idata);
	break;
      case 5:
      case 7:
	do *endl_buf++ = gray255( defcols[i & 15] );
	while (++i < idata);
	break;
      case 6:
      case 8:
	do
	{ (int *)line_ptr = defcols[i & 15];
	  *endl_buf++ = (*((unsigned *)line_ptr)++ * 51U + 100U) / 200U;
	  *endl_buf++ = (*((unsigned *)line_ptr)++ * 51U + 100U) / 200U;
	  *endl_buf++ = (*((unsigned *)line_ptr)++ * 51U + 100U) / 200U;
	  endl_buf++;
	}
	while (++i < idata);
    }
  i = pal;
  if (th->ih.headlen) i += 2; else while (--ch.length >= 0) FSKIPC(&f);
  if (ditherflag < 0)
  {
    vs_color( handle, 1, defcols[0] );
    v_pmarker( handle, 1, defcols[15] );
    vs_color( handle, 1, defcols[15] );
  }  
  else ++rg->mfdb.fd_stand;
  rg->planes = th->ih.planes;
  rg->width = th->ih.sl_width;
  rg->height = th->ih.sl_height;
  rg->x_flag = i;
  rg->mfdb.fd_addr = ip->raster_ptr;
  rg->mfdb.fd_nplanes = s.fd_nplanes;
  rg->mfdb.fd_wdwidth = s.fd_wdwidth;
  rg->w = rg->mfdb.fd_w = s.fd_w; rg->bc = rg->w >> 3;
  rg->h = rg->mfdb.fd_h = s.fd_h; rg->lc = rg->h >> 3;
  rg->xfac += 8; rg->yfac += 8; rg->draw = draw_image;
  rg->key = key_image; rg->sclick = sclick_image;
  rg->dclick = dummy_image; rg->free = dummy_image;
  ip->vrc = 1;
  ip->width = th->ih.sl_width; ip->height = th->ih.sl_height;
  ip->srcplanes = th->ih.planes; ip->desplanes = s.fd_nplanes;
  ip->pbuf = ip->line_adr; ip->bytes_left = ip->length;
#define put_func (void (*)(IBUF *ip))line_ptr
  switch (ditherflag)
  {
    case -6:
    case -5: put_func = put_m6m5; break;
    case -4:
    case -3: put_func = put_m4m3; break;
    case -2:
    case -1: put_func = put_m2m1; break;
    case  0: put_func = put_0; break;
    case  1: put_func = put_1; break;
    case  2: put_func = put_2; break;
    case  3: put_func = put_3; break;
    case  4: put_func = put_4; break;
    case  5: put_func = put_5; break;
    case  6: put_func = put_6; break;
    case  7: put_func = put_7; break;
    case  8: put_func = put_8; break;
  }
  ip->putline = put_func;
  patrun = th->ih.pat_run;
  if (th->ih.headlen == 0)				/* IMG */
    for (;;)
    {
      FGETC(&f, cdata);
      if (((char)i = cdata) != 0)
      {
	if (cdata <<= 1)			  /* solid run */
	{
	  (char)i >>= 7; do IPUTC(ip, i); while (cdata -= 2);
	}
	else					/* byte string */
	{
	  FGETC(&f, cdata);
	  do FICOPYC(&f, ip); while (--cdata);
      } }
      else
      {
	FTESTC(&f);
	if ((cdata = FFETCHC(&f)) != 0)		/* pattern run */
	{
	  line_ptr = ip->pat_buf;
	  for (i = patrun; --i >= 0;)
	    FGETC(&f, *line_ptr++);
	  do
	  { line_ptr = ip->pat_buf;
	    for (i = patrun; --i >= 0;)
	      IPUTC(ip, *line_ptr++);
	  }
	  while (--cdata);
	}
	else			       /* vertical replication */
	{
	  FGETC(&f, cdata);
	  if (++cdata) do ISKIPC(ip); while (--cdata);
	  else FGETC(&f, ip->vrc);
    } } }
  else if (patrun)			     /* IFF compressed */
    for (;;)
    {
      FTESTC(&f);
      if ((cdata = FFETCHC(&f)) >= 0)
	do FICOPYC(&f, ip); while (--cdata >= 0);
      else
      {
	FGETC(&f, (char)i);
	do IPUTC(ip, i); while (++cdata <= 0);
    } }
  else for (;;) FICOPYC(&f, ip);			/* IFF */
  finish: Fbufclose( &f );
  if ((ditherflag -= 4) > 0) Mfree( ip->dith_ptr );
  Mfree( ip );
  if (rg->xfac == 0) { Mfree( rg ); form_error( ENOENT ); return 0; }
  return rg;
}

GUIDE *load_iff( int fh, long len )
{
  FORMCHUNK fm;
  BMHDCHUNK bm;
  IMG_HEADER header;

  if (Fread( fh, sizeof(FORMCHUNK), &fm ) != sizeof(FORMCHUNK)
      || fm.fm_magic != FM_MAGIC || fm.fm_type != FM_IMAGE)
    { Fclose( fh ); form_error( ENOENT ); return 0; }
  for (;;)
  {
    len -= 8;
    if (Fread( fh, 8, &bm ) - 8)
      { Fclose( fh ); form_error( ENOENT ); return 0; }
    if (bm.bm_magic == BM_MAGIC) break;
#ifndef __TOS__
    fliplongs( (int *)&bm.bm_length, 1, 1 );
#endif
    len -= bm.bm_length += bm.bm_length & 1L;
    Fseek( bm.bm_length, fh, 1 );
  }
  if (Fread( fh, 20, &bm.bm_width ) != 20)
    { Fclose( fh ); form_error( ENOENT ); return 0; }
#ifndef __TOS__
  flipwords( (char *)&bm.bm_width, 8 );
#endif
  header.planes = bm.bm_nPlanes;
  header.sl_height = bm.bm_height;
  header.sl_width = bm.bm_width;
  header.pat_run = bm.bm_Compression;
  header.headlen = ~0U;
  return load_image( fh, (TIMG_HEADER *)&header,
		     len - (20 + sizeof(FORMCHUNK)) );
}

GUIDE *load_timg( int fh, TIMG_HEADER *th, long len );

GUIDE *load_img( int fh, long len )
{
  TIMG_HEADER th;

  if (Fread( fh, sizeof(IMG_HEADER), &th ) != sizeof(IMG_HEADER))
  {
    Fclose( fh ); form_error( ENOENT ); return 0;
  }
#ifndef __TOS__
  flipwords( (char *)&th, sizeof(IMG_HEADER) );
#endif
  if (th.ih.headlen < 8)
  {
    Fclose( fh ); form_error( ENOENT ); return 0;
  }
  len -= 16; th.ih.headlen -= 8; th.magic = 0;
  if (th.ih.headlen >= 2)
  {
    if (Fread( fh, 4, &th.magic ) - 4)
    {
      Fclose( fh ); form_error( ENOENT ); return 0;
    }
    len -= 4; th.ih.headlen -= 2;
    if (th.magic == T_MAGIC)
    {
      if (Fread( fh, 4, &th.comps ) - 4)
      {
	Fclose( fh ); form_error( ENOENT ); return 0;
      }
      switch (th.comps)
      {
	case 1:
	  *(long *)&th.green = 0;
	  if (th.ih.headlen < 2)
	  {
	    Fclose( fh ); form_error( ENOENT ); return 0;
	  }
	  len -= 4; th.ih.headlen -= 2; break;
	case 3:
	  if (Fread( fh, 4, &th.green ) - 4)
	  {
	    Fclose( fh ); form_error( ENOENT ); return 0;
	  }
	  if (th.ih.headlen < 4)
	  {
	    Fclose( fh ); form_error( ENOENT ); return 0;
	  }
	  len -= 8; th.ih.headlen -= 4; break;
	default:
	  Fclose( fh ); form_error( ENOENT ); return 0;
      }
      if (th.red < 0 || th.green < 0 || th.blue < 0 ||
	  (long)th.red + (long)th.green + (long)th.blue >
	  (long)th.ih.planes)
      {
	Fclose( fh ); form_error( ENOENT ); return 0;
      }
      return load_timg( fh, &th, len );
  } }
  return load_image( fh, &th, len );
}

void out_image( char *name )
{
  int fh;
  IMG_HEADER header;

  header.sl_width = xpixel; header.sl_height = ypixel;
  if (par.aspect != 1)
  {
    if ((fh = Fopen( name, 0 )) < 0) { form_error( -31 - fh ); return; }
    Fread( fh, sizeof(IMG_HEADER), &header ); Fclose( fh );
  }
  printbox( name, 0 ); fh = 0;
  if (par.aspect == 2)
  {
    fh = 1;
    if (*(long *)&par.x_scale)
    {
      header.sl_width *= (header.pix_width - 1) / out_width + 1;
      header.sl_height *= (header.pix_height - 1) / out_height + 1;
    }
    else
    {
      header.sl_width =
	(int)(((long)header.sl_width * header.pix_width) / out_width);
      header.sl_height =
	(int)(((long)header.sl_height * header.pix_height) / out_height);
  } }
  header.pix_width = 0;
  if (par.h_align) header.pix_width = xpixel - header.sl_width;
  if (par.h_align == 1) header.pix_width >>= 1;
  header.pix_height = 0;
  if (par.v_align) header.pix_height = ypixel - header.sl_height;
  if (par.v_align == 1) header.pix_height >>= 1;
  header.sl_width += header.pix_width - 1;
  header.sl_height += header.pix_height - 1;
  v_bit_image( ohandle, name, fh, par.x_scale, par.y_scale,
	       par.h_align, par.v_align, &header.pix_width );
  v_updwk( ohandle ); v_clrwk( ohandle ); printbox( 0, 0 );
}
