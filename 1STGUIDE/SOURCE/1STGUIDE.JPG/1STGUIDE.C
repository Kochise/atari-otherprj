#include <aes.h>
#include <vdi.h>
#include <gemdefs.h>
#include <osbind.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <scancode.h>
#include <vaproto.h>

#include "util.h"

FROM( util ) IMPORT jmp_buf setjmp_buffer;

typedef struct guide
{
  int	handle, s1, s2, s3,	/* AES-Fensterkennung; specials.	*/
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
}
GUIDE;

FROM( loader ) IMPORT GUIDE *load_file(char *name,int fh,long len,int flag);
FROM( loader ) IMPORT void config( int ks );
FROM( loader ) IMPORT void spool( int ks );

#define W_KIND	NAME | CLOSER | FULLER | MOVER | SIZER |\
		UPARROW | DNARROW | LFARROW | RTARROW | VSLIDE | HSLIDE

#define AC_SPOOL	100
#define SCRP_OPEN	1003
#define AC_HELP		1025

GUIDE	     *vg;
static GUIDE *tg;		/* Aktuelle Fensterstruktur.		*/
int	     phys_handle, handle, gl_hchar, nplanes,
	     xdesk, ydesk, wdesk, hdesk, xmax, ymax, ekind, ks,
	     xpixel, ypixel, out_width, out_height, ohandle,
	     VA_Flags;		/* Flags der VA_PROTOSTATUS send. Appl. */
static int   starter_id,	/* Identifik. der VA_START send. Appl.	*/
	     pipe[8], appl_id, menu_id, key, aw, wbox, hbox, gl_wchar;
char	     Path[256], StartPath[128], spool_buf[1040],
	     VA_Name[8];	/* Name der VA_PROTOSTATUS send. Appl.	*/
static char  File[128], Title[128];
static DTA   mydta;

static void no_more_windows( void )
{
  form_alert( 1, "[1][Keine weiteren Fenster"
		 "|vorhanden! Abhilfe durch"
		 "|Schlieáen eines Fensters.][  OK  ]" );
}

static void no_gdos( void )
{
  form_alert( 1, "[1][1STGUIDE-Warnung:"
		 "|Die Ger„teausgabe funktioniert"
		 "|nur, wenn Sie ein GDOS im"
		 "|AUTO-Ordner installiert haben.][  Aha  ]" );
}

#ifdef __TOS__
static BASPAG *Pgetpd( GUIDE *tg )
{
  BASPAG *pd;
  SYSHDR *sys;
  void   *oldstack;

  oldstack = (void *)Super( 0 );
  sys = *(SYSHDR **)0x4F2;
  sys = sys->os_base;	/* wegen Fehler in alter AHDI-Version :-( */
  if (sys->os_version >= 0x102) pd = *sys->_run;
  else if ((sys->os_palmode >> 1) - 4) pd = *(BASPAG **)0x602C;
  else pd = *(BASPAG **)0x873CL;	/* Spanisches TOS 1.0 :-( */
  if (tg) while (tg->actpd != pd && pd) pd = pd->p_parent;
  Super( oldstack );
  return pd;
}
#else
#define Pgetpd( tg ) Pgetpd()
#endif

void AVSendMessage( int msg, int p3, int p4 )
{
  if (starter_id == -1) return; if (tg) aw = tg->handle;
  pipe[0] = msg; pipe[1] = appl_id; pipe[2] = 0;
  pipe[3] = p3; pipe[4] = p4; pipe[5] = 0;
  *(char **)(pipe + 6) = "1STGUIDE";
  appl_write( starter_id, 16, pipe );
}

static void delete_guide( void )
{
  GUIDE *hg, *lg;

  hg = tg; lg = hg->next;
  (*hg->free)( hg ); Mfree( hg );
  if (hg == lg) tg = 0;
  else
  {
    tg = lg;
    while (lg->next != hg) lg = lg->next;
    lg->next = tg;
} }

void set_name( void )
{
  GUIDE *hg = tg;
  char *name;
  long len;

  len = strlen( name = hg->name );
  if (*hg->hist == 0 && name[len - 1] != '"')
    len = strlen( name = hg->path );
  if (((int)len -= (hg->w - wbox) / gl_wchar) < 0) len = 0;
  wind_set( hg->handle, WF_NAME, name + len );
}

static void set_hslider( void )
{
  GUIDE *hg = tg;
  int hslide;

  if ((hslide = (int)((hg->lbc * 1000L) / hg->bc)) != hg->hslide)
    wind_set( hg->handle, WF_HSLIDE, hg->hslide = hslide );
}

static void set_vslider( void )
{
  GUIDE *hg = tg;
  int vslide;

  if ((vslide = (int)((hg->tlc * 1000L) / hg->lc)) != hg->vslide)
    wind_set( hg->handle, WF_VSLIDE, hg->vslide = vslide );
}

void set_allslider( void )
{
  GUIDE *hg = tg;
  int lines, count;

  count = lines = hg->h / hg->yfac;
  if (hg->lc > 0) { count += hg->lc; set_vslider(); }
  wind_set( hg->handle, WF_VSLSIZE, (int)((lines * 1000L) / count) );
  count = lines = hg->w / hg->xfac;
  if (hg->bc > 0) { count += hg->bc; set_hslider(); }
  wind_set( hg->handle, WF_HSLSIZE, (int)((lines * 1000L) / count) );
}

static void calc_border( void )
{
  GUIDE *hg = tg;

  (*hg->dclick)( hg, 0, 0, -2 ); *(long *)&par.xwin = *(long *)&hg->x;
  wind_calc( WC_BORDER, W_KIND, hg->x, hg->y, hg->w, hg->h,
	     pipe + 4, pipe + 5, pipe + 6, pipe + 7 );
}

static void redraw( void )
{
  GUIDE *hg = tg;
  void (*draw)( GUIDE *tg, int *clip ) = hg->draw;
  int outer_parts, mouse_off = 0;

  pipe[6] += pipe[4] - 1; pipe[7] += pipe[5] - 1;
  if (xdesk > pipe[4]) pipe[4] = xdesk;
  if (ydesk > pipe[5]) pipe[5] = ydesk;
  if ( xmax < pipe[6]) pipe[6] = xmax;
  if ( ymax < pipe[7]) pipe[7] = ymax;
  wind_get( hg->handle, WF_FIRSTXYWH, pipe, pipe+1, pipe+2, pipe+3 );
  while (pipe[2] && pipe[3])
  {
    pipe[2] += pipe[0] - 1; pipe[3] += pipe[1] - 1; outer_parts = 4;
    if (pipe[4] >= pipe[0]) { pipe[0] = pipe[4]; --outer_parts; }
    if (pipe[5] >= pipe[1]) { pipe[1] = pipe[5]; --outer_parts; }
    if (pipe[6] <= pipe[2]) { pipe[2] = pipe[6]; --outer_parts; }
    if (pipe[7] <= pipe[3]) { pipe[3] = pipe[7]; --outer_parts; }
    if (pipe[2] >= pipe[0] && pipe[3] >= pipe[1])
    {
      if (mouse_off == 0) { mouse_off = 1; graf_mouse( M_OFF, 0 ); }
      (*draw)( hg, pipe );
    }
    if (outer_parts == 0) break;
    wind_get( hg->handle, WF_NEXTXYWH, pipe, pipe+1, pipe+2, pipe+3 );
  }
  if (mouse_off) graf_mouse( M_ON, 0 );
}

void full_redraw( void )
{
  *(long *)(pipe + 4) = *(long *)&tg->x;
  *(long *)(pipe + 6) = *(long *)&tg->w; redraw();
}

void new_redraw( void )
{
  GUIDE *hg = tg;

  *(long *)&hg->lbc = 0;
  hg->bc -= hg->w / hg->xfac; hg->lc -= hg->h / hg->yfac;
  set_allslider(); full_redraw();
}

void new_size( long force_redraw )
{
  long oldxy, oldwh, oldlbctlc;
  GUIDE *hg = tg;

  oldxy = *(long *)&hg->x;
  oldwh = *(long *)&hg->w;
  oldlbctlc = *(long *)&hg->lbc;
  if (force_redraw == 0)
  {
    hg->bc += hg->w / hg->xfac; hg->lc += hg->h / hg->yfac;
    wind_calc( WC_WORK, W_KIND, pipe[4], pipe[5], pipe[6], pipe[7],
				&hg->x, &hg->y, &hg->w, &hg->h );
  }
  hg->bc -= hg->w /= hg->xfac; hg->w *= hg->xfac;
  if (hg->bc < hg->lbc) if ((hg->lbc = hg->bc) < 0) hg->lbc = 0;
  hg->lc -= hg->h /= hg->yfac; hg->h *= hg->yfac;
  if (hg->lc < hg->tlc) if ((hg->tlc = hg->lc) < 0) hg->tlc = 0;
  hg->x = (hg->x + 3) & -8;
  oldxy -= *(long *)&hg->x;
  oldwh -= *(long *)&hg->w;
  if (oldxy |= oldwh)
  {
    calc_border();
    wind_set( hg->handle, WF_CURRXYWH, pipe[4], pipe[5], pipe[6], pipe[7] );
    wind_get( hg->handle, WF_WORKXYWH, &hg->x, &hg->y, &hg->w, &hg->h );
    if (oldwh) set_name();
  }
  if (oldwh |= force_redraw) set_allslider();
  oldlbctlc -= *(long *)&hg->lbc;
  if (oldlbctlc |= force_redraw)
  {
    pipe[0] = WM_REDRAW;
    pipe[1] = appl_id;
    pipe[2] = 0;
    pipe[3] = hg->handle;
    *(long *)(pipe + 4) = *(long *)&hg->x;
    *(long *)(pipe + 6) = *(long *)&hg->w; appl_write( appl_id, 16, pipe );
} }

static int create_window( void )
{
  int  o_xmin, o_ymin;
  long o_wmax, o_hmax;

  wind_get( 0, WF_WORKXYWH, &xdesk, &ydesk, &wdesk, &hdesk );
  xmax = xdesk + wdesk - 1; ymax = ydesk + hdesk - 1;
  o_xmin = par.xmin; o_ymin = par.ymin;
  o_wmax = par.wmax; o_hmax = par.hmax;
  wind_calc( WC_WORK, W_KIND, xdesk - 1, ydesk, wdesk + 1, hdesk,
	     &par.xmin, &par.ymin, &par.wmax, &par.hmax );
  par.xwin = (int)(((long)par.wmax * (par.xwin - o_xmin)) / o_wmax)
	     + par.xmin;
  par.wwin = (int)(((long)par.wmax * par.wwin) / o_wmax);
  par.ywin = (int)(((long)par.hmax * (par.ywin - o_ymin)) / o_hmax)
	     + par.ymin;
  par.hwin = (int)(((long)par.hmax * par.hwin) / o_hmax);
  return wind_create( W_KIND, xdesk - 1, ydesk, wdesk + 1, hdesk );
}

static void show_window( int x, int y, int w, int h )
{
  GUIDE *hg = tg;

  set_name(); calc_border();
  graf_growbox( x, y, w, h, pipe[4], pipe[5], pipe[6], pipe[7] );
  wind_open( hg->handle, pipe[4], pipe[5], pipe[6], pipe[7] );
  wind_get( hg->handle, WF_WORKXYWH, &hg->x, &hg->y, &hg->w, &hg->h );
  *(long *)&hg->hslide = 0; set_allslider();
  AVSendMessage( AV_ACCWINDOPEN, hg->handle, 0 );
}

#pragma warn -par
int filebox( char *title )
{
  int but[1];

  memswap( strrchr( Path, PATHSEP ) + 1, File, 128 );
#ifdef __TOS__
  if (global[0] >= 0x130) fsel_exinput( Path, File, but, title );
  else
#endif
    fsel_input( Path, File, but );
  memswap( strrchr( Path, PATHSEP ) + 1, File, 128 );
  return *but;
}
#pragma warn +par

void open_window( int parent, int over, int x, int y, int w, int h,
		  char *file, char *key )
{
  int   flag, found, wh, fh;
  long  length;
  GUIDE	*pg;
  char	*buf, *p;

  found = flag = 1;
  do
  { if (over == 0)
      if ((wh = create_window()) < 0)
      {
	if (parent <= 0) { no_more_windows(); slct_close(); return; }
	if (form_alert( 1, "[3][Keine weiteren Fenster|vorhanden!"
			   "|Wollen Sie das|Ausgangsfenster|ber"
			   "laden ?][šberladen|Abbrechen]" ) == 2)
	  { slct_close(); return; }
	over = 1;
      }
    if (parent > 0)
    {
      pg = tg; p = Path; strcpy( p, pg->path );
      buf = pg->hist; buf += (unsigned char)*buf;
      if (over == -1)
      {
	while (*--buf); buf -= 12; strcpy( p + buf[1], buf + 13 );
      }
      else
      {
        if (file[1] != ':')
        {
	  while (*p++); --p;
	  for (;;)
	  {
	    while (*--p != PATHSEP);
	    if (strncmp( file, "..\\", 3 )) break;
	    file += 3; if (p[-1] == ':') break;
	  }
	  ++p;
	}
	buf[1] = p - Path;
	memcpy( buf + 2, &pg->s1, 10 ); buf += 12; *buf++ = 0;
	strcpy( buf, p ); strcpy( p, file ); while (*buf++); --buf;
    } }
    else if (flag)
    {
      if (*(p = file))
	if ((p = strrchr( file, PATHSEP )) != 0 && file[1] == ':')
	  { ++p; strcpy( Path, file ); }
	else { p = strrchr( Path, PATHSEP ) + 1; strcpy( p, file ); }
      if (*p == 0)
      {
	slct_open();
	strcpy( Title, "1stGuide: Datei(en) " );
	strcat( Title, over ? "ber" : "neu" );
	strcat( Title, parent ? "laden" : "dumpen" );
	if (filebox( Title ) == 0)
	  { if (over == 0) wind_delete( wh ); slct_close(); return; }
    } }
    if (flag)
    {
      p = strrchr( Path, PATHSEP ) + 1; strcpy( Title, p );
      if ((flag = slct_first( &mydta, 0x27 )) != 0)
      {
	slct_close();
	if (*p == 0)
	  { if (over == 0) wind_delete( wh ); slct_close(); return; }
	found = flag = Findfirst( Path, &mydta, 0x27 );
    } }
    for (;;)
    {
      do
      { if (flag == 0)
	{
	  strcpy( p, mydta.d_fname ); length = mydta.d_length;
	  if ((flag = slct_next( &mydta )) != 0) slct_close();
	  if (found == 0) flag = Findnext( &mydta );
	  if ((fh = Fopen( Path, 0 )) >= 0) break;
	  if (*Title == 0) break;
	  strcpy( p, Title );
	}
	if ((fh = Fopen( Path, 0 )) >= 0)
	  { length = Fseek( 0, fh, 2 ); Fseek( 0, fh, 0 ); }
      }
      while (0);
      if (fh < 0) form_error( -31 - fh );
      else
      {
	wind_update( BEG_MCTRL ); graf_mouse( BUSYBEE, 0 );
	pg = load_file( p, fh, length, parent );
	graf_mouse( ARROW, 0 ); wind_update( END_MCTRL );
	if (pg) break;
      }
      if (flag)
	{ if (over == 0) wind_delete( wh ); slct_close(); return; }
    }
    strcpy( pg->path, Path ); *(long *)&pg->lbc = 0;
    pg->actpd = Pgetpd( 0 );
    if (tg)
    {
      pg->next = tg->next; tg->next = pg;
      *(long *)&pg->x = *(long *)&tg->x;
      if (over == 0) { pg->x += wbox; pg->y += hbox; }
    }
    else
    {
      pg->next = pg;
      *(long *)&pg->x = *(long *)&par.xwin;
    }
    if (parent > 0)
    {
      strcpy( pg->name, tg->name );
      memcpy( pg->hist, tg->hist, sizeof(tg->hist) );
      *pg->hist = buf - tg->hist;
      if (over == -1)
      {
	*strrchr( pg->name, ':' ) = 0; memcpy( &pg->s1, buf + 2, 10 );
      }
      else
      {
	buf = pg->name; while (*buf++); --buf;
	if (buf[-1] == '"') { --buf; while (*--buf != '"'); --buf; }
	*buf++ = ':'; *buf++ = ' '; strcpy( buf, key );
    } }
    else
    {
      if (parent == 0) struplo( pg->path );
      strcpy( pg->name, p ); *pg->hist = 0;
    }
    if (over)
    {
      over = 0; pg->handle = tg->handle;
      *(long *)&pg->w = *(long *)&tg->w;
      *(long *)&pg->hslide = *(long *)&tg->hslide;
      delete_guide(); set_name(); new_size( 1 );
    }
    else
    {
      pg->handle = wh;
      if (wbox * 4 > pg->w) pg->w = wbox * 4;
      if (hbox * 4 > pg->h) pg->h = hbox * 4;
      if ((wh = par.xmin + par.wmax - pg->w) < pg->x)
      {
	if (wh < par.xmin) { wh = par.xmin; pg->w = par.wmax; }
	pg->x = wh;
      }
      pg->x &= -8;
      if ((wh = par.ymin + par.hmax - pg->h) < pg->y)
      {
	if (wh < par.ymin) { wh = par.ymin; pg->h = par.hmax; }
	pg->y = wh;
      }
      pg->bc -= pg->w /= pg->xfac; pg->w *= pg->xfac;
      pg->lc -= pg->h /= pg->yfac; pg->h *= pg->yfac;
      tg = pg; show_window( x, y, w, h );
  } }
  while (flag == 0);
}

static int rebuild_windows( void )
{
  GUIDE *hg = tg;
  int flag = 0;

  if (hg && hg->handle == 0)
  {
    int count = 0; do ++count; while ((hg = hg->next) != tg);
    do
    { if (tg->actpd != Pgetpd( tg )) delete_guide();
      else
      {
	if ((tg->handle = create_window()) < 0)
	{
	  no_more_windows();
	  do delete_guide(); while (--count);
	  return flag;
	}
	show_window( 0, 0, 0, 0 ); flag = 1; tg = tg->next;
    } }
    while (--count);
  }
  return flag;
}

static void close_window( void )
{
  GUIDE *hg = tg;

  wind_close( hg->handle ); calc_border();
  graf_shrinkbox( 0, 0, 0, 0, pipe[4], pipe[5], pipe[6], pipe[7] );
  AVSendMessage( AV_ACCWINDCLOSED, hg->handle, 0 );
  wind_delete( hg->handle ); delete_guide();
}

static void doscroll( int delx, int dely )
{
  MFDB srcMFDB, desMFDB;
  long ldx, ldy, firstwh;
  void (*draw)( GUIDE *tg, int *clip );
  GUIDE *hg = tg;

  if (delx)
  {
    delx += hg->lbc;
    if (hg->bc < delx) delx = hg->bc; if (delx < 0) delx = 0;
    if (delx -= hg->lbc)
    {
      hg->lbc += delx; graf_mouse( M_OFF, 0 ); set_hslider();
  } }
  if (dely)
  {
    dely += hg->tlc;
    if (hg->lc < dely) dely = hg->lc; if (dely < 0) dely = 0;
    if (dely -= hg->tlc)
    {
      hg->tlc += dely; if (delx == 0) graf_mouse( M_OFF, 0 );
      set_vslider();
  } }
  if ((delx | dely) == 0) return;
  srcMFDB.fd_addr = 0; desMFDB.fd_addr = 0; draw = hg->draw;
  ldx = (long)delx * hg->xfac; ldy = (long)dely * hg->yfac;
  wind_get( hg->handle, WF_FIRSTXYWH, pipe, pipe+1, pipe+2, pipe+3 );
  firstwh = *(long *)(pipe + 2);
  while (pipe[2] && pipe[3])
  {
    pipe[2] += pipe[0] - 1; pipe[3] += pipe[1] - 1;
    if (xdesk > pipe[0]) pipe[0] = xdesk;
    if (ydesk > pipe[1]) pipe[1] = ydesk;
    if ( xmax < pipe[2]) pipe[2] = xmax;
    if ( ymax < pipe[3]) pipe[3] = ymax;
    delx = pipe[2]; dely = pipe[3];
    if ((delx -= pipe[0]) >= 0 && (dely -= pipe[1]) >= 0)
    {
      if ((ldx < 0 ? -ldx : ldx) <= delx && (ldy < 0 ? -ldy : ldy) <= dely)
      {
	*(long *)(pipe + 4) = *(long *)pipe; delx = pipe[0];
	*(long *)(pipe + 6) = *(long *)(pipe + 2); dely = pipe[2];
	if (ldx < 0)
	  { pipe[2] += (int)ldx; pipe[4] -= (int)ldx; }
	else
	  { pipe[0] += (int)ldx; pipe[6] -= (int)ldx; }
	if (ldy < 0)
	  { pipe[3] += (int)ldy; pipe[5] -= (int)ldy; }
	else
	  { pipe[1] += (int)ldy; pipe[7] -= (int)ldy; }
	vro_cpyfm( handle, S_ONLY, pipe, &srcMFDB, &desMFDB );
	if (ldy)
	{
	  if (ldy < 0) pipe[3] = pipe[5] - 1;
	  else pipe[1] = pipe[7] + 1;
	}
	if (ldx)
	{
	  if (ldy)
	  {
	    pipe[0] = delx; pipe[2] = dely; (*draw)( hg, pipe );
	    pipe[0] = delx; pipe[2] = dely;
	    pipe[1] = pipe[5]; pipe[3] = pipe[7];
	  }
	  if (ldx < 0) pipe[2] = pipe[4] - 1;
	  else pipe[0] = pipe[6] + 1;
      } }
      (*draw)( hg, pipe );
    }
    if (firstwh == *(long *)&hg->w) break;
    wind_get( hg->handle, WF_NEXTXYWH, pipe, pipe+1, pipe+2, pipe+3 );
  }
  graf_mouse( M_ON, 0 );
}

void goto_line( int line ) { doscroll( 0, line - tg->tlc ); }

static void goto_col( int col ) { doscroll( col - tg->lbc, 0 ); }

static void full_window( void )
{
  GUIDE *hg = tg;

  wind_get( hg->handle, WF_CURRXYWH, pipe, pipe+1, pipe+2, pipe+3 );
  if (((ks & 3) != 0) ^ par.fulldef)
  {
    if (hg->bc < (wdesk + 1 - pipe[2]) / hg->xfac)
    {
      pipe[6] = pipe[2] + hg->bc * hg->xfac;
      if (wbox * 5 > pipe[6]) pipe[6] = wbox * 5;
    }
    else pipe[6] = wdesk + 1;
    if (hg->lc < (hdesk - pipe[3]) / hg->yfac)
    {
      pipe[7] = pipe[3] + hg->lc * hg->yfac;
      if (hbox * 6 > pipe[7]) pipe[7] = hbox * 6;
    }
    else pipe[7] = hdesk;
    pipe[4] = xdesk + wdesk - pipe[6] < pipe[0]
	    ? xdesk + wdesk - pipe[6] : pipe[0];
    pipe[5] = ydesk + hdesk - pipe[7] < pipe[1]
	    ? ydesk + hdesk - pipe[7] : pipe[1];
  }
  else wind_get( hg->handle, WF_FULLXYWH, pipe+4, pipe+5, pipe+6, pipe+7 );
  if (abs( pipe[0] - pipe[4] ) < 8 && pipe[1] == pipe[5] &&
      abs( pipe[6] - pipe[2] ) < hg->xfac &&
      abs( pipe[7] - pipe[3] ) < hg->yfac)
    wind_get( hg->handle, WF_PREVXYWH, pipe+4, pipe+5, pipe+6, pipe+7 );
  if (pipe[6] <= pipe[2] && pipe[7] <= pipe[3])
    graf_shrinkbox( pipe[4], pipe[5], pipe[6], pipe[7],
		    pipe[0], pipe[1], pipe[2], pipe[3] );
  else if (pipe[6] >= pipe[2] && pipe[7] >= pipe[3])
    graf_growbox( pipe[0], pipe[1], pipe[2], pipe[3],
		  pipe[4], pipe[5], pipe[6], pipe[7] );
  new_size( 0 );
}

static void print_file( char *name )
{
  int spool_id;
  char message[132];

  strcpy( message, "        " );
  strncpy( message, par.extspool, strlen( par.extspool ) );
  if ((spool_id = appl_find( message )) >= 0)
  {
    strcpy( Path, name );
    *(char **)(pipe + 4) = Path;
    pipe[3] = (int)strlen( Path );
    pipe[6] = 1; pipe[7] = 0;
    pipe[0] = AC_SPOOL; pipe[1] = appl_id; pipe[2] = 0;
    appl_write( spool_id, 16, pipe ); return;
  }
  strcpy( message, "[1][Um Dateien auf ein Ger„t|"
		   "ausgeben zu k”nnen, muá ein|"
		   "Spooler-Accessory namens|" );
  strcat( message, par.extspool );
  strcat( message, " installiert werden.][" );
  if ((spool_id = appl_find( "CHMELEON" )) >= 0)
  {
    name = strrchr( StartPath, PATHSEP ) + 1;
    strcpy( name, par.extspool ); strcat( name, ".AC*" );
    if (Findfirst( StartPath, &mydta, 0x27 ) == 0)
    {
      ++message[1]; strcat( message, "Install.|Abbrechen]" );
      if (form_alert( 1, message ) == 2) return;
      strcpy( name, mydta.d_fname );
      *(char **)(pipe + 3) = StartPath;
      pipe[0] = VA_START; pipe[1] = appl_id; pipe[2] = 0;
      appl_write( spool_id, 16, pipe ); return;
  } }
  strcat( message, "  Aha  ]" ); form_alert( 1, message );
}

static void get_spool( char *cmd )
{
  char *q;

  if (*(q = cmd))
    if ((q = strrchr( cmd, PATHSEP )) != 0 && cmd[1] == ':')
      { ++q; strcpy( Path, cmd ); }
    else { q = strrchr( Path, PATHSEP ) + 1; strcpy( q, cmd ); }
  if (*q == 0)
  {
    slct_open();
    if (filebox( "1stGuide: Datei(en) spoolen" ) == 0)
      { slct_close(); return; }
  }
  q = strrchr( Path, PATHSEP ) + 1;
  if (slct_first( &mydta, 0x27 ) == 0) strcpy( q, mydta.d_fname );
  else if (*q == 0) { slct_close(); return; }
  for (;;)
  {
    if (strlen( spool_buf ) + strlen( Path ) >= sizeof(spool_buf) - 12)
    {
      form_alert( 1, "[1][1STGUIDE-Warnung:"
		     "|Der Spooler-Puffer ist voll.][  Aha  ]" ); break;
    }
    strcat( strcat( spool_buf, Path ), " " );
    if (slct_next( &mydta )) break;
    strcpy( q, mydta.d_fname );
  }
  slct_close();
}

static void check_spool( void )
{
  int i, work_in[103], work_out[57];

  if (*spool_buf)
  {
    if (ohandle <= 0)
    {
      i = 102; do work_in[i] = 1; while (--i > 0);
      work_in[0] = par.out_handle; work_in[10] = 2; work_in[11] = 255;
      v_opnwk( work_in, &ohandle, work_out );
      xpixel = work_out[0] + 1; ypixel = work_out[1] + 1;
      out_width = work_out[3]; out_height = work_out[4];
      if (ohandle <= 0)
      {
	form_alert( 1, "[1][1STGUIDE-Warnung:"
			"|Fehler beim ™ffnen des"
			"|Ausgabeger„tes."
			"|Ausgabe wird abgebrochen"
			"|und Spooler-Puffer geleert.][ Oh jeh ]" );
	*spool_buf = 0; return;
    } }
    ekind |= MU_TIMER;
} }

static int rmouse( void )
{
  int mbut[1], dummy[1];

  graf_mkstate( dummy, dummy, mbut, &ks ); return *mbut & 2;
}

static void spool_cmd( char *cmd )
{
  char *p;

  if (vq_gdos() == 0) { no_gdos(); return; }
  if (cmd && *cmd)
    do
    { if ((p = strchr( cmd, ' ' )) != 0) *p++ = 0;
      get_spool( cmd );
    }
    while ((cmd = p) != 0 && *cmd);
  else if (rmouse()) config( ks ); else get_spool( "" );
  check_spool();
}

static void scrp_copy( char *name )
{
  char *fbuf, sbuf[4];
  long blen, count;
  int  rfh, wfh;

  if (set_scrp() == 0) return;
  if ((rfh = Fopen( name, 0 )) < 0)
  {
    form_error( -31 - rfh ); return;
  }
  strcat( Path, strrchr( name, '.' ) );
  if ((wfh = Fcreate( Path, 0 )) < 0)
  {
    form_error( -31 - wfh );
    Fclose( rfh ); return;
  }
  graf_mouse( BUSYBEE, 0 );
  blen = Fseek( 0, rfh, 2 ); Fseek( 0, rfh, 0 );
  for (;;)
  {
    if (blen <= sizeof(sbuf))
      { blen = sizeof(sbuf); fbuf = sbuf; break; }
    if ((fbuf = Malloc( blen )) != 0) break;
    blen = Mavail() & -sizeof(sbuf);
  }
  while ((count = Fread( rfh, blen, fbuf )) > 0 &&
	 Fwrite( wfh, count, fbuf ) == count);
  if (fbuf != sbuf) Mfree( fbuf );
  graf_mouse( ARROW, 0 );
  Fclose( wfh ); Fclose( rfh );
}

static void get_entry( char *cmd, int over )
{
  if ((ks & 6) == 6 && *cmd == 0) cmd = par.indexfile;
  open_window( (ks & 12) - 12, over, 0, 0, 0, 0, cmd, 0 );
}

static void hndl_key( void )
{
  GUIDE *hg = tg;
  int wh;

  if (hg == 0) return; wind_get( 0, WF_TOP, &wh );
  do if (hg->handle == wh)
     {
       tg = hg;
       switch (key)
       {
	 case CUR_UP:    doscroll( 0, -1 ); break;
	 case CUR_DOWN:  doscroll( 0,  1 ); break;
	 case SHFT_CU:
	 case PAGE_UP:   doscroll( 0, 1 - hg->h / hg->yfac ); break;
	 case SHFT_CD:
	 case PAGE_DOWN: doscroll( 0, hg->h / hg->yfac - 1 ); break;
	 case CUR_LEFT:  doscroll( -1, 0 ); break;
	 case CUR_RIGHT: doscroll(  1, 0 ); break;
	 case SHFT_CL:   doscroll( 1 - hg->w / hg->xfac, 0 ); break;
	 case SHFT_CR:   doscroll( hg->w / hg->xfac - 1, 0 ); break;
	 case HOME:      goto_line( 0 ); break;
	 case SHFT_HOME:
	 case CUR_END:   goto_line( hg->lc ); break;
	 case INSERT:    goto_col( 0 ); break;
	 case SHFT_INS:  goto_col( hg->bc ); break;
	 case CNTRL_O:   get_entry( "", 0 ); break;
	 case CNTRL_D:   get_entry( "", 1 ); break;
	 case CNTRL_U:   close_window(); break;
	 case CNTRL_Q:   do close_window(); while (tg);
			 break;
	 case CNTRL_F:   full_window(); break;
	 case CNTRL_P:   (((ks & 10) == 10) ^ par.spoolflag ?
			 print_file : spool_cmd)( hg->path );
			 break;
	 case CNTRL_A:   config( ks ); break;
	 case UNDO:
	 case ESC:
	   if (*hg->hist) open_window( 1, -1, 0, 0, 0, 0, 0, 0 );
	   break;
	 case CNTRL_W:
	   if ((VA_Flags & (1 << 6)) == 0 && hg->next != hg)
	     { wind_set( hg->next->handle, WF_TOP ); break; }
	 default:
	   if ((*hg->key)( hg, key, ks ))
	     if (key == CNTRL_C) scrp_copy( hg->path );
	     else AVSendMessage( AV_SENDKEY, ks, key );
       }
       break;
     }
  while ((hg = hg->next) != tg);
}

static void activate( char *cmd, int over )
{
  int flag;
  char *p;

  if (over)
  {
    GUIDE *hg = tg; flag = pipe[3]; over = 0;
    if (hg)
      do if (hg->handle == flag) { tg = hg; over = 1; break; }
      while ((hg = hg->next) != tg);
  }
  if (pipe[1] != starter_id)
    { starter_id = pipe[1]; AVSendMessage( AV_PROTOKOLL, 2, 0 ); }
  flag = rebuild_windows();
  if (cmd && *cmd)
    do
    { if ((p = strchr( cmd, ' ' )) != 0) *p++ = 0;
      get_entry( cmd, over ); over = 0;
    }
    while ((cmd = p) != 0 && *cmd);
  else if (flag == 0) if (rmouse()) config( ks ); else get_entry( "", 0 );
}

static void find_index( char *index, char *file )
{
  int  check;
  char link, c, *pbuf, buf[128];
  FBUF f;

  if (pipe[1] != starter_id)
    { starter_id = pipe[1]; AVSendMessage( AV_PROTOKOLL, 2, 0 ); }
  rebuild_windows();
  if (index == 0 || *index == 0) { get_entry( file, 0 ); return; }
  do
  { if (Findfirst( file, &mydta, 0x27 ) == 0)
    {
      strcpy( buf, file );
      if ((pbuf = strrchr( buf, PATHSEP )) != 0) pbuf++;
      else pbuf = buf;
      strcpy( pbuf, mydta.d_fname ); f.buf_size = mydta.d_length;
      if ((f.handle = Fopen( buf, 0 )) >= 0) break;
    }
    if ((f.handle = Fopen( file, 0 )) < 0)
      { form_error( -31 - f.handle ); return; }
    f.buf_size = Fseek( 0, f.handle, 2 ); Fseek( 0, f.handle, 0 );
  }
  while (0);
  graf_mouse( BUSYBEE, 0 ); Fbufopen( &f ); link = LINK;
  if ((check = setjmp( setjmp_buffer )) == 0)
    do
    { do FGETC(&f, c);
      while (c != link);
      pbuf = buf;
      do
      { FGETC(&f, c);
	*pbuf++ = c;
      }
      while (c != link);
      *--pbuf = 0;
      check = stricmp( buf, index );
      pbuf = buf;
      do
      { FGETC(&f, c);
	*pbuf++ = c;
      }
      while (c != link);
      *--pbuf = 0;
    }
    while (check);
  Fbufclose( &f ); graf_mouse( ARROW, 0 );
  if (check)
  {
    strcpy( buf, "[1][1stGuide:| |\"" );
    strncat( buf, index, 28 );
    strcat( buf, "\"| |nicht gefunden][ Abbruch |Index]" );
    if (form_alert( 1, buf ) == 2) get_entry( file, 0 );
  }
  else { strcpy( Path, file ); file = buf; get_entry( file, 0 ); }
}

static void invalidate( void )
{
  GUIDE *hg = tg;

  if (hg)
  {
    int count = 0; do ++count; while ((hg = hg->next) != tg);
    do
    { if (tg->actpd != Pgetpd( tg )) delete_guide();
      else { tg->handle = 0; tg = tg->next; }
    }
    while (--count);
} }

#ifdef __TOS__
static void rec_ddmsg( char *p )
{
  static char pipename[] = "U:\\PIPE\\DRAGDROP.AA";
  long size[11], oldpipesig, len;
  int fh;

  pipename[17] = *p++; pipename[18] = *p;
  if ((fh = Fopen( pipename, 2 )) < 0) return;
  oldpipesig = Psignal( SIGPIPE, SIG_IGN );
  p = (char *)size; *p++ = 0;
  *p++ = 'A'; *p++ = 'R'; *p++ = 'G'; *p++ = 'S';
  len = 28; do *p++ = 0; while (--len);
  len = 33;
  while (Fwrite( fh, len, size ) == len)
  {
    size[0] = 0;
    if (Fread( fh, 10, (int *)size + 1 ) != 10) break;
    if ((size[0] -= 8) <= 0) break;
    do
    { len = 32; if (size[0] < len) len = size[0];
      Fread( fh, len, size + 3 );
    }
    while (size[0] -= len);
    if (size[1] == 'ARGS')
    {
      len = size[2];
      if ((p = Malloc( len + 1 )) != 0)
      {
	if (Fwrite( fh, 1, size ) - 1 == 0)
	{
	  Fread( fh, len, p ); p[len] = 0;
	  activate( p, 1 );
	}
	Mfree( p ); break;
      }
      *(char *)size += 1;
    }
    *(char *)size += 2; len = 1;
  }
  Psignal( SIGPIPE, oldpipesig ); Fclose( fh );
}
#endif

static void hndl_message( void )
{
  GUIDE *hg = tg;
  int wh = pipe[3];

  switch (pipe[0])
  {
    case WM_REDRAW:
      if (hg)
	do if (hg->handle == wh) { tg = hg; redraw(); break; }
        while ((hg = hg->next) != tg);
      break;
    case WM_TOPPED:
      wind_set( wh, WF_TOP ); break;
    case WM_CLOSED:
      if (hg)
	do if (hg->handle == wh)
	   {
	     tg = hg;
	     if (*hg->hist == 0 || par.overflag == 0) close_window();
	     else open_window( 1, -1, 0, 0, 0, 0, 0, 0 );
	     break;
	   }
        while ((hg = hg->next) != tg);
      break;
    case WM_FULLED:
      if (hg)
	do if (hg->handle == wh) { tg = hg; full_window(); break; }
        while ((hg = hg->next) != tg);
      break;
    case WM_ARROWED:
      if (hg)
	do if (hg->handle == wh)
	   {
	     int delx = 0, dely = 0, speed; tg = hg;
	     if ((speed = pipe[5]) >= 0) speed = -1;
	     for (;;)
	     {
	       switch (pipe[4])
	       {
		 case WA_UPPAGE: speed *= hg->h / hg->yfac - 1;
		 case WA_UPLINE: dely += speed; break;
		 case WA_DNPAGE: speed *= hg->h / hg->yfac - 1;
		 case WA_DNLINE: dely -= speed; break;
		 case WA_LFPAGE: speed *= hg->w / hg->xfac - 1;
		 case WA_LFLINE: delx += speed; break;
	 	 case WA_RTPAGE: speed *= hg->w / hg->xfac - 1;
		 case WA_RTLINE: delx -= speed;
	       }
	       if ((speed = pipe[7]) >= 0) break;
	       pipe[7] = 0; pipe[4] = pipe[6];
	     }
	     doscroll( delx, dely ); break;
           }
        while ((hg = hg->next) != tg);
      break;
    case WM_HSLID:
      if (hg)
	do
	  if (hg->handle == wh)
	  {
	    tg = hg;
	    goto_col( (int)((hg->bc * (long)pipe[4] + 500) / 1000) );
	    break;
	  }
        while ((hg = hg->next) != tg);
      break;
    case WM_VSLID:
      if (hg)
	do if (hg->handle == wh)
	   {
	     tg = hg;
	     goto_line( (int)((hg->lc * (long)pipe[4] + 500) / 1000) );
	     break;
	   }
        while ((hg = hg->next) != tg);
      break;
    case WM_SIZED:
    case WM_MOVED:
      if (hg)
	do if (hg->handle == wh) { tg = hg; new_size( 0 ); break; }
        while ((hg = hg->next) != tg);
      break;
    case WM_NEWTOP:
      wind_set( wh, WF_TOP ); break;
    case WM_BOTTOMED:
      wind_set( wh, WF_BOTTOM ); break;
    case AC_CLOSE:
      starter_id = -1; VA_Flags = 0; invalidate(); break;
    case AP_TERM:
      if (hg)
        if (hg->handle == 0) do delete_guide(); while (tg);
        else do close_window(); while (tg);
      break;
#ifdef __TOS__
    case AP_DRAGDROP:
      rec_ddmsg( (char *)(pipe + 7) ); break;
#endif
    case AC_SPOOL:
      spool_cmd( *(char **)(pipe + 4) ); break;
    case AC_HELP:
      find_index( *(char **)(pipe + 3),
		  pipe[5] == 0x1993 &&
		  *(char **)(pipe + 6) &&
		  **(char **)(pipe + 6)
		  ? *(char **)(pipe + 6) : par.indexfile ); break;
    case AC_OPEN:
      if (pipe[4] != menu_id) break;
      *(char **)(pipe + 4) = 0;
    case SCRP_OPEN:
      pipe[3] = pipe[5]; pipe[1] = starter_id;
    case VA_START:
      if (((ks & 10) == 10) ^ par.outdef)
	spool_cmd( *(char **)(pipe + 3) );
      else activate( *(char **)(pipe + 3), 0 );
      break;
    case VA_DRAGACCWIND:
      activate( *(char **)(pipe + 6), 1 ); break;
    case VA_PROTOSTATUS:
      strncpy( VA_Name, *(char **)(pipe + 6), 8 );
      VA_Flags = wh; break;
    case VA_FILEFONT:
      if (hg)
	do if (hg->handle == aw)
	   {
	     tg = hg; (*hg->dclick)( hg, wh, pipe[4], -1 );
	   }
	while ((hg = hg->next) != tg);
      break;
    case WM_SHADED:
      wind_get( wh, WF_CURRXYWH, pipe, pipe+1, pipe+2, pipe+3 );
      graf_shrinkbox( pipe[0], pipe[1], pipe[2], hbox,
		      pipe[0], pipe[1], pipe[2], pipe[3] );
      break;
    case WM_UNSHADED:
      wind_get( wh, WF_CURRXYWH, pipe, pipe+1, pipe+2, pipe+3 );
      graf_growbox( pipe[0], pipe[1], pipe[2], hbox,
		    pipe[0], pipe[1], pipe[2], pipe[3] );
} }

static void online_scroll( int flag )
{
  GUIDE *hg = tg;
  int base, len, mx, my, mbut;

  if (flag)
  {
    len = hg->h - ((hbox - 2) << 1); base = hg->h / hg->yfac;
    base = (int)((base * (long)len) / (hg->lc + base));
    if (hbox > base) base = hbox;
    len -= base; base = hg->y + hbox - 3 + (base >> 1);
  }
  else
  {
    len = hg->w - ((wbox - 2) << 1); base = hg->w / hg->xfac;
    base = (int)((base * (long)len) / (hg->bc + base));
    if (wbox > base) base = wbox;
    len -= base; base = hg->x + wbox - 3 + (base >> 1);
  }
  graf_mouse( FLAT_HAND, 0 ); wind_update( BEG_MCTRL );
  graf_mkstate( &mx, &my, &mbut, pipe ); mbut ^= 1;
  do
    if (flag) { if ((my -= base) < 0) my = 0; if (len < my) my = len;
		goto_line( (int)((my * (long)hg->lc) / len) );
	      }
    else      { if ((mx -= base) < 0) mx = 0; if (len < mx) mx = len;
		goto_col( (int)((mx * (long)hg->bc) / len) );
	      }
  while ((evnt_multi( MU_BUTTON | MU_M1, 1, 1, mbut, 1, mx, my, 1, 1,
		      0, 0, 0, 0, 0, 0,0,0, &mx, &my, pipe,pipe,pipe,pipe )
	  & MU_BUTTON) == 0);
  wind_update( END_MCTRL ); graf_mouse( ARROW, 0 );
  if (mbut) evnt_button( 1, 1, 0, pipe, pipe, pipe, pipe );
}

static void hndl_event( void )
{
  int event, mx, my, mbut, mclicks;

  event = evnt_multi( ekind, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		      pipe, 100, 0, &mx, &my, &mbut, &ks, &key, &mclicks );

  wind_update( BEG_UPDATE );

  if (event & MU_MESAG) hndl_message();
  if (event & MU_KEYBD)
  {
    while (evnt_multi( MU_TIMER | MU_KEYBD, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, pipe, 0, 0, pipe, pipe,
	   pipe, pipe, pipe, pipe ) & MU_KEYBD);
    hndl_key();
  }
  if (event & MU_BUTTON)
  {
    GUIDE *hg = tg;

    if (hg)
    {
      int wh = wind_find( mx, my );
      do if (hg->handle == wh)
	 {
	   tg = hg;
	   if (--mclicks == 0)
	   {
		  if (mx >= hg->x + hg->w && hg->lc > 0) online_scroll(1);
	     else if (my >= hg->y + hg->h && hg->bc > 0) online_scroll(0);
	     else if (mx < hg->x + wbox && my < hg->y) close_window();
	     else (*hg->sclick)( hg, mx, my );
	   }
	   else if (mx - hg->x >= 0 && mx - hg->x < hg->w &&
		    my - hg->y >= 0 && my - hg->y < hg->h)
	     (*hg->dclick)( hg, mx, my, ((ks & 15) != 0) ^ par.overflag );
	   break;
         }
      while ((hg = hg->next) != tg);
  } }
  if (event & MU_TIMER)
  {
    if (vg) { tg = vg; (*vg->dclick)( vg, 0, 0, -3 ); }
    if (*spool_buf) spool( ks );
    else
    {
      if (ohandle > 0) { v_clswk( ohandle ); ohandle = 0; }
      if (vg == 0) ekind &= ~MU_TIMER;
  } }
  wind_update( END_UPDATE );
}

#if DESKACC
void main()
#define RET_VAL
#else
int main( int argc, char **argv )
#define RET_VAL 1
#endif
{
  int i, work_in[11], work_out[57];

  if ((appl_id = appl_init()) == -1) return RET_VAL;
  handle = phys_handle = graf_handle( &gl_wchar, &gl_hchar, &wbox, &hbox );
  i = 9; do work_in[i] = 1; while (--i >= 0);
  work_in[10] = 2; v_opnvwk( work_in, &handle, work_out );
  vq_extnd( handle, 1, work_out ); nplanes = work_out[4];
  vst_alignment( handle, 0, 5, work_out, work_out );
  vsf_interior( handle, FIS_HOLLOW );
  Path[0] = 'A' + (char)Dgetdrv(); Path[1] = ':'; Dgetpath( Path + 2, 0 );
  strcat( Path, "\\" ); strcpy( StartPath, Path ); strcpy( File, "*.*" );
  *spool_buf = 0; tg = vg = 0; ohandle = 0; VA_Flags = 0; starter_id = -1;
  ekind = MU_MESAG | MU_KEYBD | MU_BUTTON;

#if DESKACC
  menu_register( appl_id, "  1st Guide" );
  load_fonts( 1 ); for (;;) hndl_event();
#else
#ifdef __TOS__
  if (_app == 0 || global[0] >= 0x400)
    menu_id = menu_register( appl_id, "  1st Guide" );
  if (_app == 0) { load_fonts( 1 ); for (;;) hndl_event(); }
#endif
  graf_mouse( ARROW, 0 );
  if (rmouse() && argc <= 1)
  {
    wind_update( BEG_UPDATE ); config( ks ); wind_update( END_UPDATE );
  }
  if ((((ks & 10) == 10) ^ par.outdef) == 0)
  {
    if (--argc <= 0) get_entry( "", 0 );
    else do get_entry( *++argv, 0 ); while (--argc);
  }
  else if (vq_gdos() == 0) no_gdos();
       else
       {
	 if (--argc <= 0) get_spool( "" );
	 else do get_spool( *++argv ); while (--argc);
	 check_spool();
       }
  while (tg || *spool_buf) hndl_event();
  v_clsvwk( handle ); appl_exit(); return 0;
#endif
}
