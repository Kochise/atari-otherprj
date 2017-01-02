#include <aes.h>
#include <vdi.h>
#include <gemdefs.h>
#include <osbind.h>
#include <stdlib.h>
#include <errno.h>
#include <scancode.h>

int play_sam( void *adr, void *end, long hertz, long hold, int dma );

typedef struct guide
{
  int	handle, s1;
  long	hertz;
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

  int   dma, data;
  long	dw, dh;
  char	*start, *end,
	buf[];			/* Sample-Buffer.			*/
}
GUIDE;

#include "1stguide.h"
#include "util.h"

static int  dmaflag = 0;
static char samstring[] = "---       Hertz ---",
	    hertzstring[] = "       ",
	    wstring[] = "  Fenster       ^E",
	    ostring[] = "  Original      ^S",
	    normstring[] = "  Norm ^N ",
	    dmastring[] = "  DMA ^M ";

#pragma warn -rpt
static OBJECT popup[] =
{
  0,  1, 10, G_BOX,    NONE,   SHADOWED, 0xFF1100L,		0, 0, 19,7,
  2, -1, -1, G_STRING, NONE,   DISABLED, samstring,		0, 0, 19,1,
  3, -1, -1, G_STRING, NONE,   NORMAL,   wstring,		0, 1, 19,1,
  4, -1, -1, G_BUTTON, NONE,   NORMAL,   "- ^\004",		0, 2, 6, 1,
  5, -1, -1, G_BUTTON, NONE,   DISABLED, hertzstring,		6, 2, 7, 1,
  6, -1, -1, G_BUTTON, NONE,   NORMAL,   "+ ^\003",		13,2, 6, 1,
  7, -1, -1, G_STRING, NONE,   NORMAL,   ostring,		0, 3, 19,1,
  8, -1, -1, G_STRING, NONE,   DISABLED, "----- P L A Y -----", 0, 4, 19,1,
  9, -1, -1, G_BUTTON, NONE,   NORMAL,   normstring,		0, 5, 10,1,
 10, -1, -1, G_BUTTON, NONE,   DISABLED, dmastring,		10,5, 9, 1,
  0, -1, -1, G_STRING, LASTOB, NORMAL,   "      STOP      ^T ",	0, 6, 19,1
};
#pragma warn +rpt

static void check_dma( void )
{
#ifdef __TOS__
  void *oldstack;
  long *cookiejar;

  oldstack = (void *)Super( 0 );
  cookiejar = *(long **)0x5A0;
  if (cookiejar)
  {
    while (*cookiejar)
    {
      if (*cookiejar == '_SND')
      {
	if (cookiejar[1] & 2L) popup[9].ob_state = NORMAL;
	break;
      }
      cookiejar += 2;
  } }
  Super( oldstack );
#endif
}

static void change_hz( GUIDE *tg, int flag )
{
#ifdef __TOS__
  void *oldstack;

  oldstack = (void *)Super( 0 );
  if (tg->dma)
  {
    flag += tg->data;
    if (flag < 0x80) flag = 0x80; if (flag > 0x83) flag = 0x83;
    *(int *)0xFFFF8920L = flag;
  }
  else
  {
    if ((flag = tg->data - flag) <= 0) flag = 1;
    if (flag > 255) flag = 255;
    *(char *)0xFFFFFA1FL = flag;
  }
  Super( oldstack ); tg->data = flag;
#endif
}

static void new_item( GUIDE *tg, int new )
{
  if (new == 2)
  {
    if (tg->dw == tg->w && tg->dh == tg->h) return;
    tg->dw = tg->w; tg->dh = tg->h;
  }
  else
  {
    if (tg->dw == tg->end - tg->start && tg->dh == 256) return;
    tg->dw = tg->end - tg->start; tg->dh = 256;
  }
  tg->bc = (int)((tg->dw + 31) >> 5); tg->lc = (int)((tg->dh + 7) >> 3);
  new_redraw();
}

static int *pline( int *pxy )
{
  VDI( 6, (int)(pxy - ptsin) >> 1, 0, handle ); return ptsin;
}

static void draw_sample( GUIDE *tg, int *clip )
{
  int  *pxy;
  char *p, *q;
  long len;
  int  dx, dy;

  vr_recfl( handle, clip ); vs_clip( handle, 1, clip ); pxy = ptsin;
  q = p = tg->start; len = tg->end - p; dy = tg->y - (tg->tlc << 3);
  if (tg->dw == len)
  {
    p += ((long)tg->lbc << 5) + clip[0] - tg->x;
    q = p; q += clip[2] - clip[0]; if (++q > tg->end) q = tg->end;
    dx = clip[0] - 1; if (--p < tg->start) { ++p; ++dx; }
    while (p < q)
    {
      *pxy++ = dx++;
      *pxy++ = dy + (int)(((127 - *p++) * tg->dh) >> 8);
      if (pxy == ptsin + 128)
      {
	pxy = pline( pxy ); *(long *)pxy = *(long *)(pxy + 126); pxy += 2;
  } } }
  else
  {
    dx = tg->x - (tg->lbc << 5);
    p += ((clip[0] - 1 - dx) * len) / tg->dw;
    q += ((clip[2] + 1 - dx) * len) / tg->dw;
    if (p < tg->start) p = tg->start; if (q > tg->end) q = tg->end;
    while (p < q)
    {
      *pxy++ = dx + (int)(((p - tg->start) * tg->dw) / len);
      *pxy++ = dy + (int)(((127 - *p++) * tg->dh) >> 8);
      if (pxy == ptsin + 128)
      {
	pxy = pline( pxy ); *(long *)pxy = *(long *)(pxy + 126); pxy += 2;
  } } }
  pline( pxy ); vs_clip( handle, 0, clip );
}

static void free_sample( GUIDE *tg )
{
#ifdef __TOS__
  play_sam( tg->start, tg->start, tg->hertz, 0, tg->dma );
#endif
}

static void do_play( GUIDE *tg )
{
  tg->dma = dmaflag;
#ifdef __TOS__
  tg->data = play_sam( tg->start, tg->end, tg->hertz, 1, tg->dma );
#else
  tg->data = (int)(614400L / tg->hertz);
#endif
}

#pragma warn -par
static int key_sample( GUIDE *tg, int code, int ks )
{
  switch (code)
  {
    case CNTRL_E:  new_item( tg, 2 ); return 0;
    case CNTRL_S:  new_item( tg, 6 ); return 0;
    case CNTRL_M:  if (popup[9].ob_state == NORMAL)
		     { free_sample( tg ); dmaflag = 1; do_play( tg ); }
		   return 0;
    case CNTRL_N:  free_sample( tg ); dmaflag = 0; do_play( tg ); return 0;
    case CNTRL_T:  free_sample( tg ); return 0;
    case CNTRL_CL: change_hz( tg, -1 ); return 0;
    case CNTRL_CR: change_hz( tg, 1 ); return 0;
  }
  return 1;
}
#pragma warn +par

static void sclick_sample( GUIDE *tg, int mx, int my )
{
  long value, count;
  char *string;

  value = tg->hertz; count = 6; string = samstring + 9;
  do *--string = '0' + value % 10;
  while (--count && (value /= 10) != 0);
  while (--count > 0) *--string = ' ';
  if (tg->dma == 0) value = 614400L / tg->data;
  else switch (tg->data)
       {
	 case 0x80: value =  6258L; break;
	 case 0x81: value = 12517L; break;
	 case 0x82: value = 25033L; break;
	 default:   value = 50066L;
       }
  ltoa( value, hertzstring, 10 );
  *wstring = tg->dw == tg->w && tg->dh == tg->h ? 8 : ' ';
  *ostring = tg->dw == tg->end - tg->start && tg->dh == 256 ? 8 : ' ';
  *normstring = tg->dma ? ' ' : 8; *dmastring = tg->dma ? 8 : ' ';
  switch (popup_menu( popup, 4, mx, my, objc_draw ))
  {
    case  2: new_item( tg, 2 ); break;
    case  3: change_hz( tg, -1 ); break;
    case  5: change_hz( tg, 1 ); break;
    case  6: new_item( tg, 6 ); break;
    case  8: free_sample( tg ); dmaflag = 0; do_play( tg ); break;
    case  9: free_sample( tg ); dmaflag = 1; do_play( tg ); break;
    case 10: free_sample( tg );
} }

static void dclick_sample() { }

GUIDE *load_sample( int fh, long len )
{
  GUIDE *rg;
  char  *stop, *start;

  if ((rg = Malloc( sizeof(GUIDE) + len )) == 0)
    { Fclose( fh ); form_error( EINVMEM ); return 0; }
  rg->hertz = par.hertz; start = rg->buf; rg->end = stop = start + len;
  Fread( fh, len, start ); Fclose( fh );
  if (popup->ob_next == 0)
    { --popup->ob_next; fix_tree( popup, 10 ); check_dma(); }
#ifdef __TOS__
  if (*(long *)start == 0xAABBCCDDL)		/* SoundMerlin Format */
#else
  if (*(long *)start == 0xDDCCBBAAL)
#endif
  {
    rg->hertz = 10U * ((unsigned *)start)[10]; stop = start += 50;
  }
#ifdef __TOS__
  else if (*(long *)start == 0x0000AB12L)	/* SoundMachine Format */
#else
  else if (*(long *)start == 0x12AB0000L)
#endif
  {
    rg->hertz = 13000; stop = start += 34;
  }
#ifdef __TOS__
  else if ((*(long *)start & 0xFFFFFF00L) == 'JON\0')		/* ??? */
#else
  else if ((*(long *)start & 0x00FFFFFFL) ==
	   ((long)'N' << 16) + ((long)'O' << 8) + 'J')
#endif
  {
    rg->hertz = 1000U * ((unsigned char *)start)[3]; start += 4;
  }
  while (stop > start) *--stop -= 128;
  rg->start = start; rg->xfac = 32; rg->yfac = 8;
  rg->w = 512; rg->h = 256; rg->dh = 256; rg->lc = 32;
  rg->dw = rg->end - start; rg->bc = (int)((rg->dw + 31) >> 5);
  rg->draw = draw_sample; rg->free = free_sample; rg->key = key_sample;
  rg->sclick = sclick_sample; rg->dclick = dclick_sample;
  do_play( rg ); return rg;
}