#include <aes.h>
#include <osbind.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "config.h"	/* Resource-Deklarationen			*/
#pragma warn -rpt
#include "config.rsh"	/* Modifizierte Quell-Resource vom RCS		*/
#pragma warn +rpt

#include "1stguide.h"
#include "util.h"

#define GUIDE int	/* Genaue Struktur interessiert hier nicht.	*/

FROM( sound )    IMPORT GUIDE *load_sample( int fh, long len );
FROM( text )     IMPORT GUIDE *load_text( int fh, long len );
FROM( text )     IMPORT GUIDE *load_dump( int fh, long len );
FROM( text )     IMPORT int   (*out_text( void ))( int fh );
FROM( video )    IMPORT GUIDE *load_mpg( int fh, long len );
FROM( jpicture ) IMPORT GUIDE *load_jpg( int fh, long len );
FROM( jpegddrv ) IMPORT GUIDE *load_png( int fh, long len );
FROM( jpegddrv ) IMPORT GUIDE *load_drv_jpg( int fh, long len,
					     void *jpgddrv_ptr );
FROM( image )    IMPORT GUIDE *load_img( int fh, long len );
FROM( image )    IMPORT GUIDE *load_iff( int fh, long len );
FROM( image )    IMPORT void  out_image( char *name );
FROM( meta )     IMPORT GUIDE *load_meta( int fh, long len );
FROM( meta )     IMPORT int   (*out_meta( char *name, int fh ))( int fh );
FROM( resource ) IMPORT GUIDE *load_resource( int fh, long len );

static void *get_jpgddrv_ptr( void )
{
  static void *ptr = 0;
  static int init_flag = 0;

  if (par.usedsp == 0)
  {
    ptr = 0; init_flag = 0;
  }
  else if (init_flag == 0)
  {
    void *oldstack = (void *)Super( 0 );
    long *cookiejar = *(long **)0x5A0;
    if (cookiejar)
      while (*cookiejar)
      {
	if (*cookiejar++ == '_JPD')
	{
	  ptr = (void *)*cookiejar;
	  break;
	}
	cookiejar++;
      }
    Super( oldstack );
    init_flag++;
  }
  return ptr;
}

GUIDE *load_file( char *name, int fh, long len, int flag )
{
  if (flag == 0) return load_dump( fh, len );
  if ((name = strrchr( name, '.' )) != 0)
  {
    name++;
    if (stricmp( name, "JPG" ) == 0
     || stricmp( name, "JPE" ) == 0
     || stricmp( name, "JPEG" ) == 0)
    {
      if ((name = get_jpgddrv_ptr()) != 0)
	return load_drv_jpg( fh, len, name );
      return load_jpg( fh, len );
    }
    if (stricmp( name, "MPG" ) == 0
     || stricmp( name, "MPE" ) == 0
     || stricmp( name, "MPEG" ) == 0) return load_mpg( fh, len );
    if (stricmp( name, "SAM" ) == 0
     || stricmp( name, "SND" ) == 0
     || stricmp( name, "AVR" ) == 0) return load_sample( fh, len );
    if (stricmp( name, "PNG" ) == 0) return load_png( fh, len );
    if (stricmp( name, "IMG" ) == 0) return load_img( fh, len );
    if (stricmp( name, "IFF" ) == 0) return load_iff( fh, len );
    if (stricmp( name, "GEM" ) == 0) return load_meta( fh, len );
    if (stricmp( name, "RSC" ) == 0) return load_resource( fh, len );
  }
  return load_text( fh, len );
}

void spool( int ks )
{
  static int (*out_file)( int fh ), fh = -1, nextflag = 1;
  static char message[92];
  static DTA outdta;
  char *p, *q;

  if ((~ks & 3) == 0)
    if (form_alert( 2, "[2][1STGUIDE:"
			"|Wollen Sie die Ausgabe"
			"|wirklich abbrechen und den"
			"|Spooler-Puffer leeren?][ Abbruch |Weiter]" ) == 1)
    {
      set_fonts( 0 ); if (fh >= 0) { Fclose( fh ); fh = -1; }
      nextflag = 1; *spool_buf = 0; return;
    }
  if (fh >= 0)
  {
    if ((*out_file)( fh ))
    {
      Fclose( fh ); fh = -1;
      if (nextflag)
      {
	p = strchr( spool_buf, ' ' ); *p++ = 0; strcpy( spool_buf, p );
    } }
    return;
  }
  p = strchr( spool_buf, ' ' ); *p++ = 0;
  if (nextflag) nextflag = Findfirst( spool_buf, &outdta, 0x27 );
  if (nextflag == 0)
  {
    q = strrchr( spool_buf, PATHSEP ) + 1;
    p = memmove( q + strlen( outdta.d_fname ) + 1, p, strlen( p ) + 1 );
    strcpy( q, outdta.d_fname );
    nextflag = Findnext( &outdta );
  }
  if ((q = strrchr( q, '.' )) != 0)
  {
    q++;
    if (stricmp( q, "IMG" ) == 0)
    {
      out_image( spool_buf );
      if (nextflag) strcpy( spool_buf, p ); else p[-1] = ' '; return;
    }
    if (stricmp( q, "GEM" ) == 0)
    {
      if ((fh = Fopen( spool_buf, 0 )) < 0)
      {
	form_error( -31 - fh );
	if (nextflag) strcpy( spool_buf, p ); else p[-1] = ' '; return;
      }
      out_file = out_meta( spool_buf, fh ); p[-1] = ' '; return;
    }
    if (stricmp( q, "JPG" ) == 0
     || stricmp( q, "JPE" ) == 0
     || stricmp( q, "JPEG" ) == 0
     || stricmp( q, "MPG" ) == 0
     || stricmp( q, "MPE" ) == 0
     || stricmp( q, "MPEG" ) == 0
     || stricmp( q, "SAM" ) == 0
     || stricmp( q, "SND" ) == 0
     || stricmp( q, "AVR" ) == 0
     || stricmp( q, "PNG" ) == 0
     || stricmp( q, "IFF" ) == 0
     || stricmp( q, "RSC" ) == 0)
    {
      strcpy( message, "[1][1STGUIDE-Warnung:|Ausgabe von " );
      strcat( message, q );
      strcat( message, "-Dateien|wird noch nicht untersttzt.]"
		       "[ Na sowas ]" );
      form_alert( 1, message );
      if (nextflag) strcpy( spool_buf, p ); else p[-1] = ' '; return;
  } }
  if ((fh = Fopen( spool_buf, 0 )) < 0)
  {
    form_error( -31 - fh );
    if (nextflag) strcpy( spool_buf, p ); else p[-1] = ' '; return;
  }
  out_file = out_text(); p[-1] = ' ';
}

static void file_parameter( char *buf, int save )
{
  char *p;
  int  fh;
  DTA  mydta;
  static int flag = 0;

  strcpy( buf, StartPath );
  if (flag == 0)
    do
    { p = strrchr( buf, PATHSEP ) + 1;
      strcpy( p, "1STGUIDE.AC*" );
      if (Findfirst( buf, &mydta, 0x27 ))
      {
	strcpy( p + 9, "APP" );
	if (Findfirst( buf, &mydta, 0x27 ))
	{
	  strcpy( p + 9, "PRG" );
	  if (Findfirst( buf, &mydta, 0x27 ))
	  {
	    strcpy( p + 9, "GTP" );
	    if (Findfirst( buf, &mydta, 0x27 ))
	    {
	      *p = 0; memswap( Path, buf, 128 );
	      fh = filebox( "1stGuide: Programm finden" );
	      memswap( Path, buf, 128 ); if (fh == 0) return;
	      p = strrchr( buf, PATHSEP ) + 1; if (*p == 0) return;
	      if (Findfirst( buf, &mydta, 0x27 )) break;
      } } } }
      strcpy( p, mydta.d_fname );
    }
    while (0);
  flag = 0;
  if ((fh = Fopen( buf, 2 )) < 0) { form_error( -31 - fh ); return; }
  if (Fseek( par.parameter, fh, 0 ) != par.parameter ||
      Fread( fh, 4, &mydta.d_length ) - 4 ||
      mydta.d_length != par.parameter) form_error( ENOENT );
  else
  {
    ++flag; strcpy( StartPath, buf );
    if (form_alert( 1, save ? "[1][|Voreinstellungen"
			      "|sichern?][OK| Abbruch ]"
			    : "[1][|Zuletzt gesicherte"
			      "|Einstellungen laden?][OK| Abbruch ]" )
	== 1)
    {
      Fseek( par.parameter, fh, 0 );
      if (save) Fwrite( fh, sizeof( par ), &par );
      else Fread( fh, sizeof( par ), &par );
  } }
  Fclose( fh );
}

static void mem_2_string( long value, char *string )
{
  int count;

  value >>= 10; string += 11; count = 8;
  do *--string = '0' + value % 10;
  while (--count && (value /= 10) != 0);
  while (--count > 0) *--string = ' ';
}

static long Mtotal( void )
{
  long total, size;
  void *old, *new;

  total = 0; old = 0;
  while ((size = Mavail()) >= sizeof(void *))
  {
    new = Malloc( size );
    if (new)
    {
      total += size; *(void **)new = old; old = new;
  } }
  while (old) { new = old; old = *(void **)new; Mfree( new ); }
  return total;
}

static void flip_select( int id ) { rs_object[id].ob_state ^= SELECTED; }

static int get_select( int id )
{
  if (rs_object[id].ob_state & SELECTED) return 0;
  if (rs_object[id + 1].ob_state & SELECTED) return 1;
  return 2;
}

void config( int ks )
{
  OBJECT *box;
  int    i, cent[4], clip[4];
  char   *p, sname[16], buf[128];

  if (rs_object->ob_next == 0)
  {
    --rs_object->ob_next; fix_tree( rs_object, NUM_OBS - 1 );
    itostring( nplanes,
	       rs_object[RES1].ob_spec.tedinfo->te_ptext + 7, 3 );
  }
  i = NUM_OBS - 1;
  do rs_object[i].ob_state &= ~SELECTED; while (--i >= 0);
  if (((ks & 10) == 10) ^ par.outdef)
  {
    rs_object[TOALLPAR].ob_flags = SELECTABLE | EXIT;
    rs_object[ALLPAR].ob_flags = HIDETREE;
    rs_object[TOOUTPAR].ob_flags = NONE;
    rs_object[IFSEL].ob_flags  = NONE;
    rs_object[OUTPAR].ob_flags = NONE;
    rs_object[EXTSPOOL].ob_flags = NONE;
    rs_object[IFILE].ob_flags  = NONE;
    rs_object[NUMBER].ob_flags = EDITABLE;
    rs_object[RAND].ob_flags   = EDITABLE; ks = NUMBER;
  }
  else
  {
    rs_object[TOALLPAR].ob_flags = NONE;
    rs_object[ALLPAR].ob_flags = NONE;
    rs_object[TOOUTPAR].ob_flags = SELECTABLE | EXIT;
    rs_object[IFSEL].ob_flags  = SELECTABLE | EXIT;
    rs_object[OUTPAR].ob_flags = HIDETREE;
    rs_object[EXTSPOOL].ob_flags = EDITABLE;
    rs_object[IFILE].ob_flags  = EDITABLE;
    rs_object[NUMBER].ob_flags = NONE;
    rs_object[RAND].ob_flags   = NONE; ks = IFILE;
  }
  wind_get( 0, WF_WORKXYWH, clip, clip + 1, clip + 2, clip + 3 );
  i = clip[0] + clip[2];
  p = itoa( i, rs_object[RES2].ob_spec.tedinfo->te_ptext, 10 );
  while (*p++); --p; if (i < 10000) *p++ = ' '; *p++ = '*';
  i = clip[1] + clip[3]; if (i < 10000) *p++ = ' '; itoa( i, p, 10 );
  mem_2_string( Mavail(), rs_object[MEM].ob_spec.tedinfo->te_ptext );
  mem_2_string( Mtotal(), rs_object[TOT].ob_spec.tedinfo->te_ptext );
  rs_object[EXTSPOOL].ob_spec.tedinfo->te_ptext = sname;
  strcpy( sname, par.extspool );
  rs_object[IFILE].ob_spec.tedinfo->te_ptext = buf;
  strcpy( buf, par.indexfile );
  itoa( par.out_handle, rs_object[NUMBER].ob_spec.tedinfo->te_ptext, 10 );
  itoa( par.margin, rs_object[RAND].ob_spec.tedinfo->te_ptext, 10 );
  flip_select( OUTWIN + par.outdef );
  flip_select( FULLDEF + par.fulldef );
  flip_select( FONTDEF + par.textdef );
  flip_select( DOVER + par.overflag );
  flip_select( USEDITH + par.usedith );
  flip_select( DITHMOD + par.dithmod );
  flip_select( DITHCOL + par.dithcol );
  flip_select( DITHTYP + par.dithtyp );
  if (par.spoolflag) flip_select( OUTEXT );
  if (par.usedsp) flip_select( USEDSP );
  if (par.formfeed) flip_select( FORMFEED );
  flip_select( QNORMAL + par.quality );
  flip_select( PASSEND + par.meta_scale );
  flip_select( PIXEL + par.aspect );
  flip_select( XBRUCH + par.x_scale );
  flip_select( YBRUCH + par.y_scale );
  flip_select( LINKS + par.h_align );
  flip_select( OBEN + par.v_align );
  form_center( rs_object, cent, cent + 1, cent + 2, cent + 3 );
  form_dial( FMD_START, 0, 0, 0, 0, cent[0], cent[1], cent[2], cent[3] );
  i = ROOT;
  for (;;)
  {
    box = rs_object;
    if (ks == NUMBER)	/* wegen Let'emFly !	*/
      memmove( ++box, rs_object, sizeof( rs_object ) - sizeof( OBJECT ) );
    objc_draw( box, i, MAX_DEPTH, clip[0], clip[1], clip[2], clip[3] );
    i = form_do( box, ks );
    if (ks == NUMBER)	/* wegen Let'emFly !	*/
      memmove( rs_object, box, sizeof( rs_object ) - sizeof( OBJECT ) );
    flip_select( i );
    switch (i)
    {
      case TOOUTPAR: i = OUTPAR - ALLPAR + TOALLPAR;
      case TOALLPAR: i += ALLPAR - TOALLPAR; ks = (IFILE + NUMBER) - ks;
	rs_object[TOALLPAR].ob_flags ^= SELECTABLE | EXIT;
	rs_object[TOOUTPAR].ob_flags ^= SELECTABLE | EXIT;
	rs_object[IFSEL].ob_flags ^= SELECTABLE | EXIT;
	rs_object[ALLPAR].ob_flags ^= HIDETREE;
	rs_object[OUTPAR].ob_flags ^= HIDETREE;
	rs_object[EXTSPOOL].ob_flags ^= EDITABLE;
	rs_object[IFILE].ob_flags  ^= EDITABLE;
	rs_object[NUMBER].ob_flags ^= EDITABLE;
	rs_object[RAND].ob_flags   ^= EDITABLE; break;
      case IFSEL: strcpy( Path, buf );
	if (filebox( "1stGuide: Indexdatei w„hlen" )) strcpy( buf, Path );
        i = ROOT; break;
      case SAVE:
      case OK:
	strcpy( par.extspool, sname );
	strcpy( par.indexfile, buf );
	par.out_handle = atoi(rs_object[NUMBER].ob_spec.tedinfo->te_ptext);
	par.margin = atoi( rs_object[RAND].ob_spec.tedinfo->te_ptext );
	par.outdef = get_select( OUTWIN );
	par.fulldef = get_select( FULLDEF );
	par.textdef = get_select( FONTDEF );
	par.overflag = get_select( DOVER );
	par.usedith = get_select( USEDITH );
	par.dithmod = get_select( DITHMOD );
	par.dithcol = get_select( DITHCOL );
	par.dithtyp = get_select( DITHTYP );
	par.spoolflag = rs_object[OUTEXT].ob_state & SELECTED;
	par.usedsp = rs_object[USEDSP].ob_state & SELECTED;
	par.formfeed = rs_object[FORMFEED].ob_state & SELECTED;
	par.quality = get_select( QNORMAL );
	par.meta_scale = get_select( PASSEND );
	par.aspect = get_select( PIXEL );
	par.x_scale = get_select( XBRUCH );
	par.y_scale = get_select( YBRUCH );
	par.h_align = get_select( LINKS );
	par.v_align = get_select( OBEN );
      case LOAD:
	if (i != OK) file_parameter( buf, i - LOAD );
      case ABBRUCH:
        form_dial( FMD_FINISH,0,0,0,0,cent[0], cent[1], cent[2], cent[3] );
        return;
} } }