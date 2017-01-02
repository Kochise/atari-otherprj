#include <aes.h>
#include <vdi.h>
#include <osbind.h>
#include <stdlib.h>
#include <errno.h>
#include <scancode.h>

typedef struct rsxhdr
{
  int	rsh_vrsn,   rsh_extvrsn;
  long  rsh_object, rsh_tedinfo, rsh_iconblk, rsh_bitblk, rsh_frstr,
        rsh_string, rsh_imdata,  rsh_frimg,   rsh_trindex,
	rsh_nobs,   rsh_ntree,   rsh_nted,    rsh_nib,
	rsh_nbb,    rsh_nstring, rsh_nimages, rsh_rssize;
} RSXHDR;

typedef struct guide
{
  int	handle, flip;
  long	tree;
  int	lbc,			/* left-border-count fr Fenster.	*/
	tlc,			/* top-line-count fr Fenster.		*/
	bc,			/* border-count fr Fenster.		*/
	lc,			/* line-count fr Fenster.		*/
	xfac, yfac,		/* Scroll- und Rastereinheiten.		*/
	x, y, w, h;		/* Arbeitsbereich des Fensters.		*/
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

  char   *addr;
  RSXHDR header;
  char   buf[];			/* Resourcepuffer.			*/
}
GUIDE;

#include "1stguide.h"
#include "util.h"

static char string2[4], string0[] = "---- XX B„ume ----",
	    string_i_to_m[] = " Ix86 -> M68k  ^K",
	    string_m_to_i[] = " M68k -> Ix86  ^K";

#pragma warn -rpt
static OBJECT popup[] =
{
  0,  1,  6, G_BOX,    NONE,   SHADOWED, 0xFF1100L,	       0, 0,18, 4,
  5, -1, -1, G_STRING, NONE,   DISABLED, string0,	       0, 0,18, 1,
  3, -1, -1, G_BUTTON, NONE,   NORMAL,   "<< ^\004",	       0, 1, 7, 1,
  4, -1, -1, G_BUTTON, NONE,   DISABLED, string2,	       7, 1, 4, 1,
  6, -1, -1, G_BUTTON, NONE,   NORMAL,   ">> ^\003",	       11,1, 7, 1,
  2, -1, -1, G_STRING, NONE,   DISABLED, "-- Konvertieren --", 0, 2,18, 1,
  0, -1, -1, G_STRING, LASTOB, NORMAL,   string_m_to_i,        0, 3,18, 1
};
#pragma warn +rpt

static void vdi_trans( int *addr, int w, int h )
{
  MFDB s, d;

  s.fd_addr = addr; s.fd_w = w; s.fd_h = h; s.fd_wdwidth = (w + 15) >> 4;
  s.fd_stand = 1; s.fd_nplanes = 1; s.fd_r1 = 0; *(long *)&s.fd_r2 = 0;
  d = s; vr_trnfm( handle, &s, &d );
}

static void flip_header( RSXHDR *header, char *addr )
{
  flipwords( (char *)header + 4, sizeof(RSXHDR) - 4 );
  if ((char *)header == addr) fliplongs( (int *)header + 2, 17, 0 );
}

static void flip_resource( RSXHDR *header, char *addr, int flag )
{
  OBJECT  *op;
  TEDINFO *tp;
  ICONBLK *ip;
  BITBLK  *bp;
  long    i, s;

  op = (OBJECT *)(addr + header->rsh_object);
  for (i = header->rsh_nobs; --i >= 0; ++op)
  {
    flipwords( (char *)op, sizeof( OBJECT ) );
    fliplongs( (int *)&op->ob_spec, 1, 0 );
  }
  tp = (TEDINFO *)(addr + header->rsh_tedinfo);
  for (i = header->rsh_nted; --i >= 0; ++tp)
  {
    flipwords( (char *)tp, sizeof( TEDINFO ) );
    fliplongs( (int *)tp, 3, 0 );
  }
  ip = (ICONBLK *)(addr + header->rsh_iconblk);
  for (i = header->rsh_nib; --i >= 0; ++ip)
  {
    if (flag == 0)
    {
      s = (long)ip->ib_wicon * ip->ib_hicon >> 3;
      flipwords( addr + (long)ip->ib_pmask, s );
      flipwords( addr + (long)ip->ib_pdata, s );
    }
    flipwords( (char *)ip, sizeof( ICONBLK ) );
    fliplongs( (int *)ip, 3, 0 );

    if (flag)
    {
      s = (long)ip->ib_wicon * ip->ib_hicon >> 3;
      flipwords( addr + (long)ip->ib_pmask, s );
      flipwords( addr + (long)ip->ib_pdata, s );
  } }
  bp = (BITBLK *)(addr + header->rsh_bitblk);
  for (i = header->rsh_nbb; --i >= 0; ++bp)
  {
    if (flag == 0)
      flipwords( addr + (long)bp->bi_pdata, (long)bp->bi_wb * bp->bi_hl );

    flipwords( (char *)bp, sizeof( BITBLK ) );
    fliplongs( (int *)bp, 1, 0 );

    if (flag)
      flipwords( addr + (long)bp->bi_pdata, (long)bp->bi_wb * bp->bi_hl );
  }
  fliplongs( (int *)(addr + header->rsh_frstr), header->rsh_nstring, 1 );
  fliplongs( (int *)(addr + header->rsh_frimg), header->rsh_nimages, 1 );
  fliplongs( (int *)(addr + header->rsh_trindex), header->rsh_ntree, 1 );
}

static void fix_objects( RSXHDR *header, char *addr, int flag )
{
  OBJECT  *tree, *op;
  TEDINFO *tp;
  ICONBLK *ip;
  BITBLK  *bp;
  long    k, offset;

  if (flag)
    { flip_header( header, addr ); flip_resource( header, addr, 1 ); }
  op = tree = (OBJECT *)(addr + header->rsh_object);
  for (k = 0; k < header->rsh_nobs; ++k)
  {
    offset = op->ob_spec.index;
    switch (op->ob_type & 0xFF)
    {
      case G_BUTTON:
      case G_STRING:
      case G_TITLE:
	op->ob_spec.free_string = addr + offset; break;
      case G_TEXT:
      case G_BOXTEXT:
      case G_FTEXT:
      case G_FBOXTEXT:
	op->ob_spec.tedinfo = tp = (TEDINFO *)(addr + offset);
	tp->te_ptext = addr + (long)tp->te_ptext;
	tp->te_ptmplt = addr + (long)tp->te_ptmplt;
	tp->te_pvalid = addr + (long)tp->te_pvalid; break;
      case G_IMAGE:
	op->ob_spec.bitblk = bp = (BITBLK *)(addr + offset);
	bp->bi_pdata = (int *)(addr + (long)bp->bi_pdata);
	vdi_trans( bp->bi_pdata, bp->bi_wb << 3, bp->bi_hl ); break;
      case G_ICON:
	op->ob_spec.iconblk = ip = (ICONBLK *)(addr + offset);
	ip->ib_pmask = (int *)(addr + (long)ip->ib_pmask);
	ip->ib_pdata = (int *)(addr + (long)ip->ib_pdata);
	ip->ib_ptext = addr + (long)ip->ib_ptext;
	vdi_trans( ip->ib_pmask, ip->ib_wicon, ip->ib_hicon );
	vdi_trans( ip->ib_pdata, ip->ib_wicon, ip->ib_hicon ); break;
      case G_USERDEF:
      case G_CICON:
	op->ob_flags = HIDETREE;
    }
    rsrc_obfix( tree, (int)k ); ++op;
} }

static void convert_resource( GUIDE *tg )
{
  RSXHDR *header;
  char   *addr;
  long   len;
  int    fh;

  graf_mouse( BUSYBEE, 0 );
  if ((fh = Fopen( tg->path, 2 )) < 0)
    { graf_mouse( ARROW, 0 ); form_error( -fh - 31 ); return; }
  header = &tg->header; addr = tg->addr; len = Fseek( 0, fh, 2 );
  len -= Fseek( tg->buf - addr, fh, 0 );
  Fread( fh, len, tg->buf ); Fseek( 0, fh, 0 );
  flip_resource( header, addr, tg->flip );
  if (tg->flip == 0) flip_header( header, addr );
  if ((char *)header == addr) Fwrite( fh, sizeof(RSXHDR), header );
  else
  { { int *p = (int *)header;
      int count = 18; do *p++ = (int)*((long *)header)++; while (--count);
    }
    Fwrite( fh, sizeof(RSHDR), &tg->header );
    {
      unsigned *p = (unsigned *)addr;
      int count = 18; do *--(long *)header = *--p; while (--count);
  } }
  Fwrite( fh, len, tg->buf ); Fclose( fh );
  fix_objects( header, addr, tg->flip ^= 1 ); graf_mouse( ARROW, 0 );
}

static OBJECT *set_tree( GUIDE *tg, int flag )
{
  OBJECT *tree;
  long   *poffset;

  poffset = (long *)(tg->addr + tg->header.rsh_trindex);
  tree = (OBJECT *)(tg->addr + poffset[tg->tree]);
  if (flag)
  {
    tree->ob_x = tg->x - (tg->lbc << 3);
    tree->ob_y = tg->y - (tg->tlc << 3);
  }
  else
  {
    tg->bc = (tree->ob_width + 7) >> 3;
    tg->lc = (tree->ob_height + 7) >> 3;
  }
  return tree;
}

static void new_tree( GUIDE *tg, int flag )
{
  if (flag) { if (++tg->tree >= tg->header.rsh_ntree) tg->tree = 0; }
  else if (--tg->tree < 0) tg->tree += tg->header.rsh_ntree;
  set_tree( tg, 0 ); new_redraw();
}

static void draw_resource( GUIDE *tg, int *clip )
{
  OBJECT *tree;
  int    xe, ye;

  tree = set_tree( tg, 1 );
  xe = tree->ob_x + tree->ob_width;
  ye = tree->ob_y + tree->ob_height;
  if (xe <= clip[2] || ye <= clip[3] || tree->ob_type == G_IBOX)
  {
    vr_recfl( handle, clip );
    if (xe <= clip[0] || ye <= clip[1]) return;
  }
  objc_draw( tree, ROOT, MAX_DEPTH, clip[0], clip[1],
	     clip[2] - clip[0] + 1, clip[3] - clip[1] + 1 );
}

static void free_resource() { }

#pragma warn -par
static int key_resource( GUIDE *tg, int code, int ks )
{
  switch (code)
  {
    case CNTRL_CL: new_tree( tg, 0 ); return 0;
    case CNTRL_CR: new_tree( tg, 1 ); return 0;
    case CNTRL_K:  convert_resource( tg ); return 0;
  }
  return 1;
}
#pragma warn +par

static void sclick_resource( GUIDE *tg, int mx, int my )
{
  popup[2].ob_state =
  popup[4].ob_state = tg->header.rsh_ntree == 1 ? DISABLED : NORMAL;
  itostring( (int)tg->header.rsh_ntree, string0 + 7, 3 );
  itoa( (int)tg->tree + 1, string2, 10 );
#ifdef __TOS__
  popup[6].ob_spec.free_string = tg->flip ? string_i_to_m : string_m_to_i;
#else
  popup[6].ob_spec.free_string = tg->flip ? string_m_to_i : string_i_to_m;
#endif
  switch (popup_menu( popup, 3, mx, my, objc_draw ))
  {
    case 2: new_tree( tg, 0 ); break;
    case 4: new_tree( tg, 1 ); break;
    case 6: convert_resource( tg );
} }

static void dclick_resource( GUIDE *tg, int mx, int my, int ks )
{
  OBSPEC key;
  OBJECT *tree;
  int    id, ox, oy, ow, oh;

  if (ks < 0) return; tree = set_tree( tg, 1 );
  if ((id = objc_find( tree, 0, MAX_DEPTH, mx, my )) == -1
      || tree[id].ob_next == -1) return;
  key.free_string = tree[id].ob_spec.free_string;
  switch (tree[id].ob_type & 0xFF)
  {
    case G_TITLE:
    case G_STRING:
    case G_BUTTON:	break;
    case G_TEXT:
    case G_FTEXT:
    case G_BOXTEXT:
    case G_FBOXTEXT:	key.free_string = key.tedinfo->te_ptext; break;
    case G_ICON:	key.free_string = key.iconblk->ib_ptext; break;
    default: return;
  }
  objc_offset( tree, id, &ox, &oy );
  ow = tree[id].ob_width;
  oh = tree[id].ob_height;
  tree += tree[id].ob_next;
  if (tree->ob_type == G_STRING && tree->ob_flags == HIDETREE)
    open_window( 1, ks, ox, oy, ow, oh,
		 tree->ob_spec.free_string, key.free_string );
}

GUIDE *load_resource( int fh, long len )
{
  GUIDE *rg;
  char  *buf;

  if ((rg = Malloc( sizeof(GUIDE) + len )) == 0)
    { Fclose( fh ); form_error( EINVMEM ); return 0; }
  buf = (char *)&rg->header; len -= Fread( fh, sizeof(RSHDR), buf );
#ifdef __TOS__
  if (rg->header.rsh_vrsn != 0x0003)
#else
  if (rg->header.rsh_vrsn != 0x0300)
#endif
  {
    unsigned *src = (unsigned *)(buf += sizeof(RSHDR));
    long *des = (long *)(buf + sizeof(RSHDR));
    int count = 18; do *--des = *--src; while (--count);
  }
  rg->addr = buf; Fread( fh, len, buf + sizeof(RSHDR) ); Fclose( fh );
  if (rg->header.rsh_ntree == 0)
    { Mfree( rg ); form_error( ENOENT ); return 0; }
  fix_objects( &rg->header, buf,
	       rg->flip = rg->header.rsh_ntree & 0xFF00FF00L ? 1 : 0 );
  if (popup->ob_next == 0) { --popup->ob_next; fix_tree( popup, 6 ); }
  rg->xfac = rg->yfac = 8; rg->tree = 0; set_tree( rg, 0 );
  rg->w = rg->bc << 3; rg->h = rg->lc << 3;
  rg->draw = draw_resource; rg->free = free_resource;
  rg->key = key_resource; rg->sclick = sclick_resource;
  rg->dclick = dclick_resource;
  return rg;
}