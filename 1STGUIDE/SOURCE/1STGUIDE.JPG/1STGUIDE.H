#ifndef FROM
#define FROM( MODUL )
#endif

#ifndef IMPORT
#define IMPORT extern
#endif

FROM( 1stguide ) IMPORT char Path[], StartPath[], spool_buf[], VA_Name[];
FROM( 1stguide ) IMPORT int  phys_handle, handle, gl_hchar, nplanes,
			     xdesk, ydesk, wdesk, hdesk, xmax, ymax,
			     ekind, ks, xpixel, ypixel, out_width,
			     out_height, ohandle, VA_Flags;
FROM( 1stguide ) IMPORT void AVSendMessage( int msg, int p3, int p4 );
FROM( 1stguide ) IMPORT void set_name( void );
FROM( 1stguide ) IMPORT void set_allslider( void );
FROM( 1stguide ) IMPORT void full_redraw( void );
FROM( 1stguide ) IMPORT void new_redraw( void );
FROM( 1stguide ) IMPORT void new_size( long force_redraw );
FROM( 1stguide ) IMPORT void goto_line( int line );
FROM( 1stguide ) IMPORT int  filebox( char *titel );
FROM( 1stguide ) IMPORT void open_window( int pw, int over, int x, int y,
			     int w, int h, char *file, char *key );