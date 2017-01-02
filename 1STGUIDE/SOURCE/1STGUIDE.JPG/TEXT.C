#include <aes.h>
#include <vdi.h>
#include <gemdefs.h>
#include <osbind.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <vaproto.h>
#include <scancode.h>
#include <errno.h>

typedef struct guide
{
  int	handle, row;
  long	link;
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

  char	*text;
  int   font, point, line, effect, dist;
  char  flag, null;
  char  buf[];			/* Textpuffer.				*/
}
GUIDE;

#include "1stguide.h"
#include "util.h"

FROM( util ) IMPORT jmp_buf setjmp_buffer;

#define TABU	9
#define Esc	27

#define TXT_THICKENED	1
#define TXT_LIGHT	2
#define TXT_SKEWED	4
#define TXT_UNDERLINED	8
#define TXT_SUPERSCRIPT 0x10
#define TXT_SUBSCRIPT	0x20

static int wchar, hchar, dist, oeffect = 0, effects = 0;
static struct { char space[104], text[408], end[]; } read_buf;
static char filebuf[128], keybuf[128],
	    fontstring[] = "  XXXXXXXX-Font ^E",
	    zeilstring[] = "---XXXXX Zeilen ---",
	    savestring[] = "  >> OUT-Datei  ^X";

#pragma warn -rpt
static OBJECT popup[] =
{
   0,  1,  8, G_BOX,	NONE,	SHADOWED, 0xFF1100L,		0, 0, 19,0,
   2, -1, -1, G_STRING, NONE,	DISABLED, "--- System-Font ---",0, 0, 19,1,
   3, -1, -1, G_STRING, NONE,	NORMAL,   "  Norm  (10 Pt) ^N", 0, 0, 19,0,
   4, -1, -1, G_STRING, NONE,	NORMAL,   "  Mittel (9 Pt) ^M", 0, 0, 19,0,
   5, -1, -1, G_STRING, NONE,	NORMAL,   "  Klein  (8 Pt) ^K", 0, 0, 19,0,
   6, -1, -1, G_STRING, NONE,	DISABLED, "-------------------",0, 0, 19,1,
   7, -1, -1, G_STRING, NONE,	NORMAL,   fontstring,		0, 0, 19,1,
   8, -1, -1, G_STRING, NONE,	DISABLED, zeilstring,		0, 0, 19,1,
   0, -1, -1, G_STRING, LASTOB, NORMAL,   savestring,		0, 0, 19,1
};
#pragma warn +rpt

#if defined(__TOS__) && defined(__TURBOC__)
typedef void (*DIV_FUNC)( int numer, int denom, div_t *qr );
#define mydiv(a,b,c) (*(DIV_FUNC)((char *)div + 4))(a,b,c)
#else
#define mydiv(a,b,c) *c = div(a,b)
#endif

static int text_width( char *string )
{
  int ret, extent[8];

  vqt_extent( handle, ++string, extent );
  ret = 0; while (*string++) ret += wchar;
  dist += ret;
  if (extent[2] < ret) ret = extent[2];
  return ret;
}

static int text_just( int x, int y, char *string )
{
  int ret, extent[8];
  char *p;

  vqt_extent( handle, ++string, extent );
  p = string; ret = 0; while (*p++) ret += wchar;
  dist += ret;
  if (extent[2] <= ret)
    { ret = extent[2]; v_gtext( handle, x, y, string ); return ret; }
  if ((effects & TXT_SKEWED) == 0)
    { v_justified( handle, x, y, string, ret, 1, 1 ); return ret; }
  if (extent[2] <= ret + wchar)
    { v_gtext( handle, x, y, string ); return ret; }
  v_justified( handle, x, y, string, ret + wchar, 1, 1 ); return ret;
}

static int set_fontpoint( int font, int point )
{
  int delta, extent[8];

  par.font  = vst_font( handle, font );
  par.point = vst_point( handle, point, extent, extent, &dist, &hchar );
  if (effects) vst_effects( handle, effects = 0 );
  vqt_extent( handle, "W", extent ); wchar = extent[2];
  vqt_extent( handle, "l", extent );
  if ((delta = wchar - extent[2]) < 0) delta = -delta;
  dist += delta; return delta;
}

static void new_fontpoint( GUIDE *tg, int font, int point )
{
  if (tg->font == font && tg->point == point) return;
  set_fontpoint( font, point );
  *(long *)&tg->font = *(long *)&par.font;
  if (tg->link < 0) tg->dist = dist;
  else { tg->dist /= tg->xfac; tg->dist *= wchar; }
  tg->bc += tg->w / tg->xfac; tg->xfac = wchar;
  tg->lc += tg->h / tg->yfac; tg->yfac = hchar;
  new_size( 1 );
}

static int draw_popup( OBJECT *tree, int i, int depth,
		       int xc, int yc, int wc, int hc )
{
  int dummy[1], hcell[1];

  tree[4].ob_flags = tree[3].ob_flags = tree[2].ob_flags = HIDETREE;
  objc_draw( tree, i, depth, xc, yc, wc, hc );
  if (effects) vst_effects( handle, effects = 0 );
  if (par.font != 1) vst_font( handle, 1 ); i = 2;
  do
  { vst_point( handle, 12 - i, dummy, dummy, dummy, hcell );
    tree[i].ob_flags = NONE;
    v_gtext( handle, tree->ob_x,
	     tree->ob_y + tree[i].ob_y + tree[i].ob_height - *hcell,
	     tree[i].ob_spec.free_string );
  }
  while (++i < 5);
  if (par.font != 1) vst_font( handle, par.font );
  vst_point( handle, par.point, dummy, dummy, dummy, dummy );
  return 0;
}

static int put_effect( int effect, int new, FBUF *fp )
{
  if (effect >= 0)
  {
    effect ^= new;
    if (new & TXT_THICKENED)
    {
      FPUTC(fp, 18); FPUTC(fp, effect & TXT_THICKENED ? '0' : '1');
    }
    if (new & TXT_LIGHT)
    {
      FPUTC(fp, 18); FPUTC(fp, effect & TXT_LIGHT ? 'E' : 'F');
    }
    if (new & TXT_SKEWED)
    {
      FPUTC(fp, 18); FPUTC(fp, effect & TXT_SKEWED ? '2' : '3');
    }
    if (new & TXT_UNDERLINED)
    {
      FPUTC(fp, 18); FPUTC(fp, effect & TXT_UNDERLINED ? '4' : '5');
    }
    if (new & TXT_SUPERSCRIPT)
    {
      FPUTC(fp, 18); FPUTC(fp, effect & TXT_SUPERSCRIPT ? '6' : '7');
    }
    if (new & TXT_SUBSCRIPT)
    {
      FPUTC(fp, 18); FPUTC(fp, effect & TXT_SUBSCRIPT ? '8' : '9');
  } }
  return effect;
}

static void write_out( GUIDE *tg, char *ext, int effect )
{
  int  fh, lines, flag;
  char c, *p;
  FBUF f;

  if ((p = strrchr( strrchr( Path, PATHSEP ), '.' )) != 0) *p = 0;
  graf_mouse( BUSYBEE, 0 );
  if ((fh = Fcreate( strcat( Path, ext ), 0 )) < 0)
  {
    graf_mouse( ARROW, 0 ); form_error( -31 - fh ); return;
  }
  f.handle = fh; Fbufcreate( &f );
  p = tg->buf; lines = tg->lc + tg->h / tg->yfac; flag = 2;
  if (setjmp( setjmp_buffer ) == 0)
    for (;;)
      if ((c = *p++) == 0)
      {
	FPUTC(&f, 13); FPUTC(&f, 10);
	if (--lines == 0) break;
	if (flag != 2)
	{
	  effect = put_effect( effect, effect, &f ); flag = 2;
      } }
      else if (c == Esc)
	effect = put_effect( effect, *p++ & 0x3F, &f );
      else if (c == LINK)
	switch (flag)
	{
	  case 2: effect = put_effect( effect, effect ^ 9, &f );
	  case 1: --flag; break;
	  case 0: effect = put_effect( effect, effect, &f );
		  flag = 2;
	}
      else if (flag) FPUTC(&f, c);
  Fbufwrite( &f ); Fbufclose( &f ); graf_mouse( ARROW, 0 );
}

static void write_dump( GUIDE *tg )
{
  int  fh, b, c;
  long count, val;
  char *p, *end, buf[8];
  FBUF f;

  if ((p = strrchr( strrchr( Path, PATHSEP ), '.' )) != 0) *p = 0;
  graf_mouse( BUSYBEE, 0 ); struplo( Path );
  if ((fh = Fcreate( strcat( Path, ".DMP" ), 0 )) < 0)
  {
    graf_mouse( ARROW, 0 ); form_error( -31 - fh ); return;
  }
  f.handle = fh; Fbufcreate( &f );
  p = tg->buf; end = tg->text; count = 0;
  if (setjmp( setjmp_buffer ) == 0)
    do
    { b = 7; val = count;
      do
      { c = (int)val & 15; c += '0'; if (c > '9') c += 'A' - '9' - 1;
	buf[b] = c; val >>= 4;
      }
      while (--b >= 0);
      do FPUTC(&f, buf[++b]); while (b < 7);
      FPUTC(&f, ':'); FPUTC(&f, ' '); count += val = 16;
      for (;;)
      {
	FPUTC(&f, ' '); if (--val < 0) break;
	if (p < end)
	{
	  b = (unsigned char)*p++;
	  c = b >> 4; c += '0'; if (c > '9') c += 'A' - '9' - 1;
	  b &= 15; b += '0'; if (b > '9') b += 'A' - '9' - 1;
	}
	else { ++p; c = b = ' '; }
	FPUTC(&f, c); FPUTC(&f, b);
      }
      FPUTC(&f, ' '); p -= val = 16;
      do
      { c = (unsigned char)*p++;
	if (c < ' ') c = '.';
	FPUTC(&f, c);
      }
      while (p < end && --val);
      FPUTC(&f, 13); FPUTC(&f, 10);
    }
    while (p < end);
  Fbufwrite( &f ); Fbufclose( &f ); graf_mouse( ARROW, 0 );
}

static void rect_invert( int x1, int y1, int x2, int y2 )
{
  int pxy[4];

  pxy[0] = x1; pxy[1] = y1; pxy[2] = x2; pxy[3] = y2;
  vswr_mode( handle, MD_XOR ); vsf_interior( handle, FIS_SOLID );
  vr_recfl( handle, pxy );
  vsf_interior( handle, FIS_HOLLOW ); vswr_mode( handle, MD_REPLACE );
}

static void draw_text( GUIDE *tg, int *clip )
{
  int  x, y, fx, flag;
  char *text, *next;

  if (*(long *)&tg->font != *(long *)&par.font)
    set_fontpoint( tg->font, tg->point );
  vr_recfl( handle, clip ); y = clip[1];
  if (tg->lc < 0)
  {
    flag = tg->y + tg->h - 1 + tg->lc * tg->yfac;
    if (flag < clip[3]) { if (flag < y) return; clip[3] = flag; }
  }
  vs_clip( handle, 1, clip ); mydiv( y - tg->y, tg->yfac, (div_t *)clip );
  text = tg->text; y -= ((div_t *)clip)->rem;
  flag = tg->tlc + ((div_t *)clip)->quot;
  if (flag -= tg->line)
  {
    tg->line += flag;
    if (tg->flag == 0)
    {
      char c, e = Esc;

      if (flag < 0)
	do
	  for (;;)
	  {
	    if ((c = *--text) == 0) break;
	    if (c == e)
	    {
	      x = 15 & Esc;
	      if (*--text != c) { x = 15; (char)x &= (++text)[1]; }
	      tg->effect ^= x;
	  } }
	while (++flag < 0);
      else
	do
	  for (;;)
	  {
	    if ((c = *++text) == 0) break;
	    if (c == e) { x = 15; (char)x &= *++text; tg->effect ^= x; }
	  }
	while (--flag > 0);
    }
    else
      if (flag < 0)
	do { while (*--text); } while (++flag < 0);
      else
	do { while (*++text); } while (--flag > 0);
  }
  if (tg->effect != effects) vst_effects( handle, effects = tg->effect );
  tg->text = next = text; x = fx = tg->x - tg->lbc * tg->xfac; dist = 0;
  for (;;)
    switch ((unsigned char)*++next)
    {
      case 0:	 if (effects) text_just( x, y, text );
		 else v_gtext( handle, x, y, text + 1 );
		 if ((y += tg->yfac) <= clip[3])
		 {
		   x = fx; dist = 0; flag = 0; text = next; break;
		 }
		 vs_clip( handle, 0, clip ); return;
      case TABU: *next = 0; text_just( x, y, text ); *next = TABU;
		 x = dist; x += tg->dist; x -= x % tg->dist;
		 dist = x; x += fx; text = next; break;
      case Esc:  *next = 0; x += text_just( x, y, text ); *next++ = Esc;
		 vst_effects( handle, effects ^= *next & 15 );
		 text = next; break;
      case (unsigned char)LINK:
	switch (flag)
	{
	  case 0: *next = 0; x += text_just( x, y, text ); *next = LINK;
		  flag = 1; text = next; break;
	  case 1: vst_effects( handle, 9 );
		  *next = 0; x += flag = text_just(x,y,text); *next = LINK;
		  vst_effects( handle, effects );
		  if (next == (char *)tg + tg->link)
		    rect_invert( x - flag, y, x - 1, y + tg->yfac - 1 );
		  flag = 4;
	  case 2: flag -= 2; text = next;
}   }   }

static void draw_dump( GUIDE *tg, int *clip )
{
  int   e, f, cflag, *pintin;
  long  count, val;
  char  *pbyte;
  div_t xqr, yqr;

  if (*(long *)&tg->font != *(long *)&par.font)
    set_fontpoint( tg->font, tg->point );
  e = clip[2]; f = clip[3];
  if (tg->bc < 0)
    { int x = tg->x - 1 + 77 * tg->xfac; if (x < e) e = x; }
  if (tg->lc < 0)
    { int y = tg->y + tg->h - 1 + tg->lc * tg->yfac; if (y < f) f = y; }
  if (e < clip[2] || f < clip[3] || tg->xfac < tg->dist)
  {
    vr_recfl( handle, clip ); clip[2] = e; clip[3] = f;
    if (e < clip[0] || f < clip[1]) return;
  }
  if (effects) vst_effects( handle, effects = 0 );
  cflag = tg->dist + tg->yfac - 2;
  mydiv( clip[2] - tg->x, tg->xfac, &xqr ); e = xqr.quot+1; cflag-=xqr.rem;
  mydiv( clip[0] - tg->x, tg->xfac, &xqr ); e -= xqr.quot; cflag +=xqr.rem;
  mydiv( clip[3] - tg->y, tg->yfac, &yqr ); f = yqr.quot; cflag -= yqr.rem;
  mydiv( clip[1] - tg->y, tg->yfac, &yqr ); f -= yqr.quot;
  if (cflag += yqr.rem) vs_clip( handle, 1, clip );
  pbyte = tg->buf; pbyte += (long)(tg->tlc + yqr.quot) << 4;
  ptsin[1] = clip[1] - yqr.rem;
  if (tg->xfac < tg->dist) { ptsin[0] = tg->x - tg->lbc * tg->xfac; e=76; }
  else { ptsin[0] = clip[0] - xqr.rem; vdipb[1] += tg->lbc + xqr.quot; }
  do
  { pintin = intin + 77;
    *--pintin = ' '; val = 15; count = tg->text - pbyte;
    while (val >= count) { *--pintin = ' '; --val; }
    do *--pintin = (unsigned char)pbyte[val]; while (--val >= 0);
    *--pintin = ' '; val = 15;
    while (val >= count)
      { int c = ' '; *--pintin = c; *--pintin = c; *--pintin = c; --val; }
    do
    { int b, c; c = (unsigned char)pbyte[val];
      *--pintin = ' ';
      b = c & 15; b += '0'; if (b > '9') b += 'A' - '9' - 1;
      *--pintin = b;
      c >>= 4; c += '0'; if (c > '9') c += 'A' - '9' - 1;
      *--pintin = c;
    }
    while (--val >= 0);
    *--pintin = ' '; *--pintin = ' ';
    *--pintin = ':'; val = pbyte - tg->buf;
    do
    { int c; c = (int)val & 15; c += '0'; if (c > '9') c += 'A' - '9' - 1;
      *--pintin = c; val >>= 4;
    }
    while (pintin > intin);
    pbyte += 16; VDI( 8, 1, e, handle ); ptsin[1] += tg->yfac;
  }
  while (--f >= 0);
  vdipb[1] = pintin; if (cflag) vs_clip( handle, 0, clip );
}

static void free_text() { load_fonts( 0 ); }

static void invert( int wh, int x, int y, int w, int h )
{
  int pxy[8];

  if (xdesk > (pxy[4] = x - w    )) pxy[4] = xdesk;
  if (ydesk > (pxy[5] = y        )) pxy[5] = ydesk;
  if ( xmax < (pxy[6] = x - 1    )) pxy[6] = xmax;
  if ( ymax < (pxy[7] = y + h - 1)) pxy[7] = ymax;
  wind_get( wh, WF_FIRSTXYWH, pxy, pxy + 1, pxy + 2, pxy + 3 );
  while (pxy[2] && pxy[3])
  {
    pxy[2] += pxy[0] - 1; pxy[3] += pxy[1] - 1;
    if (pxy[4] > pxy[0]) pxy[0] = pxy[4];
    if (pxy[5] > pxy[1]) pxy[1] = pxy[5];
    if (pxy[6] < pxy[2]) pxy[2] = pxy[6];
    if (pxy[7] < pxy[3]) pxy[3] = pxy[7];
    if (pxy[0] <= pxy[2] && pxy[1] <= pxy[3]) vr_recfl( handle, pxy );
    wind_get( wh, WF_NEXTXYWH, pxy, pxy + 1, pxy + 2, pxy + 3 );
} }

static void draw_link( GUIDE *tg )
{
  char *text, *next;
  int  x, fx, flag;

  text = tg->text; flag = tg->row;
  if (flag -= tg->line)
  {
    tg->line += flag;
    if (tg->flag == 0)
    {
      char c, e = Esc;

      if (flag < 0)
	do
	  for (;;)
	  {
	    if ((c = *--text) == 0) break;
	    if (c == e)
	    {
	      x = 15 & Esc;
	      if (*--text != c) { x = 15; (char)x &= (++text)[1]; }
	      tg->effect ^= x;
	  } }
	while (++flag < 0);
      else
	do
	  for (;;)
	  {
	    if ((c = *++text) == 0) break;
	    if (c == e) { x = 15; (char)x &= *++text; tg->effect ^= x; }
	  }
	while (--flag > 0);
    }
    else
      if (flag < 0)
	do { while (*--text); } while (++flag < 0);
      else
	do { while (*++text); } while (--flag > 0);
  }
  if (tg->effect != effects) vst_effects( handle, effects = tg->effect );
  tg->text = next = text; fx = x = tg->x - tg->lbc * wchar; dist = 0;
  for (;;)
    switch ((unsigned char)*++next)
    {
      case TABU: x = dist += (int)(next - text - 1) * wchar + tg->dist;
		 x -= x % tg->dist; dist = x; x += fx; text = next; break;
      case Esc:  *next = 0; x += text_width( text ); *next++ = Esc;
		 vst_effects( handle, effects ^= *next & 15 );
		 text = next; break;
      case (unsigned char)LINK:
	switch (flag)
	{
	  case 0: *next = 0; x += text_width( text ); *next = LINK;
		  flag = 1; text = next; break;
	  case 1: vst_effects( handle, 9 );
		  *next = 0; x += flag = text_width( text ); *next = LINK;
		  vst_effects( handle, effects );
		  if (next == (char *)tg + tg->link)
		  {
		    invert( tg->handle, x,
			    tg->y + (tg->row - tg->tlc) * hchar,
			    flag, hchar );
		    return;
		  }
		  flag = 4;
	  case 2: flag -= 2; text = next;
}   }   }

static void set_newlink( GUIDE *tg, char *link, int row )
{
  long newlink = link - (char *)tg;
  int winlines = tg->h / tg->yfac;

  if (row - tg->tlc < 0 || row - tg->tlc >= winlines)
    goto_line( row - (winlines >> 1) );
  if (newlink == tg->link) return;
  graf_mouse( M_OFF, 0 );
  vswr_mode( handle, MD_XOR ); vsf_interior( handle, FIS_SOLID );
  if (tg->row - tg->tlc >= 0 && tg->row - tg->tlc < winlines)
    draw_link( tg );
  tg->link = newlink; tg->row = row; draw_link( tg );
  vsf_interior( handle, FIS_HOLLOW ); vswr_mode( handle, MD_REPLACE );
  graf_mouse( M_ON, 0 );
}

static void del_quotes( GUIDE *tg )
{
  char *end = tg->name; while (*end++); end -= 2;
  if (*end == '"') { while (*--end != '"'); *--end = 0; set_name(); }
}

static int key_text( GUIDE *tg, int code, int ks )
{
  char *key, *end;

  if (*(long *)&tg->font != *(long *)&par.font)
    set_fontpoint( tg->font, tg->point );
  switch (code)
  {
    case CNTRL_N: new_fontpoint( tg, 1, 10 ); return 0;
    case CNTRL_M: new_fontpoint( tg, 1, 9 ); return 0;
    case CNTRL_K: new_fontpoint( tg, 1, 8 ); return 0;
    case CNTRL_E: AVSendMessage( AV_ASKFILEFONT, 0, 0 ); return 0;
    case CNTRL_X: strcpy( Path, tg->path );
		  if (tg->link < 0) write_dump( tg );
		  else write_out( tg, ".OUT", 0 );
		  return 0;
    case CNTRL_C: if (set_scrp())
		    if (tg->link < 0) write_dump( tg );
		    else {
			   write_out( tg, ".TXT", -1 );
			   write_out( tg, ".OUT", 0 );
			 }
		  return 0;
    case BACKSPACE:
    case DELETE: if (tg->link < 0) return 1;
		 if (code == DELETE) del_quotes( tg );
		 else
		 {
		   end = tg->name; while (*end++);
		   if (end[-2] == '"')
		   {
		     if (end[-4] == '"') end[-5] = 0;
		     end[-3] = '"'; end[-2] = 0; set_name();
		 } }
		 return 0;
#ifndef __TOS__
    case SHFT_TAB:
#endif
    case TAB: if (tg->link <= 0) return 1;
	      key = (char *)tg + tg->link;
#ifdef __TOS__
	      code = tg->row; if (ks & 3)
#else
	      ks = code; code = tg->row; if (ks == SHFT_TAB)
#endif
	      {
		while (*--key != LINK);
		for (;;)
		  switch ((unsigned char)*--key)
		  {
		    case 0: if (--code < 0) return 0;
			    break;
		    case (unsigned char)LINK:
			while (*--key != LINK); del_quotes( tg );
			set_newlink( tg, key, code ); return 0;
	      }   }
	      else
	      {
		while (*++key != LINK);
		for (;;)
		  switch ((unsigned char)*++key)
		  {
		    case 0: if (++code == tg->lc+tg->h/tg->yfac) return 0;
			    break;
		    case (unsigned char)LINK:
			while (*++key != LINK); del_quotes( tg );
			set_newlink( tg, key, code ); return 0;
	      }   }
    case HELP:
    case RETURN:
    case ENTER: if (tg->link <= 0) return 1;
		end = key = (char *)tg + tg->link;
		while (*--key != LINK); while (*++end != LINK);
		((char *)tg)[tg->link] = *end = 0;
		strcpy( filebuf, (char *)tg + tg->link + 1 );
		strcpy( keybuf, key + 1 );
		((char *)tg)[tg->link] = *end = LINK;
		open_window( 1, code == HELP, 0, 0, 0, 0,
			     filebuf, keybuf ); return 0;
  }
  if ((code &= 0xFF) < ' ' || tg->link < 0) return 1;
  end = tg->name; while (*++end); if (end - tg->name > 124) return 0;
  if (end[-1] != '"') { *end++ = ' '; *end++ = '"'; *end++ = '"'; *end=0; }
  end[-1] = code;
  code = -1; while (end[--code] != '"'); code = ~code; key = tg->buf; ks=0;
  if (tg->link)
    for (;;)
      switch ((unsigned char)*key++)
      {
	case 0:
	  if (++ks == tg->lc + tg->h / tg->yfac)
	  {
	    end[-1] = '"'; if (end[-2] == '"') end[-3] = 0; return 0;
	  }
	  break;
	case (unsigned char)LINK:
	  if (strnicmp( key, end - code, code ) == 0)
	  {
	    while (*key++ != LINK); set_newlink( tg, key - 1, ks );
	    *end = '"'; end[1] = 0; set_name(); return 0;
	  }
	  while (*key++ != LINK); while (*key++ != LINK);
      }
  for (;;)
  {
    if (strstr( key, end - code ))
    {
      goto_line( ks - ((tg->h / tg->yfac) >> 1) );
      *end = '"'; end[1] = 0; set_name(); return 0;
    }
    while (*key++);
    if (++ks == tg->lc + tg->h / tg->yfac)
    {
      end[-1] = '"'; if (end[-2] == '"') end[-3] = 0; return 0;
} } }

static int handle_link( GUIDE *tg, int mx, int my, int ks )
{
  char *text, *next, *key;
  int  x, y, fx, flag;

  if (tg->link <= 0) return 0;
  if (*(long *)&tg->font != *(long *)&par.font)
    set_fontpoint( tg->font, tg->point );
  y = tg->y; if ((my - y - tg->h) / hchar > tg->lc - tg->tlc) return 0;
  text = tg->text; flag = tg->tlc; while ((y += hchar) <= my) ++flag;
  if (flag -= tg->line)
  {
    tg->line += flag;
    if (tg->flag == 0)
    {
      char c;

      if (flag < 0)
	do
	  for (;;)
	  {
	    if ((c = *--text) == 0) break;
	    if (c == Esc)
	    {
	      x = 15 & Esc;
	      if (*--text != c) { x = 15; (char)x &= (++text)[1]; }
	      tg->effect ^= x;
	  } }
	while (++flag < 0);
      else
	do
	  for (;;)
	  {
	    if ((c = *++text) == 0) break;
	    if (c == Esc) { x = 15; (char)x &= *++text; tg->effect ^= x; }
	  }
	while (--flag > 0);
    }
    else
      if (flag < 0)
	do { while (*--text); } while (++flag < 0);
      else
	do { while (*++text); } while (--flag > 0);
  }
  if (tg->effect != effects) vst_effects( handle, effects = tg->effect );
  tg->text = next = text; fx = x = tg->x - tg->lbc * wchar; dist = 0;
  for (;;)
    switch ((unsigned char)*++next)
    {
      case 0:	 return 0;
      case TABU: x = dist += (int)(next - text - 1) * wchar + tg->dist;
		 x -= x % tg->dist; dist = x; x += fx; text = next; break;
      case Esc:  *next = 0; x += text_width( text ); *next++ = Esc;
		 vst_effects( handle, effects ^= *next & 15 );
		 text = next; break;
      case (unsigned char)LINK:
	switch (flag)
	{
	  case 0: *next = 0; x += text_width( text ); *next = LINK;
		  if (mx < x) return 0;
		  flag = 1; key = next; text = next; break;
	  case 1: vst_effects( handle, 9 );
		  *next = 0; x += my = text_width( text ); *next = LINK;
		  vst_effects( handle, effects );
		  flag = 4; if (mx < x) ++flag;
	  case 2: flag -= 2; text = next; break;
	  case 3: del_quotes( tg ); set_newlink( tg, text, tg->line );
		  if (ks >= 0)
		  {
		    *next = *text = 0;
		    strcpy( filebuf, text + 1 );
		    strcpy( keybuf, key + 1);
		    *next = *text = LINK;
		    open_window( 1, ks, x - my, y - hchar, my, hchar,
				 filebuf, keybuf );
		  }
		  return 1;
}   }   }

static void sclick_text( GUIDE *tg, int mx, int my )
{
  int id;

  if (handle_link( tg, mx, my, -1 )) return;
  popup[6].ob_state = VA_Flags & 2 ? NORMAL : DISABLED;
  strncpy( fontstring + 2, VA_Flags & 2 ? VA_Name : "VA_FILE ", 8 );
  strncpy( savestring + 5, tg->link >= 0 ? "OUT" : "DMP", 3 );
  itostring( tg->lc + tg->h / tg->yfac, zeilstring + 8, 6 );
  id = 5; if (tg->font == 1 && tg->point <= 10) id = 12 - tg->point;
  switch (id = popup_menu( popup, id, mx, my, draw_popup ))
  {
    case 2:
    case 3:
    case 4: new_fontpoint( tg, 1, 12 - id ); break;
    case 6: AVSendMessage( AV_ASKFILEFONT, 0, 0 ); break;
    case 8: strcpy( Path, tg->path );
	    if (tg->link < 0) write_dump( tg );
	    else write_out( tg, ".OUT", 0 );
} }

static void dclick_text( GUIDE *tg, int mx, int my, int ks )
{
  if (ks < 0)
  {
    if (ks == -1) new_fontpoint( tg, mx, my );
    else *(long *)&par.wwin = *(long *)&tg->w;
    return;
  }
  handle_link( tg, mx, my, ks );
}

static void init_popup( void )
{
  int i, count;

  vst_point( handle, 10, &popup->ob_next,&popup->ob_next, &wchar, &hchar );
  popup->ob_next = -1; count = 0; i = 1;
  do
  { popup[i].ob_width *= wchar;
    popup[i].ob_y = count;
    count += popup[i].ob_height = popup[i].ob_height ? gl_hchar : hchar;
  }
  while (++i < 9);
  popup->ob_width *= wchar; popup->ob_height = count;
}

GUIDE *load_text( int fh, long len )
{
  int   i, effect, flag;
  long  oldlen;
  char  *wp, *rp, *rend;
  GUIDE *rg;

  if ((rg = Malloc( (1 + sizeof(GUIDE)) + len )) == 0)
    { Fclose( fh ); form_error( EINVMEM ); return 0; }
  wp = rg->text = &rg->null; *wp++ = 0; rp = wp;
  rend = rp + Fread( fh, len, rp ); Fclose( fh );
  if (popup->ob_next == 0) init_popup();
  i = 8;
  if (rend > rp && rend[-1] == TABU)
    if (--rend > rp && rend[-1] == 13)
      if (--rend > rp && rend[-1] == TABU)
	if (--rend > rp && rend[-1] == 13)
	{
	  while (--rend > rp && rend[-1] == TABU) ++i;
	  if ((i -= 8) <= 0) i = 8;
	}
  rg->dist = i; *(long *)&rg->line = 0;
  rg->link = 0; rg->lc = 0; rg->bc = 1; rg->flag = 1;
  effect = 0; flag = 0; len = 0;
  while (rp < rend)
    switch (i = (unsigned char)*rp++)
    {
      case   0:
      case  10: while (wp[-1] == ' ' && wp[-2] != Esc) { --wp; --len; }
		if (len > rg->bc)
		{
		  if (len <= 512) rg->bc = (int)len;
		  else
		  {
		    while (*--wp);
		    if (wp - (char *)rg < rg->link) rg->link = 0;
		    ++wp; *wp++ = 14; *wp++ = 15;
		} }
		*wp++ = 0; ++rg->lc; flag = 0; len = 0; break;
      case TABU: len += rg->dist; len -= len % rg->dist; *wp++ = i; break;
      case  25: i = '-'; ++len; *wp++ = i; break; /* WP Soft Slash	*/
      case  28:				/* 1st Word Soft Space		*/
      case  29:				/* 1st Word			*/
      case  30: i = ' '; ++len; *wp++ = i; break; /* WP Wide Space	*/
      case  31: while (rp < rend && *rp++ != 10); break;
      case  11: ++rp;			/* 1st Word Soft Formfeed	*/
      case  13: break;
      case  18:	i = effect;
		switch (*rp++)
		{
		  case '0': effect |= TXT_THICKENED;	break;
		  case '1': effect &= ~TXT_THICKENED;	break;
		  case '2': effect |= TXT_SKEWED;	break;
		  case '3': effect &= ~TXT_SKEWED;	break;
		  case '4': effect |= TXT_UNDERLINED;	break;
		  case '5': effect &= ~TXT_UNDERLINED;	break;
		  case '6': effect |= TXT_SUPERSCRIPT;	break;
		  case '7': effect &= ~TXT_SUPERSCRIPT;	break;
		  case '8': effect |= TXT_SUBSCRIPT;	break;
		  case '9': effect &= ~TXT_SUBSCRIPT;	break;
		  case 'E': effect |= TXT_LIGHT;	break;
		  case 'F': effect &= ~TXT_LIGHT;
		}
		if ((i ^= effect) != 0)
		{
		  rg->flag = 0; *wp++ = Esc; *wp++ = i;
		}
		break;
      case Esc: if ((i = *rp++) != Esc)
		{
		  i &= 0x3F; i ^= effect; effect ^= i;
		  if (i)
		  {
		    rg->flag = 0; *wp++ = Esc; *wp++ = i;
		} }
		break;
      case (unsigned char)LINK:
		switch (flag)
		{
		  case 0: flag = 1; break;
		  case 1: if (rg->link == 0)
			  {
			    rg->link = wp - (char *)rg;
			    rg->row = rg->lc;
			  }
			  flag = 2; oldlen = len; break;
		  case 2: flag = 0; len = oldlen - 2;
		}
      default:  ++len; *wp++ = i;
    }
  if (len > rg->bc)
  {
    if (len <= 512) rg->bc = (int)len;
    else
    {
      while (*--wp);
      if (wp - (char *)rg < rg->link) rg->link = 0;
      ++wp; *wp++ = 14; *wp++ = 15;
  } }
  *wp = 0; while (--rg->lc >= 0 && *--wp == 0);
  if ((rg->lc += 2) <= 0) { Mfree( rg ); form_error( ENOENT ); return 0; }
  *(long *)&rg->w = *(long *)&par.wwin; load_fonts( 1 );
  if (set_fontpoint( par.font, par.point ) == 0)
    rg->w = (rg->bc + 1) * wchar;
  *(long *)&rg->font = *(long *)&par.font;
  rg->dist *= rg->xfac = wchar; rg->yfac = hchar;
  if (par.textdef) AVSendMessage( AV_ASKFILEFONT, 0, 0 );
  rg->draw = draw_text; rg->free = free_text; rg->key = key_text;
  rg->sclick = sclick_text; rg->dclick = dclick_text;
  return rg;
}

GUIDE *load_dump( int fh, long len )
{
  GUIDE *rg;

  if (len > 16L * 32767) len = 16L * 32767;
  if ((rg = Malloc( sizeof(GUIDE) + len )) == 0)
    if ((len = Mavail() - sizeof(GUIDE)) <= 0 ||
	(rg = Malloc( sizeof(GUIDE) + len )) == 0)
      { Fclose( fh ); form_error( EINVMEM ); return 0; }
  len = Fread( fh, len, rg->buf ); Fclose( fh );
  if (len <= 0)
  {
    Mfree(rg); form_alert( 1,"[1][|Nichts zu dumpen.][  OK  ]" ); return 0;
  }
  if (popup->ob_next == 0) init_popup();
  rg->text = rg->buf; rg->text += len; rg->link = -1;
  rg->bc = 76; rg->lc = (int)((len + 15) >> 4);
  *(long *)&rg->w = *(long *)&par.wwin; load_fonts( 1 );
  if (set_fontpoint( par.font, par.point ) == 0)
    rg->w = 77 * wchar;
  *(long *)&rg->font = *(long *)&par.font;
  rg->xfac = wchar; rg->yfac = hchar; rg->dist = dist;
  if (par.textdef) AVSendMessage( AV_ASKFILEFONT, 0, 0 );
  rg->draw = draw_dump; rg->free = free_text; rg->key = key_text;
  rg->sclick = sclick_text; rg->dclick = dclick_text;
  return rg;
}

static void outchar( int c )
{
  static int flag = 0;
  static char *read_ptr = read_buf.text;

  if ((*read_ptr = c) == 0)
  {
    read_ptr = read_buf.text; c = par.margin;
    v_alpha_text( ohandle, read_buf.text - c );
    if (flag)
    {
      read_buf.text[-2] = ' '; read_buf.text[-1] = ' ';
      read_buf.text[-c] = ' '; read_buf.text[1-c] = ' ';
      par.margin = c -= 4; flag = 0;
    }
    if (c && oeffect & TXT_UNDERLINED)
    {
      par.margin = c += 4; flag = 1;
      read_buf.text[-2] = 18; read_buf.text[-1] = '4';
      read_buf.text[-c] = 18; read_buf.text[1-c] = '5';
  } }
  else if (read_ptr < read_buf.end - 1) ++read_ptr;
}

static void out_effect( int new )
{
  oeffect ^= new;

  if (oeffect & TXT_THICKENED)
  {
    outchar( 18 ); outchar( new & TXT_THICKENED ? '0' : '1' );
  }
  if (oeffect & TXT_LIGHT)
  {
    outchar( 18 ); outchar( new & TXT_LIGHT ? 'E' : 'F' );
  }
  if (oeffect & TXT_SKEWED)
  {
    outchar( 18 ); outchar( new & TXT_SKEWED ? '2' : '3' );
  }
  if (oeffect & TXT_UNDERLINED)
  {
    outchar( 18 ); outchar( new & TXT_UNDERLINED ? '4' : '5' );
  }
  if (oeffect & TXT_SUPERSCRIPT)
  {
    outchar( 18 ); outchar( new & TXT_SUPERSCRIPT ? '6' : '7' );
  }
  if (oeffect & TXT_SUBSCRIPT)
  {
    outchar( 18 ); outchar( new & TXT_SUBSCRIPT ? '8' : '9' );
  }
  oeffect = new;
}

static int out_line( int fh )
{
  int flag;
  unsigned char c[2];

  flag = 2;
  while (Fread( fh, 1, c ) > 0)
    switch (*c)
    {
      case  10: if (flag != 2) out_effect( 0 );
		outchar( 10 ); outchar( 0 ); return 0;
      case  11: Fread( fh, 1, c ); break; /* 1st Word Soft Formfeed	*/
      case Esc: Fread( fh, 1, c );
		if (*c == Esc) { outchar( Esc ); outchar( Esc ); }
		else out_effect( *c & 0x3F ); break;
      case  28:				  /* 1st Word Soft Space	*/
      case  29:				  /* 1st Word			*/
      case  30: outchar( ' ' ); break;	  /* 1st Word Wide Space	*/
      case  31: while (Fread( fh, 1, c ) > 0 && *c != 10); break;
      case (unsigned char)LINK:
		switch (flag)
		{
		  case 2: out_effect( 9 );
		  case 1: --flag; break;
		  case 0: flag = 2; out_effect( 0 );
		}
		break;
      case  25: *c = '-';		  /* 1st Word Soft Slash	*/
      default:  if (flag) outchar( *c );
    }
  outchar( 13 - par.formfeed ); outchar( 0 ); return 1;
}

int (*out_text( void ))( int fh )
{
  memset( read_buf.space, ' ', sizeof(read_buf.space) );
  if (par.quality) { outchar( 18 ); outchar( 'C' - par.quality ); }
  out_effect( 0 ); return out_line;
}