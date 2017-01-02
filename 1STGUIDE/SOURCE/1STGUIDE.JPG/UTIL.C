#include <aes.h>
#include <vdi.h>
#include <osbind.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "1stguide.h"

struct { char parmagic[4];
	 long hertz;
	 int  outdef, fulldef, textdef, spoolflag, overflag,
	      usedith, dithmod, dithcol, dithtyp, usedsp,
	      out_handle, margin, formfeed, quality, meta_scale,
	      aspect, x_scale, y_scale, h_align, v_align,
	      xwin, ywin, wwin, hwin,
	      xmin, ymin, wmax, hmax,
	      font, point;
	 char extspool[16], indexfile[128];
       }
par = { 'p', 'f', 'u', 'i',
	7000,
	0, 1, 0, 0, 0,
	1, 1, 1, 1, 1,
	21, 0, 1, 0, 1,
	2, 1, 1, 1, 0,
	40, 85, 528, 256,
	0, 38, 621, 343,
	1, 10,
	"CALCLOCK", "D:\\1STGUIDE\\1STGUIDE.IDX"
      };

typedef struct
{
	unsigned long	id;		/* Selectric ID (SLCT)		*/
	unsigned int	version;	/* version (BCD-Format)		*/
	struct
	{	unsigned 	: 8;	/* reserved			*/
		unsigned pthsav : 1;	/* save GEMDOS paths		*/
		unsigned stdest : 1;	/* stay in destination path	*/
		unsigned autloc : 1;	/* auto-locator			*/
		unsigned numsrt	: 1;	/* numsort			*/
		unsigned lower	: 1;	/* use lowercase letters	*/
		unsigned dclick	: 1;	/* open folder on dclick	*/
		unsigned hidden : 1;	/* show hidden files		*/
		unsigned bypass : 1;	/* Selectric ON/OFF		*/
	} config;
	int	sort;			/* sort-mode (neg. = rev.)	*/
	int	num_ext;		/* number of extensions		*/
	char	*(*ext)[];		/* preset extensions		*/
	int	num_paths;		/* number of paths		*/
	char	*(*paths)[];		/* preset paths			*/
	int	comm;			/* communication word		*/
	int	in_count;		/* input counter		*/
	void	*in_ptr;		/* input pointer		*/
	int	out_count;		/* output counter		*/
	void	*out_ptr;		/* output pointer		*/
	int	cdecl	(*get_first)( DTA *pdta, int attrib );
	int	cdecl 	(*get_next)( DTA *pdta );
	int	cdecl	(*release_dir)( void );
}
SLCT_STR;

static SLCT_STR *slct = 0;
static int      opened = 0;

jmp_buf setjmp_buffer;

typedef struct
{
  unsigned char *pbuf;
  long          bytes_left;
  unsigned char *fbuf;
  long          buf_size;
  unsigned char sbuf[4];
  int		handle;
}
FBUF;

void Fbufopen( FBUF *fp )
{
  fp->bytes_left = 0;
  while (fp->buf_size > sizeof(fp->sbuf))
  {
    if ((fp->fbuf = Malloc( fp->buf_size )) != 0) return;
    fp->buf_size = Mavail() & -sizeof(fp->sbuf);
  }
  fp->buf_size = sizeof(fp->sbuf);
  fp->fbuf = fp->sbuf;
}

void Fbufcreate( FBUF *fp )
{
  do
  { fp->bytes_left = Mavail() & -sizeof(fp->sbuf);
    if (fp->bytes_left <= sizeof(fp->sbuf))
    {
      fp->bytes_left = sizeof(fp->sbuf);
      fp->pbuf = fp->sbuf; break;
  } }
  while ((fp->pbuf = Malloc( fp->bytes_left )) == 0);
  fp->fbuf = fp->pbuf;
  fp->buf_size = fp->bytes_left;
}

void Fbufread( FBUF *fp )
{
  long count;

  fp->pbuf = fp->fbuf;
  count = Fread( fp->handle, fp->buf_size, fp->pbuf );
  fp->bytes_left += count;
  if (count <= 0) longjmp( setjmp_buffer, 1 );
}

void Fbufwrite( FBUF *fp )
{
  long count;

  if ((count = fp->buf_size - fp->bytes_left) != 0)
  {
    fp->pbuf = fp->fbuf;
    fp->bytes_left += count;
    if (Fwrite( fp->handle, count, fp->pbuf ) != count)
      longjmp( setjmp_buffer, 1 );
} }

void Fbufclose( FBUF *fp )
{
  if (fp->fbuf != fp->sbuf) Mfree( fp->fbuf ); Fclose( fp->handle );
}

int Findfirst( char *name, DTA *pdta, int attrib )
{
  DTA *pold;
  int ret;

  pold = Fgetdta(); Fsetdta( pdta );
  ret = Fsfirst( name, attrib ); Fsetdta( pold ); return ret;
}

int Findnext( DTA *pdta )
{
  DTA *pold;
  int ret;

  pold = Fgetdta(); Fsetdta( pdta );
  ret = Fsnext(); Fsetdta( pold ); return ret;
}

void slct_open( void )
{
#ifdef __TOS__
  if (opened) return;
  if (slct == 0)
  {
    void *oldstack = (void *)Super( 0 );
    long *cookiejar = *(long **)0x5A0;
    if (cookiejar)
    {
      while (*cookiejar)
      {
	if (*cookiejar++ == 'FSEL')
	{
	  SLCT_STR *p = (SLCT_STR *)*cookiejar;
	  if (p->id == 'SLCT' && p->version >= 0x102) slct = p;
	  break;
	}
	cookiejar++;
    } }
    Super( oldstack );
  }
  if (slct) { slct->comm = 9; opened++; }
#endif
}

int slct_first( DTA *pdta, int attrib )
{
  if (opened) return (*slct->get_first)( pdta, attrib );
  return 1;
}

int slct_next( DTA *pdta )
{
  if (opened) return (*slct->get_next)( pdta );
  return 1;
}

void slct_close( void )
{
  if (opened) { opened = 0; (*slct->release_dir)(); }
}

void fix_tree( OBJECT *tree, int count )
{
  do rsrc_obfix( tree, count ); while (--count >= 0);
}

void memswap( char *p, char *q, long count )
{
  while (--count >= 0) { char t = *p; *p++ = *q; *q++ = t; }
}

void struplo( char *s )
{
  char c, mask = 0x20;

  while ((c = *s) != 0)
  {
    c |= mask;
    if ('a' <= c && c <= 'z') *s ^= mask;
    s++;
} }

static void background( int *rect )
{
  static int cop[8];
  static MFDB screen, save;

  if (rect)
  {
    *(long *)cop = *(long *)rect; *(long *)(cop + 4) = 0;
    cop[6] = rect[2] - 1; cop[2] = rect[0] + cop[6];
    cop[7] = rect[3] - 1; cop[3] = rect[1] + cop[7];
    *(long *)&save.fd_w = *(long *)(rect + 2);
    save.fd_wdwidth = (save.fd_w + 15) >> 4;
    save.fd_nplanes = nplanes; save.fd_stand = 0;
    wind_get( 0, WF_SCREEN, (int *)&save.fd_addr, (int *)&save.fd_addr + 1,
	      &save.fd_r2, &save.fd_r3 );
    save.fd_r1 = 0; *(long *)&save.fd_r2 = 0; screen.fd_addr = 0;
  }
  else
  {
    memswap( (char *)cop, (char *)(cop + 4), 8 );
    memswap( (char *)&screen, (char *)&save, sizeof( MFDB ) );
  }
  vro_cpyfm( handle, S_ONLY, cop, &screen, &save );
}

void printbox( char *name, int count )
{
  static char page[] = "Seite XXX",
	      no_cancel[] = "Abbruch nicht m”glich!";
#pragma warn -rpt
  static OBJECT prbox[] =
  {
    0,  1,  3, G_BOX,    NONE, OUTLINED, 0x21100L, 0, 0, 38, 5,
    2, -1, -1, G_STRING, NONE, NORMAL, "Drucke",   2, 1, 6,  1,
    3, -1, -1, G_STRING, NONE, NORMAL, 0,	   9, 1, 29, 1,
    0, -1, -1, G_STRING, LASTOB, NORMAL, 0,	   2, 3, 22, 1
  };
#pragma warn +rpt
  int clip[4];

  graf_mouse( M_OFF, 0 );
  if (name)
  {
    itoa( count, page + 6, 10 );
    prbox[3].ob_spec.free_string = count ? page : no_cancel;
    if ((count = (int)strlen( name ) - 29) > 0) name += count;
    prbox[2].ob_spec.free_string = name;
    if (prbox->ob_next == 0) { --prbox->ob_next; fix_tree( prbox, 3 ); }
    form_center( prbox, clip, clip + 1, clip + 2, clip + 3 );
    background( clip );
    wind_get( 0, WF_WORKXYWH, clip, clip + 1, clip + 2, clip + 3 );
    objc_draw( prbox, ROOT, MAX_DEPTH, clip[0],clip[1],clip[2],clip[3] );
  }
  else background( 0 );
  graf_mouse( M_ON, 0 );
}

void flipwords( char *val, long count )
{
  char c;

  while ((count -= 2) >= 0)
    { c = *val++; val[-1] = val[0]; *val++ = c; }
}

void fliplongs( int *val, long count, int flag )
{
  int w;

  while (--count >= 0)
  {
    if (flag) flipwords( (char *)val, 4 );
    w = *val++; val[-1] = val[0]; *val++ = w;
} }

void itostring( int value, char *string, int count )
{
  do *--string = '0' + value % 10;
  while (--count && (value /= 10) != 0);
  while (--count > 0) *--string = ' ';
}

void load_fonts( int flag )
{
  static int count = 0;

  if (vq_gdos())
    if (flag) { if (count == 0) vst_load_fonts( handle, 0 ); ++count; }
    else if (--count == 0) vst_unload_fonts( handle, 0 );
}

void set_fonts( int flag )
{
  static int loaded = 0;

  if (flag) vst_load_fonts( ohandle, 0 );
  else if (loaded) vst_unload_fonts( ohandle, 0 );
  loaded = flag;
}

int set_scrp( void )
{
  char *p, *q, buf[128];

  if (scrp_read( p = buf ) && *p)
  {
    q = Path; while ((*q++ = *p++) != 0);
    --q; if (q[-1] != '\\') *q++ = '\\';
    *q++ = 'S'; *q++ = 'C'; *q++ = 'R'; *q++ = 'A'; *q++ = 'P'; *q = 0;
    return 1;
  }
  form_alert( 1, "[1][|Kein Clipboard-Pfad gesetzt.][Abbruch]" );
  return 0;
}

static void change_select( OBJECT *tree, int id )
{
  if (id && (tree[id].ob_state & DISABLED) == 0)
    objc_change( tree, id, 0, xdesk, ydesk, wdesk, hdesk,
		 tree[id].ob_state ^ SELECTED, 1 );
}

int popup_menu( OBJECT *tree, int initob, int mx, int my,
		int (*draw)( OBJECT *tree, int startob, int depth,
			     int xc, int yc, int wc, int hc ) )
{
  int id, event, mbut, d[1], pop[4];

  pop[0] = mx - tree[initob].ob_x - (tree[initob].ob_width >> 1);
  if (xdesk > pop[0]) pop[0] = xdesk;
  pop[2] = tree->ob_width + 4;
  if ((id = pop[0] + pop[2] - 1 - xmax) > 0) pop[0] -= id;
  pop[1] = my - tree[initob].ob_y - (tree[initob].ob_height >> 1);
  if (ydesk > pop[1]) pop[1] = ydesk;
  pop[3] = tree->ob_height + 4;
  if ((id = pop[1] + pop[3] - 1 - ymax) > 0) pop[1] -= id;
  tree->ob_x = pop[0] + 1;
  tree->ob_y = pop[1] + 1;
  graf_mouse( M_OFF, 0 ); background( pop );
  id = tree[initob].ob_state & DISABLED;
  if (id == 0) *tree[initob].ob_spec.free_string = 8;
  (*draw)( tree, 0, 1, xdesk, ydesk, wdesk, hdesk );
  if (id == 0) *tree[initob].ob_spec.free_string = ' ';
  graf_mkstate( &mx, &my, &mbut, d ); mbut ^= 1;
  wind_update( BEG_MCTRL );

  do
  { if ((id = objc_find( tree, 0, 1, mx, my )) < 0) id = 0;
    change_select( tree, id ); graf_mouse( M_ON, 0 );
    objc_offset( tree, id, &mx, &my );

    event = evnt_multi( MU_BUTTON | MU_M1, 1, 1, mbut, id ? 1 : 0, mx, my,
			tree[id].ob_width, tree[id].ob_height,
			0, 0, 0, 0, 0, 0, 0, 0, &mx, &my, d, d, d, d );

    graf_mouse( M_OFF, 0 ); change_select( tree, id );
  }
  while ((event & MU_BUTTON) == 0);

  wind_update( END_MCTRL );
  if (tree[id].ob_state & DISABLED || id == initob) id = 0;
  if (id)
  {
    event = 6;
    for (;;)
    {
      change_select( tree, id );
      if (--event == 0) break; evnt_timer( 50, 0 );
  } }
  background( 0 ); graf_mouse( M_ON, 0 );
  if (mbut) evnt_button( 1, 1, 0, d, d, d, d );
  return id;
}