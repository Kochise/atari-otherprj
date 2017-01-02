#include <aes.h>
#include <vdi.h>
#include <gemdefs.h>
#include <osbind.h>
#include <string.h>
#include <scancode.h>
#include <vaproto.h>

#include "util.h"

typedef struct guide
{
  int	handle, s1, s2, s3,	/* AES-Fensterkennung; specials.	*/
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
}
GUIDE;

FROM( loader ) IMPORT GUIDE *load_file( char *name, long len, int flag );
FROM( loader ) IMPORT void config( int ks );
FROM( loader ) IMPORT void spool( int ks );

#define W_KIND	NAME | CLOSER | FULLER | MOVER | SIZER |\
		UPARROW | DNARROW | LFARROW | RTARROW | VSLIDE | HSLIDE

#define AC_SPOOL	100
#define SCRP_OPEN	1003
#define AC_HELP		1025

#define WH			tg->handle
#define PATH			tg->path
#define WNAME			tg->name
#define HIST			tg->hist
#define BC			tg->bc
#define LBC			tg->lbc
#define LC			tg->lc
#define TLC			tg->tlc
#define XFAC			tg->xfac
#define YFAC			tg->yfac
#define X			tg->x
#define Y			tg->y
#define W			tg->w
#define H			tg->h
#define HSLID			tg->hslide
#define VSLID			tg->vslide
#define KEY( tg, code, ks )	(*tg->key)( tg, code, ks )
#define DRAW( tg, clip )	(*tg->draw)( tg, clip )
#define FREE( tg )		(*tg->free)( tg )
#define SCLICK( tg, mx, my )	(*tg->sclick)( tg, mx, my )
#define DCLICK( tg, mx, my,ks )	(*tg->dclick)( tg, mx, my, ks )

static GUIDE *tg;		/* Aktuelle Fensterstruktur.		*/
int	     phys_handle, handle, gl_hchar,
	     xdesk, ydesk, wdesk, hdesk, xmax, ymax, nplanes, ks,
	     xpixel, ypixel, out_width, out_height, ohandle = 0,
	     VA_Flags = 0;	/* Flags der VA_PROTOSTATUS send. Appl. */
static int   starter_id = -1,	/* Identifik. der VA_START send. Appl.	*/
	     ekind = MU_MESAG | MU_KEYBD | MU_BUTTON,
	     tw,		/* Aktuelles Fenster.			*/
	     pipe[8], key, appl_id, wbox, hbox, gl_wchar, aw;
char	     Path[128], StartPath[128], spool_buf[1040],
	     VA_Name[8];	/* Name der VA_PROTOSTATUS send. Appl.	*/
static char  no_win[] = "[1][Keine weiteren Fenster"
			"|vorhanden! Abhilfe durch"
			"|Schlie·en eines Fensters.][  OK  ]",
	     no_gdos[] = "[1][1STGUIDE-Warnung:"
			"|Die GerÑteausgabe funktioniert"
			"|nur, wenn Sie ein GDOS im"
			"|AUTO-Ordner installiert haben.][  Aha  ]";
static DTA   mydta;

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
  if (starter_id == -1) return; aw = tw;
  pipe[0] = msg; pipe[1] = appl_id; pipe[2] = 0;
  pipe[3] = p3; pipe[4] = p4; pipe[5] = 0;
  *(char **)(pipe + 6) = "1STGUIDE";
  appl_write( starter_id, 16, pipe );
}

static void delete_guide( void )
{
  GUIDE *hg, *lg;

  hg = lg = tg->next; FREE( tg ); Mfree( tg );
  if (hg == tg) tg = 0;
  else
  {
    while (hg->next != tg) hg = hg->next;
    tg = hg->next = lg;
} }

static int find_guide( void )
{
  GUIDE *hg;

  if ((hg = tg) != 0)
    do if (WH == tw) return 1; while ((tg = tg->next) != hg);
  return 0;
}

void set_name( void )
{
  char *name;
  int  len;

  len = (int)strlen( name = WNAME );
  if (*HIST == 0 && name[len - 1] != '"') len = (int)strlen( name = PATH );
  if ((len -= (W - wbox) / gl_wchar) < 0) len = 0;
  wind_set( WH, WF_NAME, name + len );
}

static void set_hslider( void )
{
  int hslide;

  if ((hslide = (int)((LBC * 1000L) / BC)) != HSLID)
    wind_set( WH, WF_HSLIDE, HSLID = hslide );
}

static void set_vslider( void )
{
  int vslide;

  if ((vslide = (int)((TLC * 1000L) / LC)) != VSLID)
    wind_set( WH, WF_VSLIDE, VSLID = vslide );
}

void set_allslider( void )
{
  int lines, count;

  count = lines = H / YFAC; if (LC > 0) { count += LC; set_vslider(); }
  wind_set( WH, WF_VSLSIZE, (int)((lines * 1000L) / count) );
  count = lines = W / XFAC; if (BC > 0) { count += BC; set_hslider(); }
  wind_set( WH, WF_HSLSIZE, (int)((lines * 1000L) / count) );
}

static void calc_border( void )
{
  DCLICK( tg, 0, 0, -2 ); *(long *)&par.xwin = *(long *)&X;
  wind_calc( WC_BORDER, W_KIND, X, Y, W, H, pipe+4,pipe+5,pipe+6,pipe+7 );
}

static void calc_work( void )
{
  wind_calc( WC_WORK,W_KIND, pipe[4],pipe[5],pipe[6],pipe[7],&X,&Y,&W,&H );
}

static void get_work( void )
{
  wind_get( WH, WF_WORKXYWH, &X, &Y, &W, &H );
}

static int get_pipe( int flag )
{
  static long oldwh;
  int ret;

  if (flag == WF_NEXTXYWH && oldwh == *(long *)&W) return 0;
  wind_get( tw, flag, pipe, pipe + 1, pipe + 2, pipe + 3 );
  oldwh = *(long *)(pipe + 2);
  if ((ret = pipe[2]) != 0) ret = pipe[3];
  pipe[2] += pipe[0] - 1; pipe[3] += pipe[1] - 1;
  return ret;
}

static void redraw( void )
{
  if (get_pipe( WF_FIRSTXYWH ))
  {
    graf_mouse( M_OFF, 0 );
    pipe[6] += pipe[4] - 1; pipe[7] += pipe[5] - 1;
    if (xdesk > pipe[4]) pipe[4] = xdesk;
    if (ydesk > pipe[5]) pipe[5] = ydesk;
    if ( xmax < pipe[6]) pipe[6] = xmax;
    if ( ymax < pipe[7]) pipe[7] = ymax;
    do
    { if (pipe[4] > pipe[0]) pipe[0] = pipe[4];
      if (pipe[5] > pipe[1]) pipe[1] = pipe[5];
      if (pipe[6] < pipe[2]) pipe[2] = pipe[6];
      if (pipe[7] < pipe[3]) pipe[3] = pipe[7];
      if (pipe[2] >= pipe[0] && pipe[3] >= pipe[1]) DRAW( tg, pipe );
    }
    while (get_pipe( WF_NEXTXYWH ));
    graf_mouse( M_ON, 0 );
} }

void full_redraw( void )
{
  *(long *)(pipe + 4) = *(long *)&X; *(long *)(pipe + 6) = *(long *)&W;
  redraw();
}

void new_redraw( void )
{
  *(long *)&LBC = 0; BC -= W / XFAC; LC -= H / YFAC;
  set_allslider(); full_redraw();
}

static void set_curr( void )
{
  calc_border();
  wind_set( tw, WF_CURRXYWH, pipe[4], pipe[5], pipe[6], pipe[7] );
  get_work();
}

void new_size( int flag )
{
  long oldxy, oldwh;

  oldxy = *(long *)&X; oldwh = *(long *)&W;
  if (flag) { BC += W / XFAC; LC += H / YFAC; calc_work(); }
  BC -= W /= XFAC; W *= XFAC; if (BC < LBC) if ((LBC = BC) < 0) LBC = 0;
  LC -= H /= YFAC; H *= YFAC; if (LC < TLC) if ((TLC = LC) < 0) TLC = 0;
  if (*(long *)&X != oldxy || *(long *)&W != oldwh)
    { set_name(); set_curr(); }
  set_allslider();
  if (flag == 0)
  {
    pipe[0] = WM_REDRAW;
    pipe[1] = appl_id;
    pipe[2] = 0;
    pipe[3] = WH;
    *(long *)(pipe + 4) = *(long *)&X;
    *(long *)(pipe + 6) = *(long *)&W; appl_write( appl_id, 16, pipe );
} }

static int create_window( void )
{
  wind_get( 0, WF_WORKXYWH, &xdesk, &ydesk, &wdesk, &hdesk );
  xmax = xdesk + wdesk - 1; ymax = ydesk + hdesk - 1;
  wind_calc( WC_WORK, W_KIND, xdesk - 1, ydesk, wdesk + 1, hdesk,
	     pipe + 4, pipe + 5, pipe + 6, pipe + 7 );
  {
    long n_wmax = pipe[6];
    par.xwin = (int)((n_wmax * (par.xwin-par.xmin)) / par.wmax) + pipe[4];
    par.wwin = (int)((n_wmax * par.wwin) / par.wmax);
  }
  { long n_hmax = pipe[7];
    par.ywin = (int)((n_hmax * (par.ywin-par.ymin)) / par.hmax) + pipe[5];
    par.hwin = (int)((n_hmax * par.hwin) / par.hmax);
  }
  *(long *)&par.xmin = *(long *)(pipe + 4);
  *(long *)&par.wmax = *(long *)(pipe + 6);
  return wind_create( W_KIND, xdesk - 1, ydesk, wdesk + 1, hdesk );
}

static void show_window( int x, int y, int w, int h )
{
  set_name(); calc_border();
  graf_growbox( x, y, w, h, pipe[4], pipe[5], pipe[6], pipe[7] );
  wind_open( tw, pipe[4], pipe[5], pipe[6], pipe[7] ); get_work();
  *(long *)&HSLID = 0; set_allslider();
  AVSendMessage( AV_ACCWINDOPEN, tw, 0 );
}

#pragma warn -par
int filebox( char *titel )
{
  static char File[17] = "*.*";
  int but[1];

  memswap( strrchr( Path, PATHSEP ) + 1, File, 17 );
#ifdef __TOS__
  if (global[0] >= 0x130) fsel_exinput( Path, File, but, titel );
  else
#endif
    fsel_input( Path, File, but );
  memswap( strrchr( Path, PATHSEP ) + 1, File, 17 );
  return *but;
}
#pragma warn +par

void open_window( int parent, int over, int x, int y, int w, int h,
		  char *file, char *key )
{
  int   len, flag, found;
  long  length;
  GUIDE	*pg;
  char	*buf, *p, titel[40];

  flag = 1;
  do
  { if (over == 0)
      if ((len = create_window()) >= 0) tw = len;
      else
      {
	if (parent <= 0) { form_alert( 1, no_win ); slct_close(); return; }
	if (form_alert( 1, "[3][Keine weiteren Fenster|vorhanden!"
			   "|Wollen Sie das|Ausgangsfenster|Åber"
			   "laden ?][öberladen|Abbrechen]" ) == 2)
	  { slct_close(); return; }
	over = 1;
      }
    if (parent > 0)
    {
      p = Path; strcpy( p, PATH ); buf = HIST; buf += (unsigned char)*buf;
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
	memcpy( buf + 2, &tg->s1, 10 ); buf += 12; *buf++ = 0;
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
	strcpy( titel, "1stGuide: Datei(en) " );
	strcat( titel, over ? "Åber" : "neu" );
	strcat( titel, parent == 0 ? "dumpen" : "laden" );
	if (filebox( titel ) == 0)
	  { if (over == 0) wind_delete( tw ); slct_close(); return; }
    } }
    graf_mouse( BUSYBEE, 0 );
    for (;;)
    {
      if (flag)
      {
        found = 1; p = strrchr( Path, PATHSEP ) + 1;
	if (slct_first( &mydta, 0x27 ))
	{
	  slct_close();
	  if (*p && (found = Findfirst( Path, &mydta, 0x27 )) != 0)
	    form_error( -found - 31 );
	  if (found)
	  {
	    graf_mouse( ARROW, 0 );
	    if (over == 0) wind_delete( tw ); slct_close(); return;
      } } }
      strcpy( p, mydta.d_fname ); length = mydta.d_length;
      if ((flag = slct_next( &mydta )) != 0) slct_close();
      if (found == 0 && parent <= 0) flag = Findnext( &mydta );
      pg = tg; tg = load_file( Path, length, parent );
      if (tg) break; tg = pg;
      if (flag)
      {
	graf_mouse( ARROW, 0 );
	if (over == 0) wind_delete( tw ); slct_close(); return;
    } }
    graf_mouse( ARROW, 0 );
    strcpy( PATH, Path ); *(long *)&LBC = 0; WH = tw;
    tg->actpd = Pgetpd( 0 );
    if (pg)
    {
      tg->next = pg->next; pg->next = tg; *(long *)&X = *(long *)&pg->x;
    }
    else { tg->next = tg; *(long *)&X = *(long *)&par.xwin; }
    if (parent > 0)
    {
      strcpy( WNAME, pg->name );
      memcpy( HIST, pg->hist, sizeof(HIST) ); *HIST = buf - pg->hist;
      if (over == -1)
      {
	*strrchr( WNAME, ':' ) = 0; memcpy( &tg->s1, buf + 2, 10 );
      }
      else
      {
	buf = WNAME; while (*buf++); --buf;
	if (buf[-1] == '"') { --buf; while (*--buf != '"'); --buf; }
	*buf++ = ':'; *buf++ = ' '; strcpy( buf, key );
    } }
    else
    {
      if (parent == 0) strlwr( PATH ); strcpy( WNAME, p ); *HIST = 0;
    }
    if (over)
    {
      *(long *)&W = *(long *)&pg->w;
      *(long *)&HSLID = *(long *)&pg->hslide;
      tg = pg; delete_guide(); set_name(); new_size( 0 );
    }
    else
    {
      if (pg) { X += wbox; Y += hbox; }
      if ((len = wbox << 2) > W) W = len;
      if ((len = hbox << 2) > H) H = len;
      if ((len = par.xmin + par.wmax - W) < X)
      {
	if (len < par.xmin) { len = par.xmin; W = par.wmax; }
	X = len;
      }
      X &= -8;
      if ((len = par.ymin + par.hmax - H) < Y)
      {
	if (len < par.ymin) { len = par.ymin; H = par.hmax; }
	Y = len;
      }
      BC -= W /= XFAC; W *= XFAC;
      LC -= H /= YFAC; H *= YFAC;
      show_window( x, y, w, h );
    }
    over = 0;
  }
  while (flag == 0);
}

static int rebuild_windows( void )
{
  GUIDE *hg;
  int   count, flag = 0;

  if (tg && WH == 0)
  {
    hg = tg; count = 0; do ++count; while ((hg = hg->next) != tg);
    do
    { if (tg->actpd != Pgetpd( tg )) delete_guide();
      else
      {
	if ((WH = tw = create_window()) < 0)
	{
	  form_alert( 1, no_win );
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
  wind_close( tw ); calc_border();
  graf_shrinkbox( 0, 0, 0, 0, pipe[4], pipe[5], pipe[6], pipe[7] );
  AVSendMessage( AV_ACCWINDCLOSED, tw, 0 );
  wind_delete( tw ); delete_guide();
}

static void copy( void )
{
  MFDB srcMFDB, desMFDB;

  srcMFDB.fd_addr = 0; desMFDB.fd_addr = 0;
  vro_cpyfm( handle, S_ONLY, pipe, &srcMFDB, &desMFDB );
}

static void upscroll( int delta )
{
  long ld;

  if ((TLC -= delta) < 0)
    { delta += TLC; TLC = 0; if (delta == 0) return; }
  graf_mouse( M_OFF, 0 ); set_vslider();
  if (get_pipe( WF_FIRSTXYWH ))
  {
    ld = (long)delta * YFAC;
    do
    { if (xdesk > pipe[0]) pipe[0] = xdesk;
      if (ydesk > pipe[1]) pipe[1] = ydesk;
      if ( xmax < pipe[2]) pipe[2] = xmax;
      if ( ymax < pipe[3]) pipe[3] = ymax;
      if (pipe[2] >= pipe[0] && (delta = pipe[3] - pipe[1]) >= 0)
      {
	if (ld <= delta)
	{
	  *(long *)(pipe + 4) = *(long *)pipe;
	  *(long *)(pipe + 6) = *(long *)(pipe + 2);
	  pipe[3] -= (int)ld; pipe[5] += (int)ld; copy();
	  pipe[3] = pipe[5] - 1;
	}
	DRAW( tg, pipe );
    } }
    while (get_pipe( WF_NEXTXYWH ));
  }
  graf_mouse( M_ON, 0 );
}

static void dnscroll( int delta )
{
  int  id;
  long ld;

  if ((id = LC - (TLC += delta)) < 0)
    { if ((TLC += id) < 0) TLC = 0; if ((delta += id) <= 0) return; }
  graf_mouse( M_OFF, 0 ); set_vslider();
  if (get_pipe( WF_FIRSTXYWH ))
  {
    ld = (long)delta * YFAC;
    do
    { if (xdesk > pipe[0]) pipe[0] = xdesk;
      if (ydesk > pipe[1]) pipe[1] = ydesk;
      if ( xmax < pipe[2]) pipe[2] = xmax;
      if ( ymax < pipe[3]) pipe[3] = ymax;
      if (pipe[2] >= pipe[0] && (delta = pipe[3] - pipe[1]) >= 0)
      {
	if (ld <= delta)
	{
	  *(long *)(pipe + 4) = *(long *)pipe;
	  *(long *)(pipe + 6) = *(long *)(pipe + 2);
	  pipe[1] += (int)ld; pipe[7] -= (int)ld; copy();
	  pipe[1] = pipe[7] + 1;
	}
	DRAW( tg, pipe );
    } }
    while (get_pipe( WF_NEXTXYWH ));
  }
  graf_mouse( M_ON, 0 );
}

static void lfscroll( int delta )
{
  long ld;

  if ((LBC -= delta) < 0)
    { delta += LBC; LBC = 0; if (delta == 0) return; }
  graf_mouse( M_OFF, 0 ); set_hslider();
  if (get_pipe( WF_FIRSTXYWH ))
  {
    ld = (long)delta * XFAC;
    do
    { if (xdesk > pipe[0]) pipe[0] = xdesk;
      if (ydesk > pipe[1]) pipe[1] = ydesk;
      if ( xmax < pipe[2]) pipe[2] = xmax;
      if ( ymax < pipe[3]) pipe[3] = ymax;
      if ((delta = pipe[2] - pipe[0]) >= 0 && pipe[3] >= pipe[1])
      {
	if (ld <= delta)
	{
	  *(long *)(pipe + 4) = *(long *)pipe;
	  *(long *)(pipe + 6) = *(long *)(pipe + 2);
	  pipe[2] -= (int)ld; pipe[4] += (int)ld; copy();
	  pipe[2] = pipe[4] - 1;
	}
	DRAW( tg, pipe );
    } }
    while (get_pipe( WF_NEXTXYWH ));
  }
  graf_mouse( M_ON, 0 );
}

static void rtscroll( int delta )
{
  int  id;
  long ld;

  if ((id = BC - (LBC += delta)) < 0)
    { if ((LBC += id) < 0) LBC = 0; if ((delta += id) <= 0) return; }
  graf_mouse( M_OFF, 0 ); set_hslider();
  if (get_pipe( WF_FIRSTXYWH ))
  {
    ld = (long)delta * XFAC;
    do
    { if (xdesk > pipe[0]) pipe[0] = xdesk;
      if (ydesk > pipe[1]) pipe[1] = ydesk;
      if ( xmax < pipe[2]) pipe[2] = xmax;
      if ( ymax < pipe[3]) pipe[3] = ymax;
      if ((delta = pipe[2] - pipe[0]) >= 0 && pipe[3] >= pipe[1])
      {
	if (ld <= delta)
	{
	  *(long *)(pipe + 4) = *(long *)pipe;
	  *(long *)(pipe + 6) = *(long *)(pipe + 2);
	  pipe[0] += (int)ld; pipe[6] -= (int)ld; copy();
	  pipe[0] = pipe[6] + 1;
	}
	DRAW( tg, pipe );
    } }
    while (get_pipe( WF_NEXTXYWH ));
  }
  graf_mouse( M_ON, 0 );
}

void goto_line( int line )
{
  if ((line -= TLC) < 0) upscroll( -line );
  if (line > 0) dnscroll( line );
}

static void goto_col( int col )
{
  if ((col -= LBC) < 0) lfscroll( -col );
  if (col > 0) rtscroll( col );
}

static void full_window( void )
{
  wind_get( tw, WF_CURRXYWH, pipe, pipe + 1, pipe + 2, pipe + 3 );
  wind_get( tw, WF_FULLXYWH, pipe + 4, pipe + 5, pipe + 6, pipe + 7 );
  if (pipe[0] - pipe[4] < 8 && pipe[1] == pipe[5] &&
      pipe[6] - pipe[2] < XFAC && pipe[7] - pipe[3] < YFAC)
  {
    wind_get( tw, WF_PREVXYWH, pipe + 4, pipe + 5, pipe + 6, pipe + 7 );
    graf_shrinkbox( pipe[4], pipe[5], pipe[6], pipe[7],
		    pipe[0], pipe[1], pipe[2], pipe[3] );
  }
  else graf_growbox( pipe[0], pipe[1], pipe[2], pipe[3],
		     pipe[4], pipe[5], pipe[6], pipe[7] );
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
  strcpy( message, "[1][Um Dateien auf ein GerÑt|"
		   "ausgeben zu kînnen, mu· ein|"
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
    if (ohandle == 0)
    {
      i = 102; do work_in[i] = 1; while (--i > 0);
      work_in[0] = par.out_handle; work_in[10] = 2; work_in[11] = 255;
      v_opnwk( work_in, &ohandle, work_out );
      xpixel = work_out[0] + 1; ypixel = work_out[1] + 1;
      out_width = work_out[3]; out_height = work_out[4];
      if (ohandle <= 0)
      {
	form_alert( 1, "[1][1STGUIDE-Warnung:"
			"|Fehler beim ôffnen des"
			"|AusgabegerÑtes."
			"|Ausgabe wird abgebrochen"
			"|und Spooler-Puffer geleert.][ Oh jeh ]" );
	ohandle = 0; *spool_buf = 0; return;
    } }
    ekind = MU_MESAG | MU_KEYBD | MU_BUTTON | MU_TIMER;
} }

static int rmouse( void )
{
  int mbut[1], dummy[1];

  graf_mkstate( dummy, dummy, mbut, &ks ); return *mbut & 2;
}

static void spool_cmd( char *cmd )
{
  char *p;

  if (vq_gdos() == 0) { form_alert( 1, no_gdos ); return; }
  if (cmd && *cmd)
    do
    { if ((p = strchr( cmd, ' ' )) != 0) *p++ = 0;
      get_spool( cmd );
    }
    while ((cmd = p) != 0 && *cmd);
  else if (rmouse()) config( ks ); else get_spool( "" );
  check_spool();
}

static void scrp_copy( void )
{
  char *fbuf, sbuf[4];
  long blen, count;
  int  rfh, wfh;

  if (set_scrp() == 0) return; graf_mouse( BUSYBEE, 0 );
  if ((rfh = Fopen( PATH, 0 )) < 0)
  {
    wfh = rfh; graf_mouse( ARROW, 0 );
    form_error( -wfh - 31 ); return;
  }
  strcat( Path, strrchr( PATH, '.' ) );
  if ((wfh = Fcreate( Path, 0 )) < 0)
  {
    Fclose( rfh ); graf_mouse( ARROW, 0 );
    form_error( -wfh - 31 ); return;
  }
  blen = Fseek( 0, rfh, 2 ); Fseek( 0, rfh, 0 );
  if ((fbuf = Malloc( blen )) == 0)
    if ((blen = Mavail()) > 4) fbuf = Malloc( blen );
    else { blen = 4; fbuf = sbuf; }
  while ((count = Fread( rfh, blen, fbuf )) > 0 &&
	 Fwrite( wfh, count, fbuf ) == count);
  if (fbuf != sbuf) Mfree( fbuf );
  Fclose( wfh ); Fclose( rfh ); graf_mouse( ARROW, 0 );
}

static void get_entry( char *cmd, int over )
{
  if ((ks & 6) == 6 && *cmd == 0) cmd = par.indexfile;
  open_window( (ks & 12) - 12, over, 0, 0, 0, 0, cmd, 0 );
}

static void hndl_key( void )
{
  wind_get( 0, WF_TOP, &tw ); if (find_guide() == 0) return;
  switch (key)
  {
    case CUR_UP:    upscroll( 1 ); break;
    case CUR_DOWN:  dnscroll( 1 ); break;
    case SHFT_CU:   upscroll( H / YFAC - 1 ); break;
    case SHFT_CD:   dnscroll( H / YFAC - 1 ); break;
    case CUR_LEFT:  lfscroll( 1 ); break;
    case CUR_RIGHT: rtscroll( 1 ); break;
    case SHFT_CL:   lfscroll( W / XFAC - 1 ); break;
    case SHFT_CR:   rtscroll( W / XFAC - 1 ); break;
    case HOME:      goto_line( 0 ); break;
    case SHFT_HOME: goto_line( LC ); break;
    case INSERT:    goto_col( 0 ); break;
    case SHFT_INS:  goto_col( BC ); break;
    case CNTRL_O:   get_entry( "", 0 ); break;
    case CNTRL_D:   get_entry( "", 1 ); break;
    case CNTRL_U:   close_window(); break;
    case CNTRL_Q:   do { tw = WH; close_window(); } while (tg);
		    break;
    case CNTRL_F:   full_window(); new_size( 1 ); break;
    case CNTRL_P:   (((ks & 10) == 10) ^ par.spoolflag ?
		    print_file : spool_cmd)( PATH );
		    break;
    case CNTRL_A:   config( ks ); break;
    case UNDO:
    case ESC:	    if (*HIST) open_window( 1, -1, 0, 0, 0, 0, 0, 0 );
		    break;
    case CNTRL_W:   if ((VA_Flags & (1 << 6)) == 0 && tg->next != tg)
		      { wind_set( tg->next->handle, WF_TOP ); return; }
    default:	    if (KEY( tg, key, ks ))
		      if (key == CNTRL_C) scrp_copy();
		      else AVSendMessage( AV_SENDKEY, ks, key );
} }

static void activate( char *cmd, int over )
{
  int  flag;
  char *p;

  if (pipe[3] < 0) over = 0;
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

static void find_index( char *index )
{
  int  fh;
  char *pbuf, buf[128];
  FBUF f;

  if (pipe[1] != starter_id)
    { starter_id = pipe[1]; AVSendMessage( AV_PROTOKOLL, 2, 0 ); }
  rebuild_windows();
  if (*index == 0)
    { open_window( -1, 0, 0, 0, 0, 0, par.indexfile, 0 ); return; }
  graf_mouse( BUSYBEE, 0 );
  if ((fh = Fopen( par.indexfile, 0 )) < 0)
  {
    graf_mouse( ARROW, 0 ); form_error( -fh - 31 ); return;
  }
  f.handle = fh; Fbufopen( &f );
  for (;;)
    switch (Fgetc( &f ))
    {
      case  -1: Fbufclose( &f ); graf_mouse( ARROW, 0 );
		strcpy( buf, "[1][1st Guide:| |\"" );
		strncat( buf, index, 28 );
		strcat( buf, "\"| |nicht gefunden][  OK  ]" );
		form_alert( 1, buf ); return;
      case (unsigned char)LINK:
		pbuf = buf; while ((*pbuf++ = Fgetc( &f )) != LINK);
		*--pbuf = 0; fh = stricmp( buf, index );
		pbuf = buf; while ((*pbuf++ = Fgetc( &f )) != LINK);
		if (fh == 0)
		{
		  *--pbuf = 0; Fbufclose( &f );
		  graf_mouse( ARROW, 0 ); strcpy( Path, par.indexfile );
		  get_entry( buf, 0 ); return;
}   }		}

static void invalidate( void )
{
  GUIDE *hg;
  int   count;

  if (tg == 0) return;
  hg = tg; count = 0; do ++count; while ((hg = hg->next) != tg);
  do
  { if (tg->actpd != Pgetpd( tg )) delete_guide();
    else { WH = 0; tg = tg->next; }
  }
  while (--count);
}

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
  tw = pipe[3]; find_guide();
  switch (*pipe)
  {
    case WM_ARROWED: switch (pipe[4])
		     {
			case WA_UPLINE: upscroll( 1 ); break;
			case WA_DNLINE: dnscroll( 1 ); break;
			case WA_UPPAGE: upscroll( H / YFAC - 1 ); break;
			case WA_DNPAGE: dnscroll( H / YFAC - 1 ); break;
			case WA_LFLINE: lfscroll( 1 ); break;
			case WA_RTLINE: rtscroll( 1 ); break;
			case WA_LFPAGE: lfscroll( W / XFAC - 1 ); break;
			case WA_RTPAGE: rtscroll( W / XFAC - 1 );
		     }
		     break;
    case WM_REDRAW:  redraw(); break;
    case WM_CLOSED:  if (*HIST == 0 || par.overflag == 0) close_window();
		     else open_window( 1, -1, 0, 0, 0, 0, 0, 0 );
		     break;
    case WM_TOPPED:
    case WM_NEWTOP:  wind_set( tw, WF_TOP ); break;
    case WM_MOVED:   calc_work(); X = (X + 3) & -8; set_curr(); break;
    case WM_FULLED:  full_window();
    case WM_SIZED:   new_size( 1 ); break;
#pragma warn -sig
    case WM_VSLID:   goto_line( ((long)LC * pipe[4] + 500) / 1000 ); break;
    case WM_HSLID:   goto_col( ((long)BC * pipe[4] + 500) / 1000 ); break;
#pragma warn +sig
    case AC_CLOSE:   starter_id = -1; VA_Flags = 0; invalidate(); break;
#ifdef __TOS__
    case AP_DRAGDROP: rec_ddmsg( (char *)(pipe + 7) ); break;
#endif
    case AC_SPOOL:   spool_cmd( *(char **)(pipe + 4) ); break;
    case AC_HELP:    find_index( *(char **)(pipe + 3) ); break;
    case AC_OPEN:    *(char **)(pipe + 4) = 0;
    case SCRP_OPEN:  pipe[3] = pipe[5]; pipe[1] = starter_id;
    case VA_START:   if (((ks & 10) == 10) ^ par.outdef)
		       spool_cmd( *(char **)(pipe + 3) );
		     else activate( *(char **)(pipe + 3), 0 );
		     break;
    case VA_DRAGACCWIND: activate( *(char **)(pipe + 6), 1 ); break;
    case VA_PROTOSTATUS: strncpy( VA_Name, *(char **)(pipe + 6), 8 );
			 VA_Flags = pipe[3]; break;
    case VA_FILEFONT: tw = aw;
		      if (find_guide()) DCLICK( tg, pipe[3], pipe[4], -1 );
} }

#pragma warn -sig
static void online_scroll( int flag )
{
  int base, len, mx, my, mbut;

  if (flag)
  {
    if (LC <= 0) return;
    len = H - ((hbox - 2) << 1); base = H / YFAC;
    base = (base * (long)len) / (LC + base); if (hbox > base) base = hbox;
    len -= base; base = Y + hbox - 3 + (base >> 1);
  }
  else
  {
    if (BC <= 0) return;
    len = W - ((wbox - 2) << 1); base = W / XFAC;
    base = (base * (long)len) / (BC + base); if (wbox > base) base = wbox;
    len -= base; base = X + wbox - 3 + (base >> 1);
  }
  graf_mouse( FLAT_HAND, 0 ); wind_update( BEG_MCTRL );
  graf_mkstate( &mx, &my, &mbut, pipe ); mbut ^= 1;
  do
    if (flag) { if ((my -= base) < 0) my = 0; if (len < my) my = len;
		goto_line( (my * (long)LC) / len );
	      }
    else      { if ((mx -= base) < 0) mx = 0; if (len < mx) mx = len;
		goto_col( (mx * (long)BC) / len );
	      }
  while ((evnt_multi( MU_BUTTON | MU_M1, 1, 1, mbut, 1, mx, my, 1, 1,
		      0, 0, 0, 0, 0, 0,0,0, &mx, &my, pipe,pipe,pipe,pipe )
	  & MU_BUTTON) == 0);
  wind_update( END_MCTRL ); graf_mouse( ARROW, 0 );
  if (mbut) evnt_button( 1, 1, 0, pipe, pipe, pipe, pipe );
}
#pragma warn +sig

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
  if (event & MU_BUTTON && (tw = wind_find( mx, my )) > 0 && find_guide())
  {
    if (mclicks == 1)
    {
	   if (mx >= X + W) online_scroll( 1 );
      else if (my >= Y + H) online_scroll( 0 );
      else if (mx < X + wbox && my < Y) close_window();
      else SCLICK( tg, mx, my );
    }
    else if (mx - X >= 0 && mx - X < W && my - Y >= 0 && my - Y < H)
	   DCLICK( tg, mx, my, ((ks & 15) != 0) ^ par.overflag );
  }
  if (event & MU_TIMER)
  {
    spool( ks );
    if (*spool_buf == 0)
    {
      v_clswk( ohandle ); ohandle = 0; ekind &= ~MU_TIMER;
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
  vsf_color( handle, 0 );
  Path[0] = 'A' + (char)Dgetdrv(); Path[1] = ':'; Dgetpath( Path + 2, 0 );
  strcat( Path, "\\" ); strcpy( StartPath, Path ); *spool_buf = 0; tg = 0;

#if DESKACC
  menu_register( appl_id, "  1st Guide" );
  load_fonts( 1 ); for (;;) hndl_event();
#else
#ifdef __TOS__
  if (_app == 0 || global[0] >= 0x400)
    menu_register( appl_id, "  1st Guide" );
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
  else if (vq_gdos() == 0) form_alert( 1, no_gdos );
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