#include <aes.h>
#include <gemdefs.h>
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
FROM( image )    IMPORT GUIDE *load_img( int fh );
FROM( image )    IMPORT GUIDE *load_iff( int fh );
FROM( image )    IMPORT void  out_image( char *name );
FROM( meta )     IMPORT GUIDE *load_meta( int fh, long len );
FROM( meta )     IMPORT int   (*out_meta( char *name, int fh ))( int fh );
FROM( resource ) IMPORT GUIDE *load_resource( int fh, long len );

GUIDE *load_file( char *name, long len, int flag )
{
  int fh;

  if ((fh = Fopen( name, 0 )) < 0) { form_error( -fh - 31 ); return 0; }
  if (flag == 0) return load_dump( fh, len );
  if ((name = strrchr( name, '.' )) != 0)
  {
    name = strupr( name + 1 );
    if (strcmp( name, "IMG" ) == 0) return load_img( fh );
    if (strcmp( name, "IFF" ) == 0) return load_iff( fh );
    if (strcmp( name, "RSC" ) == 0) return load_resource( fh, len );
    if (strcmp( name, "GEM" ) == 0) return load_meta( fh, len );
    if (strcmp( name, "SAM" ) == 0
     || strcmp( name, "SND" ) == 0) return load_sample( fh, len );
  }
  return load_text( fh, len );
}

void spool( int ks )
{
  static int (*out_file)( int fh ), fh = -1, nextflag = 1;
  static DTA outdta;
  static char message[] = "[1][1STGUIDE-Warnung:|Ausgabe von XXX-Dateien"
			  "|wird noch nicht untersttzt.][ Na sowas ]";
  char *p, *q;

  if ((~ks & 3) == 0)
    if (form_alert( 2, "[2][1STGUIDE:"
			"|Wollen Sie die Ausgabe"
			"|wirklich abbrechen und den"
			"|Spooler-Puffer leeren?][Abbruch|Weiter]" ) == 1)
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
  if (nextflag)
  {
    int ret;

    if ((ret = Findfirst( spool_buf, &outdta, 0x27 )) != 0)
    {
      form_error( -ret - 31 ); strcpy( spool_buf, p ); return;
  } }
  p = memmove( (q = strrchr( spool_buf, PATHSEP ) + 1) +
		strlen( outdta.d_fname ) + 1, p, strlen( p ) + 1 );
  strcpy( q, outdta.d_fname ); nextflag = Findnext( &outdta );
  if ((q = strrchr( q, '.' )) != 0)
  {
    q = strupr( q + 1 );
    if (strcmp( q, "IMG" ) == 0)
    {
      out_image( spool_buf );
      if (nextflag) strcpy( spool_buf, p ); else p[-1] = ' '; return;
    }
    if (strcmp( q, "GEM" ) == 0)
    {
      if ((fh = Fopen( spool_buf, 0 )) < 0)
      {
	form_error( -fh - 31 );
	if (nextflag) strcpy( spool_buf, p ); else p[-1] = ' '; return;
      }
      out_file = out_meta( spool_buf, fh ); p[-1] = ' '; return;
    }
    if (strcmp( q, "RSC" ) == 0
     || strcmp( q, "IFF" ) == 0
     || strcmp( q, "SAM" ) == 0
     || strcmp( q, "SND" ) == 0)
    {
      strncpy( message + 34, q, 3 ); form_alert( 1, message );
      if (nextflag) strcpy( spool_buf, p ); else p[-1] = ' '; return;
  } }
  if ((fh = Fopen( spool_buf, 0 )) < 0)
  {
    form_error( -fh - 31 );
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
  {
    p = strrchr( buf, PATHSEP ) + 1;
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
	    if ((fh = Findfirst( buf, &mydta, 0x27 )) != 0)
	    {
	      form_error( -fh - 31 ); return;
	    }
	    p = strrchr( buf, PATHSEP ) + 1;
    } } } }
    strcpy( p, mydta.d_fname );
  }
  flag = 0;
  if ((fh = Fopen( buf, 2 )) < 0) { form_error( -fh - 31 ); return; }
  if (Fseek( par.parameter, fh, 0 ) != par.parameter ||
      Fread( fh, 4, &mydta.d_length ) - 4 ||
      mydta.d_length != par.parameter) form_error( ENOENT );
  else
  {
    ++flag; strcpy( StartPath, buf );
    if (form_alert( 1, save ? "[1][|Voreinstellungen"
			      "|sichern?][   OK   |Abbruch]"
			    : "[1][|Zuletzt gesicherte"
			      "|Einstellungen laden?][   OK   |Abbruch]" )
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
    total += size;
    new = Malloc( size ); *(void **)new = old; old = new;
  }
  while ((new = old) != 0) { old = *(void **)new; Mfree( new ); }
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
  char   sname[16], buf[128];

  i = NUM_OBS - 1;
  if (rs_object->ob_next == 0)
    { --rs_object->ob_next; fix_tree( rs_object, i ); }
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
  mem_2_string( Mavail(), rs_object[MEM].ob_spec.free_string );
  mem_2_string( Mtotal(), rs_object[TOT].ob_spec.free_string );
  rs_object[EXTSPOOL].ob_spec.tedinfo->te_ptext = sname;
  strcpy( sname, par.extspool );
  rs_object[IFILE].ob_spec.tedinfo->te_ptext = buf;
  strcpy( buf, par.indexfile );
  itoa( par.out_handle, rs_object[NUMBER].ob_spec.tedinfo->te_ptext, 10 );
  itoa( par.margin, rs_object[RAND].ob_spec.tedinfo->te_ptext, 10 );
  flip_select( OUTWIN + par.outdef );
  flip_select( OUTINT + par.spoolflag );
  flip_select( DOVER + par.overflag );
  flip_select( FONTDEF + par.textdef );
  flip_select( DITHCOL + par.dithcol );
  flip_select( JA + par.no_ff );
  flip_select( QNORMAL + par.quality );
  flip_select( PASSEND + par.meta_scale );
  flip_select( PIXEL + par.aspect );
  flip_select( XBRUCH + par.x_scale );
  flip_select( YBRUCH + par.y_scale );
  flip_select( LINKS + par.h_align );
  flip_select( OBEN + par.v_align );
  wind_get( 0, WF_WORKXYWH, clip, clip + 1, clip + 2, clip + 3 );
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
      case LOAD: file_parameter( buf, 0 );
        form_dial( FMD_FINISH,0,0,0,0,cent[0], cent[1], cent[2], cent[3] );
        return;
      case SAVE:
      case OK:
	strcpy( par.extspool, sname );
	strcpy( par.indexfile, buf );
	par.out_handle = atoi(rs_object[NUMBER].ob_spec.tedinfo->te_ptext);
	par.margin = atoi( rs_object[RAND].ob_spec.tedinfo->te_ptext );
	par.outdef = get_select( OUTWIN );
	par.spoolflag = get_select( OUTINT );
	par.overflag = get_select( DOVER );
	par.textdef = get_select( FONTDEF );
	par.dithcol = get_select( DITHCOL );
	par.no_ff = get_select( JA );
	par.quality = get_select( QNORMAL );
	par.meta_scale = get_select( PASSEND );
	par.aspect = get_select( PIXEL );
	par.x_scale = get_select( XBRUCH );
	par.y_scale = get_select( YBRUCH );
	par.h_align = get_select( LINKS );
	par.v_align = get_select( OBEN );
	if (i == SAVE) file_parameter( buf, 1 );
      case ABBRUCH:
        form_dial( FMD_FINISH,0,0,0,0,cent[0], cent[1], cent[2], cent[3] );
        return;
} } }