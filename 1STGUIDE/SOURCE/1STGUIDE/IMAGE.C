#include <aes.h>
#include <vdi.h>
#include <osbind.h>
#include <stdlib.h>
#include <string.h>
#include <scancode.h>
#include <errno.h>

typedef struct guide
{
  int	handle, planes, width, height,
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

#ifdef __TOS__
#define X_MAGIC  'XIMG'
#define FM_MAGIC 'FORM'
#define FM_IMAGE 'ILBM'
#define BM_MAGIC 'BMHD'
#define CM_MAGIC 'CMAP'
#define BO_MAGIC 'BODY'
#else
#define X_MAGIC  'X'+((long)'I'<<8)+((long)'M'<<16)+((long)'G'<<24)
#define FM_MAGIC 'F'+((long)'O'<<8)+((long)'R'<<16)+((long)'M'<<24)
#define FM_IMAGE 'I'+((long)'L'<<8)+((long)'B'<<16)+((long)'M'<<24)
#define BM_MAGIC 'B'+((long)'M'<<8)+((long)'H'<<16)+((long)'D'<<24)
#define CM_MAGIC 'C'+((long)'M'<<8)+((long)'A'<<16)+((long)'P'<<24)
#define BO_MAGIC 'B'+((long)'O'<<8)+((long)'D'<<16)+((long)'Y'<<24)
#endif

typedef struct
{
  int version, headlen, planes, pat_run,
      pix_width, pix_height, sl_width, sl_height;
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
static unsigned char Pli2Vdi[256], Vdi2Pli[256];
static char string2[] = { 0, 0, 0, 0 },
	    string3[] = "XXXXX *XXXXX Pixel",
	    string4[] = "---- XX Ebenen ----",
	    string5[] = " Sichern (XIMG) ^X",
	    ostring[] = "  Original-Pal. ^E",
	    sstring[] = "  Standard-Pal. ^S",
	    dithermatrix[] = {
				17, 61, 27, 51, 18, 63, 25, 49,
				41,  5, 38, 14, 42,  7, 37, 13,
				31, 55, 20, 56, 28, 52, 22, 59,
				34, 10, 44,  0, 32,  8, 46,  3,
				19, 62, 24, 48, 16, 60, 26, 50,
				43,  6, 36, 12, 40,  4, 39, 15,
				29, 53, 23, 58, 30, 54, 21, 57,
				33,  9, 47,  2, 35, 11, 45,  1
			     };
static int defcols[][3] = {
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

static long gray( int *rgb )
{
  return (598L * rgb[0] + 1174L * rgb[1] + 228L * rgb[2] + 15625) / 31250;
}

static int get_pix( void )
{
  int pel[2];

  *(long *)pel = 0; v_pmarker( handle, 1, pel );
  v_get_pixel( handle, 0, 0, pel, pel + 1 );
  return *pel;
}

static void get_true( int *rgb, void *buf )
{
  static int pxy[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  static MFDB screen = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	      check  = { 0, 0, 1, 0, 0, 0, 0, 0, 0 };

  vs_color( handle, 1, rgb ); v_pmarker( handle, 1, pxy );
  check.fd_addr = buf;
  check.fd_nplanes = check.fd_w = nplanes;
  check.fd_wdwidth = (check.fd_w + 15) >> 4;
  vro_cpyfm( handle, S_ONLY, pxy, &screen, &check );
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

static void setconv( void )
{
  static char vdi2pli[] = { 0,15,1,2,4,6,3,5,7,8,9,10,12,14,11,13 };
  int i, k;

  i = 255;
  do
  { char c = 0x80; c &= i;
    c += (i & (1 << 5)) << 1;
    c += (i & (1 << 3)) << 2;
    c += (i & (1 << 1)) << 3; half[i] = c;
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
  i = 255 * 2;
  do
  { k = *(int *)((char *)doub + i);
    ((int *)four)[i] = doub[(unsigned)k >> 8]; k &= 0xFF; k <<= 1;
    ((int *)four)[i + 1] = *(int *)((char *)doub + k);
  }
  while ((i -= 2) >= 0);
  i = 255; if (nplanes < 8) i = (1 << nplanes) - 1;
  k = 15; if (i < k || nplanes > k) i = k;
  maxconv = i;
  do
  { if (maxconv <= 15) k = vdi2pli[i];
    else
    {
      vsm_color( handle, i ); k = get_pix();
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
  int	   pli, *rgb_ptr;
  unsigned char c[4];

  count = count / 3 - 1; pli = 0;
  do
  { rgb_ptr = rgb_list + 3 * pli2vdi( pli, count );
    if (xflag) Fwrite( fh, 6, rgb_ptr );
    else
    {
      c[0] = (unsigned char)((*rgb_ptr++ * 255L) / 1000);
      c[1] = (unsigned char)((*rgb_ptr++ * 255L) / 1000);
      c[2] = (unsigned char)((*rgb_ptr++ * 255L) / 1000);
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
  do vq_color( handle, fh, 0, rgb_list + fh * 3 ); while (++fh < i);
  graf_mouse( BUSYBEE, 0 );
  if ((fh = Fopen( tg->path, 2 )) < 0)
  {
    graf_mouse( ARROW, 0 ); form_error( -fh - 31 ); return;
  }
  i = (1 << tg->planes) * 3;
  if (tg->x_flag & 1)
  {
    if (tg->x_flag & 2)
    {
      CHUNK_HEADER ch;

      ch.length = sizeof(FORMCHUNK);
      do
      { Fseek( (ch.length + 1) & -2, fh, 1 );
	if (Fread( fh, 8, &ch ) < 8)
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
  vr_trnfm( handle, src, des );
#ifndef __TOS__
  if (des->fd_stand) flipwords( des->fd_addr, size );
#endif
  if (src->fd_addr != des->fd_addr) Mfree( src->fd_addr );
}

static void draw_image( GUIDE *tg, int *clip )
{
  char  *des;
  long  size1, size2, lxy[4];
  int   i, k, index[2], pxy[8];
  GUIDE *hg;
  MFDB  d, s;

  *(long *)index = 0; ++*index;
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
    if (pxy[7] >= clip[3] && pxy[6] >= clip[0]) clip[0] = pxy[6] + 1;
    if (pxy[6] >= clip[2] && pxy[7] >= clip[1]) clip[1] = pxy[7] + 1;
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
    vsf_interior( handle, FIS_USER ); vsf_color( handle, 1 );
    vr_recfl( handle, pxy + 4 );
    vsf_color( handle, 0 ); vsf_interior( handle, FIS_SOLID ); return;
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
  tg = hg; d.fd_nplanes = s.fd_nplanes; s.fd_addr = des;
  des = d.fd_addr; vr_trnfm( handle, &d, &s ); d.fd_addr = 0;
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
static int key_image( GUIDE *tg, int code, int ks )
{
  switch (code)
  {
    case CNTRL_CL: if (tg->scale >= -2) change_scale(tg,-1,0,-1); return 0;
    case CNTRL_CR: if (tg->scale <=  2) change_scale(tg,-1,0, 1); return 0;
    case CNTRL_E: if ((tg->x_flag & 4) == 0) switch_colors( tg ); return 0;
    case CNTRL_S: if (tg->x_flag & 4) switch_colors( tg ); return 0;
    case CNTRL_X: save_palette( tg ); return 0;
  }
  return 1;
}
#pragma warn +par

static void sclick_image( GUIDE *tg, int mx, int my )
{
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

static void dummy_image() { }

static GUIDE *load_image( int fh, int planes, int height, int width,
			  int special, int headlen )
{
  GUIDE *rg;
  char  cdata, *line_buf, *endl_buf, *line_ptr,
	*raster_ptr, *plane_ptr, *buf_ptr;
  int	idata, i, linelen, line, vrc, ditherflag;
  long	bufsize, planesize, rastsize;
  MFDB	s;
  FBUF  f;

  s.fd_wdwidth = (width + 15) >> 4; linelen = s.fd_wdwidth << 1;
  s.fd_w = (width + 7) & -8; s.fd_h = (height + 7) & -8;
  bufsize = (long)linelen * planes;
  planesize = (long)linelen * s.fd_h;
  s.fd_nplanes = planes > 1 ? nplanes : 1;
  i = 0; if (s.fd_nplanes >= 16) i = 1 - (s.fd_nplanes >> 3);
  if (s.fd_nplanes < planes && (i = par.dithcol) != 0)
  {
    if (s.fd_nplanes < 3) i = 1;
    if (i == 1) s.fd_nplanes = 1;
  }
  rastsize = planesize * s.fd_nplanes;
  idata = 1 << planes; line = idata * 6 + 2;
  if ((ditherflag = i) != 0)
  {
    i = idata;
    if (ditherflag != 1) { i <<= 1; if (ditherflag != -1) i <<= 1; }
  }
  if ((rg = Malloc( sizeof(GUIDE) + line + rastsize )) == 0)
    { Fclose( fh ); form_error( EINVMEM ); return 0; }
  if ((buf_ptr = Malloc( bufsize + i )) == 0)
    { Mfree( rg ); Fclose( fh ); form_error( EINVMEM ); return 0; }
  if (popup->ob_next == 0)
    { --popup->ob_next; fix_tree( popup, 9 ); setconv(); }
  rg->planes = planes; rg->width = width; rg->height = height;
  width = headlen ? (s.fd_w >> 3) : linelen;
  line_buf = (char *)rg->rgb_list;
  raster_ptr = line_buf + line;
  endl_buf = buf_ptr + bufsize; --idata;
  if (headlen)	/* IMG */
  {
    i = 0;
    if (headlen >= 10)
    {
      Fread( fh, 4, line_buf );
      if (*(long *)line_buf == X_MAGIC)
      {
	Fread( fh, 2, line_buf );
#ifndef __TOS__
	flipwords( line_buf, 2 );
#endif
	line_buf += 2;
	do
	{ line_ptr = line_buf + 6 * pli2vdi( i, idata );
	  Fread( fh, 6, line_ptr );
#ifndef __TOS__
	  flipwords( line_ptr, 6 );
#endif
	  switch (ditherflag)
	  {
	    case -3:
	    case -2:
	      get_true( (int *)line_ptr,
			(long *)endl_buf + vdi2pli( i, idata ) );
	      break;
	    case -1:
	      get_true( (int *)line_ptr,
			(int *)endl_buf + vdi2pli( i, idata ) );
	      break;
	    case 1:
	      endl_buf[vdi2pli( i, idata )] = gray( (int *)line_ptr );
	      break;
	    case 2:
	      plane_ptr = endl_buf + (vdi2pli( i, idata ) << 2);
	      *plane_ptr++ = (*((int *)line_ptr)++ << 3) / 125;
	      *plane_ptr++ = (*((int *)line_ptr)++ << 3) / 125;
	      *plane_ptr   = (*(int *)line_ptr     << 3) / 125;
	} }
	while (++i <= idata);
	i = 1;
    } }
    Fseek( headlen << 1, fh, 0 );
  }
  else		/* IFF */
  {
    i = 2;
    for (;;)
    {
      CHUNK_HEADER ch;

      if (Fread( fh, 8, &ch ) < 8)
      {
	Mfree( buf_ptr ); Mfree( rg ); Fclose( fh );
	form_error( ENOENT ); return 0;
      }
#ifndef __TOS__
      fliplongs( (int *)&ch.length, 1, 1 );
#endif
      if (ch.magic == BO_MAGIC) break;
      if (ch.magic == CM_MAGIC)
      {
	*((int *)line_buf)++ = 0; i = 0;
	do
	{ Fread( fh, 3, f.sbuf );
	  line_ptr = line_buf + 6 * pli2vdi( i, idata );
	  *((int *)line_ptr)++ = (int)((f.sbuf[0] * 200L) / 51);
	  *((int *)line_ptr)++ = (int)((f.sbuf[1] * 200L) / 51);
	  *(int *)line_ptr     = (int)((f.sbuf[2] * 200L) / 51);
	  line_ptr -= 4;
	  switch (ditherflag)
	  {
	    case -3:
	    case -2:
	      get_true( (int *)line_ptr,
			(long *)endl_buf + vdi2pli( i, idata ) );
	      break;
	    case -1:
	      get_true( (int *)line_ptr,
			(int *)endl_buf + vdi2pli( i, idata ) );
	      break;
	    case 1:
	      endl_buf[vdi2pli( i, idata )] = gray( (int *)line_ptr );
	      break;
	    case 2:
	      plane_ptr = endl_buf + (vdi2pli( i, idata ) << 2);
	      *plane_ptr++ = (*((int *)line_ptr)++ << 3) / 125;
	      *plane_ptr++ = (*((int *)line_ptr)++ << 3) / 125;
	      *plane_ptr   = (*(int *)line_ptr     << 3) / 125;
	} }
	while ((ch.length -= 3) > 0 && ++i <= idata);
	i = 3;
      }
      Fseek( (ch.length + 1) & -2, fh, 1 );
  } }
  switch (ditherflag)
  {
    case -3:
    case -2:
      if ((i & 1) == 0)
      {
	get_true( defcols[15], (long *)endl_buf + idata );
	while (--idata >= 0)
	  get_true( defcols[idata & 15], (long *)endl_buf + idata );
      }
      else
      {
	vs_color( handle, 1, defcols[0] );
	v_pmarker( handle, 1, defcols[15] );
      }
      vs_color( handle, 1, defcols[15] ); break;
    case -1:
      if ((i & 1) == 0)
      {
	get_true( defcols[15], (int *)endl_buf + idata );
	while (--idata >= 0)
	  get_true( defcols[idata & 15], (int *)endl_buf + idata );
      }
      else
      {
	vs_color( handle, 1, defcols[0] );
	v_pmarker( handle, 1, defcols[15] );
      }
      vs_color( handle, 1, defcols[15] ); break;
    case 1: if (i & 1) break;
      endl_buf[idata] = 0;
      while (--idata >= 0)
	endl_buf[idata] = gray( defcols[idata & 15] );
      break;
    case 2: if (i & 1) break;
      ((long *)endl_buf)[idata] = 0;
      while (--idata >= 0)
      {
	plane_ptr = endl_buf + (idata << 2);
	(int *)line_ptr = defcols[idata & 15];
	*plane_ptr++ = (*((int *)line_ptr)++ << 3) / 125;
	*plane_ptr++ = (*((int *)line_ptr)++ << 3) / 125;
	*plane_ptr   = (*(int *)line_ptr     << 3) / 125;
  }   }
  rg->x_flag = i; rg->scale = 0;
  rg->mfdb.fd_addr = raster_ptr;
  rg->mfdb.fd_stand = 0; if (ditherflag >= 0) ++rg->mfdb.fd_stand;
  rg->mfdb.fd_nplanes = s.fd_nplanes;
  rg->mfdb.fd_wdwidth = s.fd_wdwidth;
  *(long *)&rg->w = *(long *)&rg->mfdb.fd_w = *(long *)&s.fd_w;
  rg->bc = rg->w >> 3; rg->lc = rg->h >> 3;
  rg->xfac = rg->yfac = 8; rg->draw = draw_image;
  rg->free = dummy_image; rg->key = key_image;
  rg->sclick = sclick_image; rg->dclick = dummy_image;
  f.handle = fh; Fbufopen( &f );
  memset( raster_ptr, 0, rastsize ); line = 0;
  for (;;)
  {
    vrc = 1; line_buf = buf_ptr;
    do
    { line_ptr = line_buf; line_buf += width;
      do
	if (headlen)						/* IMG */
	  switch (idata = Fgetc( &f ))
	  {
	    case 0:    if ((idata = Fgetc( &f )) != 0)	/* pattern run */
		       {
			 plane_ptr = line_ptr;
			 for (i = special; --i >= 0;)
			   *line_ptr++ = Fgetc( &f );
			 while (--idata > 0)
			   for (i = special; --i >= 0;)
			     *line_ptr++ = *plane_ptr++;
			 break;
		       }			/* vertical replication */
		       if (Fgetc( &f ) == 0xFF) vrc = Fgetc( &f ); break;
	    case 0x80: idata = Fgetc( &f );		  /* bit string */
		       while (--idata >= 0) *line_ptr++ = Fgetc( &f );
		       break;
	    default:   if (idata < 0)	      /* unexpected end of file */
			 { line_ptr = line_buf; break; }
		       i = 0; if ((char)idata < 0) --i;    /* solid run */
		       (char)idata <<= 1;
		       do *line_ptr++ = i; while (idata -= 2);
	  }
	else if (special == 0) *line_ptr++ = Fgetc( &f );	/* IFF */
	else if ((cdata = Fgetc( &f )) >= 0)	     /* IFF compressed */
	     { do *line_ptr++ = Fgetc( &f ); while (--cdata >= 0); }
	     else
	     { i = Fgetc( &f ); do *line_ptr++ = i; while (++cdata <= 0); }
      while (line_ptr < line_buf);
    }
    while ((line_buf += width & 1) < endl_buf);
    vrc += line;
    do
    { line_ptr = buf_ptr; i = linelen;
      switch (ditherflag)
      {
	case -3:
	  do
	  { cdata = 0x80;
	    do
	    { int pix = 0; idata = 4; plane_ptr = line_ptr;
	      do
	      { if (*plane_ptr & cdata) pix |= idata;
		idata <<= 1;
	      }
	      while ((plane_ptr += linelen) < endl_buf);
	      *((long *)raster_ptr)++ = *(long *)(endl_buf + pix);
	    }
	    while ((unsigned char)cdata >>= 1);
	    ++line_ptr;
	  }
	  while (--i);
	  break;
	case -2:
	  do
	  { cdata = 0x80;
	    do
	    { int pix = 0; idata = 4; plane_ptr = line_ptr;
	      do
	      { if (*plane_ptr & cdata) pix |= idata;
		idata <<= 1;
	      }
	      while ((plane_ptr += linelen) < endl_buf);
	      plane_ptr = endl_buf + pix;
	      *raster_ptr++ = *plane_ptr++;
	      *raster_ptr++ = *plane_ptr++;
	      *raster_ptr++ = *plane_ptr++;
	    }
	    while ((unsigned char)cdata >>= 1);
	    ++line_ptr;
	  }
	  while (--i);
	  break;
	case -1:
	  do
	  { cdata = 0x80;
	    do
	    { int pix = 0; idata = 2; plane_ptr = line_ptr;
	      do
	      { if (*plane_ptr & cdata) pix |= idata;
		idata <<= 1;
	      }
	      while ((plane_ptr += linelen) < endl_buf);
	      *((int *)raster_ptr)++ = *(int *)(endl_buf + pix);
	    }
	    while ((unsigned char)cdata >>= 1);
	    ++line_ptr;
	  }
	  while (--i);
	  break;
	case 0: idata = 1;
	  do
	  { if (--idata) line_buf += planesize;
	    else { line_buf = raster_ptr; idata = s.fd_nplanes; }
	    do *((int *)line_buf)++ |= *((int *)line_ptr)++;
	    while (i -= 2);
	    line_buf -= i = linelen;
	  }
	  while (line_ptr < endl_buf);
	  if (planes >= s.fd_nplanes) { raster_ptr += linelen; break; }
	  do
	  { int mask; idata = 1;
	    line_ptr = raster_ptr; mask = *((int *)raster_ptr)++;
	    do
	    { line_ptr += planesize;
	      if (idata < planes) mask &= *(int *)line_ptr;
	      else *(int *)line_ptr = mask;
	    }
	    while (++idata < s.fd_nplanes);
	  }
	  while (i -= 2);
	  break;
	case 1: line_buf = dithermatrix + ((line & 7) << 3);
	  do
	  { cdata = 0x80;
	    do
	    { int pix = 0; idata = 1; plane_ptr = line_ptr;
	      do
	      { if (*plane_ptr & cdata) pix |= idata;
		idata <<= 1;
	      }
	      while ((plane_ptr += linelen) < endl_buf);
	      if (endl_buf[pix] <= *line_buf++) *raster_ptr |= cdata;
	    }
	    while ((unsigned char)cdata >>= 1);
	    line_buf -= 8; ++raster_ptr; ++line_ptr;
	  }
	  while (--i);
	  break;
	case 2: line_buf = dithermatrix + ((line & 7) << 3);
	  do
	  { cdata = 0x80;
	    do
	    { int pix = 0; idata = 4; plane_ptr = line_ptr;
	      do
	      { if (*plane_ptr & cdata) pix |= idata;
		idata <<= 1;
	      }
	      while ((plane_ptr += linelen) < endl_buf);
	      plane_ptr = endl_buf + pix; pix = 0;
	      (char)idata = *line_buf++;
	      if (*plane_ptr++ > (char)idata) pix += 4; /* Rot  */
	      if (*plane_ptr++ > (char)idata) pix += 2; /* GrÅn */
	      if (*plane_ptr++ > (char)idata) pix += 1; /* Blau */
	      plane_ptr = raster_ptr;
	      if ((pix <<= 4) == 0)	/* Schwarz */
	      {
		idata = s.fd_nplanes;
		do { *plane_ptr |= cdata; plane_ptr += planesize; }
		while (--idata);
	      }
	      else if (pix != 0x70)	/* nicht Weiss */
	      {
		if (((char)pix <<= 1) < 0) *plane_ptr |= cdata;
		plane_ptr += planesize;
		if (((char)pix <<= 1) < 0) *plane_ptr |= cdata;
		plane_ptr += planesize;
		if (((char)pix <<= 1) < 0) *plane_ptr |= cdata;
	    } }
	    while ((unsigned char)cdata >>= 1);
	    line_buf -= 8; ++raster_ptr; ++line_ptr;
	  }
	  while (--i);
      }
      if (++line == height)
      {
	Fbufclose( &f ); Mfree( buf_ptr ); return rg;
    } }
    while (line < vrc);
} }

GUIDE *load_iff( int fh )
{
  FORMCHUNK fm;
  BMHDCHUNK bm;

  Fread( fh, sizeof(FORMCHUNK), &fm );
  if (fm.fm_magic != FM_MAGIC || fm.fm_type != FM_IMAGE)
    { Fclose( fh ); form_error( ENOENT ); return 0; }
  for (;;)
  {
    if (Fread( fh, 8, &bm ) < 8)
      { Fclose( fh ); form_error( ENOENT ); return 0; }
    if (bm.bm_magic == BM_MAGIC) break;
#ifndef __TOS__
    fliplongs( (int *)&bm.bm_length, 1, 1 );
#endif
    Fseek( (bm.bm_length + 1) & -2, fh, 1 );
  }
  Fread( fh, 20, &bm.bm_width );
#ifndef __TOS__
  flipwords( (char *)&bm.bm_width, 8 );
#endif
  return load_image( fh, bm.bm_nPlanes, bm.bm_height, bm.bm_width,
		     bm.bm_Compression, 0 );
}

GUIDE *load_img( int fh )
{
  IMG_HEADER header;

  Fread( fh, sizeof(IMG_HEADER), &header );
#ifndef __TOS__
  flipwords( (char *)&header, sizeof(IMG_HEADER) );
#endif
  return load_image( fh, header.planes, header.sl_height, header.sl_width,
		     header.pat_run, header.headlen );
}

#pragma warn -sig
void out_image( char *name )
{
  int        fh;
  IMG_HEADER header;

  header.sl_width = xpixel; header.sl_height = ypixel;
  if (par.aspect != 1)
  {
    if ((fh = Fopen( name, 0 )) < 0) { form_error( -fh - 31 ); return; }
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
	((long)header.sl_width * header.pix_width) / out_width;
      header.sl_height =
	((long)header.sl_height * header.pix_height) / out_height;
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