#ifndef FROM
#define FROM( MODUL )
#endif

#ifndef IMPORT
#define IMPORT extern
#endif

#define PATHSEP '\\'
#define LINK	'Ý'

typedef struct
{
  unsigned char *pbuf, *ebuf, *fbuf, sbuf[4];
  int		handle;
}
FBUF;

FROM( util ) IMPORT struct
{
  long parameter, hertz;
  int  outdef, spoolflag, overflag, textdef, dithcol, dummy,
       out_handle, margin, no_ff, quality, meta_scale,
       aspect, x_scale, y_scale, h_align, v_align,
       xwin, ywin, wwin, hwin,
       xmin, ymin, wmax, hmax,
       font, point;
  char extspool[16], indexfile[128];
}
par;

FROM( util ) IMPORT void printbox( char *name, int count );
FROM( util ) IMPORT void Fbufopen( FBUF *fp );
FROM( util ) IMPORT int  Fgetc( FBUF *fp );
FROM( util ) IMPORT void Fbufcreate( FBUF *fp );
FROM( util ) IMPORT void Fbufflush( FBUF *fp );
FROM( util ) IMPORT void Fputc( int ch, FBUF *fp );
FROM( util ) IMPORT void Fbufclose( FBUF *fp );
FROM( util ) IMPORT int  Findfirst( char *name, DTA *pdta, int attrib );
FROM( util ) IMPORT int  Findnext( DTA *pdta );
FROM( util ) IMPORT void slct_open( void );
FROM( util ) IMPORT int  slct_first( DTA *pdta, int attrib );
FROM( util ) IMPORT int  slct_next( DTA *pdta );
FROM( util ) IMPORT void slct_close( void );
FROM( util ) IMPORT void memswap( char *p, char *q, long count );
FROM( util ) IMPORT void flipwords( char *val, long count );
FROM( util ) IMPORT void fliplongs( int *val, long count, int flag );
FROM( util ) IMPORT void fix_tree( OBJECT *tree, int count );
FROM( util ) IMPORT void itostring( int value, char *string, int count );
FROM( util ) IMPORT void load_fonts( int flag );
FROM( util ) IMPORT void set_fonts( int flag );
FROM( util ) IMPORT int  set_scrp( void );
FROM( util ) IMPORT int  popup_menu( OBJECT *tree, int initob, int mx,
			 int my, int (*draw)( OBJECT *tree, int startob,
			 int depth, int xc, int yc, int wc, int hc ) );