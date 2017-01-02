#include <aes.h>
#include <vdi.h>
#include <gemdefs.h>
#include <osbind.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <scancode.h>

typedef struct guide
{
  int	handle, page;
  long  offset;
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

  long	newoff, ow, oh, dw, dh, rw, rh;
  int   ox, oy, vhandle, rcflag,
	bas0[1], base[3],
	code[1];		/* Datenbereich.			*/
}
GUIDE;

#include "1stguide.h"
#include "util.h"

FROM( image ) IMPORT GUIDE *load_img( int fh );

#define V_CLRWK      3
#define V_ESC        5
#define V_PLINE      6
#define V_PMARKER    7
#define V_GTEXT      8
#define V_FILLAREA   9
#define V_GDP       11
#define VST_HEIGHT  12
#define VSL_WIDTH   16
#define VSM_HEIGHT  19
#define VST_POINT  107
#define VR_RECFL   114
#define VS_CLIP    129

#define V_ARC        2
#define V_PIESLICE   3
#define V_CIRCLE     4
#define V_ELLIPSE    5
#define V_ELLARC     6
#define V_ELLPIE     7
#define V_JUSTIFIED 10

#define V_BIT_IMAGE 23
#define V_TOPBOT 18501

typedef struct
{
  int id, headlen, version, transform, min_x, min_y, max_x, max_y,
      pwidth, pheight, ll_x, ll_y, ur_x, ur_y, bit_image;
}
META_HEADER;

static int  ox, oy, dx, dy, page, rcflag, rect[4];
static long ow, oh, dw, dh, rh;
static char *fname, pagestring[4],
	    wstring[] = "  Fenster       ^E",
	    ostring[] = "  Original      ^S";

#pragma warn -rpt
static OBJECT popup[] =
{
  0,  1,  7, G_BOX,    NONE,   SHADOWED, 0xFF1100L,	0, 0, 19, 4,
  2, -1, -1, G_STRING, NONE,   DISABLED, "- Mažstab / Seite -", 0,0,19,1,
  3, -1, -1, G_STRING, NONE,   NORMAL,   wstring,	0, 1, 19, 1,
  4, -1, -1, G_BUTTON, NONE,   NORMAL,   "1 \001",	0, 2, 4,  1,
  5, -1, -1, G_BUTTON, NONE,   NORMAL,   "^\004 -",	4, 2, 6,  1,
  6, -1, -1, G_BUTTON, NONE,   DISABLED, pagestring,	10,2, 3,  1,
  7, -1, -1, G_BUTTON, NONE,   NORMAL,   "+ ^\003",	13,2, 6,  1,
  0, -1, -1, G_STRING, LASTOB, NORMAL,   ostring,	0, 3, 19, 1
};
#pragma warn +rpt

static void new_item( GUIDE *tg, int new )
{
  if (new -= 2)
  {
    if (tg->dw == tg->rw && tg->dh == tg->rh) return;
    tg->dw = tg->rw; tg->dh = tg->rh;
  }
  else
  {
    if (tg->dw == tg->w && tg->dh == tg->h) return;
    tg->dw = tg->w; tg->dh = tg->h;
  }
  tg->bc = ((int)tg->dw + 7) >> 3; tg->lc = ((int)tg->dh + 7) >> 3;
  if (tg->page)
    do
    { int *p = tg->bas0; (char *)p += tg->offset;
      tg->newoff = tg->offset;
      ((int *)&tg->offset)[0] = p[0]; p[0] = V_CLRWK;
      ((int *)&tg->offset)[1] = p[3];
    }
    while (--tg->page);
  new_redraw();
}

#pragma warn -sig
static void transform( int n )
{
  int ix;

  for (ix = contrl[1] << 1; (ix -= 2) >= 0;)
    if (ix == n)
    {
      if ((ptsin[ix] = (ptsin[ix] * dw) / ow) == 0) ++ptsin[ix];
      if ((ptsin[ix+1] = (ptsin[ix+1] * dh) / oh) == 0) ++ptsin[ix+1];
    }
    else
    {
      ptsin[ix] -= ox;
      ptsin[ix] = (ptsin[ix] * dw) / ow;
      ptsin[ix] += dx;
      ptsin[ix+1] -= oy;
      ptsin[ix+1] = (ptsin[ix+1] * dh) / oh;
      if (rcflag == 0) ptsin[ix+1] = dh - ptsin[ix+1];
      ptsin[ix+1] += dy;
}   }

static void make_path( void )
{
  int i, k;

  i = contrl[3];
  contrl[3] += k = (int)(strrchr( fname, PATHSEP ) - fname + 1);
  while (--i >= 5) intin[i + k] = intin[i];
  while (--k >= 0) intin[k + 5] = fname[k];
}

static void do_command( void )
{
  int n = -2;

  switch (contrl[0])
  {
    case V_GDP: switch (contrl[5])
		{
		  case V_ARC:
		  case V_PIESLICE:  n += 2;
		  case V_CIRCLE:    n += 2;
		  case V_ELLIPSE:
		  case V_ELLARC:
		  case V_ELLPIE:
		  case V_JUSTIFIED: n += 4;
		}
		transform( n ); break;
    case VST_POINT: if (contrl[6] == ohandle)
		      intin[0] = (intin[0] * dh) / rh;
		    else
		    {
		      CALL_VDI();
		      contrl[0] = VST_HEIGHT;
		      contrl[1] = 1;
		      contrl[3] = 0;
		      ptsin[0] = 0;
		      if ((ptsin[1] = (ptsout[1] * dh) / rh) == 0)
			++ptsin[1];
		    }
		    break;
    case VST_HEIGHT:
    case VSM_HEIGHT:
    case V_TOPBOT:
    case VSL_WIDTH: n = 0;
    case V_ESC:     if (contrl[5] == V_BIT_IMAGE)
		      if (intin[6] != ':') make_path();
    case V_PLINE:
    case V_PMARKER:
    case V_GTEXT:
    case V_FILLAREA:
    case VR_RECFL:  transform( n ); break;
    case VS_CLIP:
      if (intin[0])
      {	{
	  int t;
	  t = ox; if (t > ptsin[0]) ptsin[0] = t;
	  t += ow - 1; if (t < ptsin[2]) ptsin[2] = t;
	  t = oy; if (t > ptsin[1]) ptsin[1] = t;
	  t += oh - 1; if (t < ptsin[3]) ptsin[3] = t;
	}
	transform( n );
	if (rect[0] > ptsin[0]) ptsin[0] = rect[0];
	if (rect[1] > ptsin[1]) ptsin[1] = rect[1];
	if (rect[2] < ptsin[2]) ptsin[2] = rect[2];
	if (rect[3] < ptsin[3]) ptsin[3] = rect[3];
      }
      else
      {
	++intin[0]; *(long *)ptsin = *(long *)rect;
	*(long *)(ptsin + 2) = *(long *)(rect + 2);
  }   }
  CALL_VDI();
}

static void draw_meta( GUIDE *tg, int *clip )
{
  long count;

  vr_recfl( handle, clip ); vs_clip( tg->vhandle, 1, clip );
  dx = tg->x - (tg->lbc << 3);
  dy = tg->y - (tg->tlc << 3);
  ox = tg->ox; oy = tg->oy; rcflag = tg->rcflag;
  ow = tg->ow; oh = tg->oh; dw = tg->dw; dh = tg->dh; rh = tg->rh;
  *(long *)rect = *(long *)clip;
  *(long *)(rect + 2) = *(long *)(clip + 2);
  fname = tg->path; clip = tg->base; (char *)clip += tg->offset;
  count = 2; count += *clip++ << 2; count += *clip++ << 1;
  (char *)clip += count;
  for (;;)
  {
    switch (contrl[0] = *clip++)
    {
      case	-1: clip = tg->base;
      case V_CLRWK: tg->newoff = (char *)clip - (char *)tg->base; return;
    }
    contrl[1] = *clip++;
    contrl[3] = *clip++;
    contrl[5] = *clip++;
    contrl[6] = tg->vhandle;
    if ((count = contrl[1] << 2) > 0)
      { memcpy( ptsin, clip, count ); (char *)clip += count; }
    if ((count = contrl[3] << 1) > 0)
      { memcpy( intin, clip, count ); (char *)clip += count; }
    do_command();
} }

static void free_meta( GUIDE *tg )
{
  if (vq_gdos()) vst_unload_fonts( tg->vhandle, 0 );
  v_clsvwk( tg->vhandle );
}

static int key_meta( GUIDE *tg, int code, int ks )
{
  switch (code)
  {
    case CNTRL_E:  new_item( tg, 2 ); return 0;
    case CNTRL_S:  new_item( tg, 7 ); return 0;
    case CNTRL_CL: if (tg->offset)
		   {
		     ks &= 3;
		     do
		     { int *p = tg->bas0; (char *)p += tg->offset;
		       tg->newoff = tg->offset;
		       ((int *)&tg->offset)[0] = p[0]; p[0] = V_CLRWK;
		       ((int *)&tg->offset)[1] = p[3];
		     }
		     while (--tg->page && ks);
		     full_redraw();
		   }
		   return 0;
    case CNTRL_CR: if (tg->newoff)
		   {
		     int *p = tg->bas0; (char *)p += tg->newoff;
		     p[0] = ((int *)&tg->offset)[0];
		     p[3] = ((int *)&tg->offset)[1];
		     tg->offset = tg->newoff; ++tg->page; full_redraw();
		   }
		   return 0;
  }
  return 1;
}

static void sclick_meta( GUIDE *tg, int mx, int my )
{
  popup[3].ob_state = popup[4].ob_state = tg->offset ? NORMAL : DISABLED;
  popup[6].ob_state = tg->newoff ? NORMAL : DISABLED;
  itoa( tg->page + 1, pagestring, 10 );
  *wstring = tg->dw == tg->w && tg->dh == tg->h ? 8 : ' ';
  *ostring = tg->dw == tg->rw && tg->dh == tg->rh ? 8 : ' ';
  switch (mx = popup_menu( popup, 5, mx, my, objc_draw ))
  {
    case 2:
    case 7: new_item( tg, mx ); break;
    case 3:
    case 4: do
	    { int *p = tg->bas0; (char *)p += tg->offset;
	      tg->newoff = tg->offset;
	      ((int *)&tg->offset)[0] = p[0]; p[0] = V_CLRWK;
	      ((int *)&tg->offset)[1] = p[3];
	    }
	    while (--tg->page && mx != 4);
	    full_redraw(); break;
    case 6: {
	      int *p = tg->bas0; (char *)p += tg->newoff;
	      p[0] = ((int *)&tg->offset)[0];
	      p[3] = ((int *)&tg->offset)[1];
	      tg->offset = tg->newoff; ++tg->page; full_redraw();
} }	    }

static void dclick_meta() { }

static long read_header( int fh )
{
  META_HEADER header;

  Fread( fh, sizeof(META_HEADER), &header );
#ifdef __TOS__
  flipwords( (char *)&header, sizeof(META_HEADER) );
#endif
  rcflag = header.ll_y > 0;
  ox = header.ll_x;
  oy = rcflag ? header.ur_y : -header.ll_y;
  ow = labs( (long)header.ur_x - (long)header.ll_x ) + 1;
  oh = labs( (long)header.ur_y - (long)header.ll_y ) + 1;
  dw = header.pwidth;
  dh = header.pheight;
  return header.headlen << 1;
}

static void set_dest( int handle, int pix_w, int pix_h )
{
  int point, pw, ph, val[7];

  vst_font( handle, 2 );
  point = vst_point ( handle, 99, val, val, val, val);
  vqt_fontinfo( handle, val, val, val + 2, val, val );
  val[3] += val[5] + 1;
  ph = (point * 25400L) / (72L * val[3]);
  pw = ((long)ph * pix_w) / pix_h;
  dw = (dw * 100) / pw;
  dh = (dh * 100) / ph;
  vst_font( handle, 1 );
}

GUIDE *load_meta( int fh, long len )
{
  GUIDE *rg;
  char  *p;
  long  hlen;
  int   *code, i, vhandle, work_in[11], work_out[57];

  len -= hlen = read_header( fh );
  if ((rg = Malloc( sizeof(GUIDE) + len )) == 0)
    { Fclose( fh ); form_error( EINVMEM ); return 0; }
  *(long *)rg->base = 0; code = rg->code;
  Fseek( hlen, fh, 0 ); len = Fread( fh, len, code ); Fclose( fh );
  code[len >> 1] = -1;
#ifdef __TOS__
  flipwords( (char *)code, len );
#endif
  if (code[0] == V_ESC && code[3] == V_BIT_IMAGE)
  {
    p = strrchr( Path, PATHSEP ) + 1;
    i = code[2] - 5; code += (code[1] << 1) + 9;
    while (--i >= 0) *p++ = *code++; *p = 0;
    Mfree( rg );
    if ((fh = Fopen( Path, 0 )) < 0) { form_error( -fh - 31 ); return 0; }
    return load_img( fh );
  }
  if (popup->ob_next == 0) { --popup->ob_next; fix_tree( popup, 7 ); }
  vhandle = phys_handle; work_in[10] = 2;
  i = 9; do work_in[i] = 1; while (--i >= 0);
  v_opnvwk( work_in, &vhandle, work_out );
  if (vhandle <= 0) { Mfree( rg ); form_error( EINVMEM ); return 0; }
  if (vq_gdos()) vst_load_fonts( vhandle, 0 );
  set_dest( vhandle, work_out[3], work_out[4] );
  rg->page = 0; rg->offset = 0; rg->newoff = 0;
  rg->ox = ox; rg->oy = oy; rg->rcflag = rcflag;
  rg->ow = ow; rg->oh = oh;
  rg->rw = rg->dw = dw;
  rg->rh = rg->dh = dh;
  rg->vhandle = vhandle; rg->xfac = rg->yfac = 8;
  rg->w = (int)rg->dw + 7; rg->h = (int)rg->dh + 7;
  rg->lc = rg->h >> 3; rg->bc = rg->w >> 3;
  rg->draw = draw_meta; rg->free = free_meta;
  rg->key = key_meta; rg->sclick = sclick_meta;
  rg->dclick = dclick_meta; return rg;
}

static int out_page( int fh )
{
  char *p;
  long count;

  *(p = strchr( fname, ' ' )) = 0; printbox( fname, ++page );
  while (Fread( fh, 2, contrl ) - 2 == 0 && *contrl != -1)
  {
    Fread( fh, 2, contrl + 1 );
    Fread( fh, 2, contrl + 3 );
    Fread( fh, 2, contrl + 5 );
#ifdef __TOS__
    flipwords( (char *)contrl, 12 );
#endif
    contrl[6] = ohandle;
    if ((count = contrl[1] << 2) > 0)
    {
      Fread( fh, count, ptsin );
#ifdef __TOS__
      flipwords( (char *)ptsin, count );
#endif
    }
    if ((count = contrl[3] << 1) > 0)
    {
      Fread( fh, count, intin );
#ifdef __TOS__
      flipwords( (char *)intin, count );
#endif
    }
    if (*contrl == V_CLRWK)
      { do_command(); printbox( 0, 0 ); *p = ' '; return 0; }
    do_command();
  }
  v_updwk( ohandle ); v_clrwk( ohandle );
  vs_clip( ohandle, 0, rect ); set_fonts( 0 );
  printbox( 0, 0 ); *p = ' '; return 1;
}

int (*out_meta( char *name, int fh ))( int fh )
{
  fname = name; Fseek( read_header( fh ), fh, 0 ); set_fonts( 1 );
  set_dest( ohandle, out_width, out_height ); rh = dh;
  if (par.meta_scale == 0) { dw = xpixel; dh = ypixel; }
  dx = 0;
  if (par.h_align) dx = xpixel - dw;
  if (par.h_align == 1) dx >>= 1;
  dy = 0;
  if (par.v_align) dy = ypixel - dh;
  if (par.v_align == 1) dy >>= 1;
  page = 0; *(long *)rect = 0; rect[2] = xpixel - 1; rect[3] = ypixel - 1;
  vs_clip( ohandle, 1, rect ); return out_page;
}