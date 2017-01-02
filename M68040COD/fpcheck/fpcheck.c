#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include "template.h"

/*
 *		Copyright (C) Motorola, Inc. 1990
 *			All Rights Reserved
 *
 *	For details on the license for this file, please see the
 *	file, README.lic, in this same directory.
 */

/*
 *	fpcheck --- run floating point code and check results
 */

#define	COMMENT	'#'	/* comment character */
#define	MAXBUF	1024	/* max. input line */
#define	FSIZE	12	/* number of bytes in an extended precision float */
#define	TESTREG	5	/* which floating point register to use in tests */
			/* TESTREG must match templates */

char	*Filename;
FILE	*Fp;
int	Testno;
char	Vector[MAXBUF];	/* current input test vector */
int	Vec_print;	/* set when Vector has been printed */
int	Errcount;	/* number of errors encountered */
int	Verbose;	/* set to get full fpsr printing */
int	Logging;	/* set to log each testcase to a file */
int	Local;		/* echo/don't echo characters in STANDALONE mode */
int	Ulps_s;		/* Units in the last place to compare (single) */
int	Ulps_d;		/* Units in the last place to compare (double) */
int	Ulps_x;		/* Units in the last place to compare (extended) */

int	In_c;		/* number of input values given (0,1,2) */
char	In_s;		/* size of input(s) */
int	Out_c;		/* number of out values (0,1,2) */
char	Out_s;		/* size of output(s) */
int	Expect_c;	/* number of expected results given (0,1,2) */
char	Expect_s;	/* size of expected results */
int	Equal_seen;	/* set if '=' has been seen */

char	In1[FSIZE];	/* 1st input operand */
char	In2[FSIZE];	/* 2nd input operand */
char	Expect1[FSIZE];	/* 1st expected result */
char	Expect2[FSIZE];	/* 2nd expected result */
char	Out1[FSIZE];	/* 1st actual result */
char	Out2[FSIZE];	/* 2nd actual result */
char	Fp_known[FSIZE];/* known value to load before all tests */
char	Fp_reg[8][FSIZE];/* float registers in/out during test */
int	Fp_ctrl[3];	/* fpiadr,fpcr,fpsr in and out */

int	Gen_in[16];	/* general register snapshot before test */
int	Gen_out[16];	/* ... after test */
short int Ccr_in;	/* Condition codes before test */
short int Ccr_out;	/* ... after test */

struct template *Template;
int	Direction;	/* fmove direction */
int	Dn;		/* set if destination is supposed to be Dn */

#define	Fpcr	Fp_ctrl[0]
#define	Fpsr	Fp_ctrl[1]
#define	Fpiar	Fp_ctrl[2]

int	Statick;	/* Static K-factor (-1 means not specified) */
int	Dynamick;	/* Dynamic K-factor */

/* FPSR Flag bits and fields */
/* If < 0, don't check.  If zero, verify it is clear, If >0 verify it is set */
int	N;
int	Z;
int	I;
int	NAN;
int	Qbyte;
int	BSUN;
int	SNAN;
int	OPERR;
int	OVFL;
int	UNFL;
int	DZ;
int	INEX2;
int	INEX1;
int	AIOP;
int	AOVFL;
int	AUNFL;
int	ADZ;
int	AINEX;

/* FPSR bits */
#define	N_bit		0x08000000
#define	Z_bit		0x04000000
#define	I_bit		0x02000000
#define	NAN_bit		0x01000000
#define	BSUN_bit	0x00008000	/* Also in FPCR */
#define	SNAN_bit	0x00004000	/* Also in FPCR */
#define	OPERR_bit	0x00002000	/* Also in FPCR */
#define	OVFL_bit	0x00001000	/* Also in FPCR */
#define	UNFL_bit	0x00000800	/* Also in FPCR */
#define	DZ_bit		0x00000400	/* Also in FPCR */
#define	INEX2_bit	0x00000200	/* Also in FPCR */
#define	INEX1_bit	0x00000100	/* Also in FPCR */
#define	AIOP_bit	0x00000080
#define	AOVFL_bit	0x00000040
#define	AUNFL_bit	0x00000020
#define	ADZ_bit		0x00000010
#define	AINEX_bit	0x00000008

/* FPCR bits */
#define	RN_mode		0x00	/* default */
#define	RZ_mode		0x10
#define	RM_mode		0x20
#define	RP_mode		0x30
#define	Round_mask	0x30

#define	X_mode		0x00	/* default */
#define	S_mode		0x40
#define	D_mode		0x80
#define	Prec_mask	0xC0

/* Direction settings */
#define	IN		0
#define	OUT		1

/* signal handling variables */
int	Expect_SIGFPE;		/* expecting SIGFPE */
int	Expect_bsun;
int	Expect_inex;
int	Expect_dz;
int	Expect_unfl;
int	Expect_operr;
int	Expect_ovfl;
int	Expect_nan;
int	Expect_unimp;
int	Expect_unsup;

int	Got_SIGFPE;		/* was an exception generated? */
int	Got_bsun;
int	Got_inex;
int	Got_dz;
int	Got_unfl;
int	Got_operr;
int	Got_ovfl;
int	Got_nan;
int	Got_unimp;
int	Got_unsup;

int	TAKEN;		/* for testing branch taken/nottaken */
int	Branch_taken;

/* For STANDALONE operation, this address points to an array of 9 */
/* integers that are incremented when a user exception is generated */
/* See the code in setup_sig() and collect_sig() */
#define	EBASE	0xE0000

char *getitem();
struct template *lookop();
struct template *isop();

#ifndef STANDALONE
/*
 *	fpecatch --- record any SIGFPE's
 *
 *	In the UNIX environment this signal will be received when the FPCR
 *	enables an exception and a floating point instruction results
 *	in that exception being generated.  On Sun/BSD systems the
 *	type of exception is passed in the 'code' argument to
 *	the signal handler.  For most SYSV systems, there is no way to
 *	determine which type of SIGFPE was generated.
 *
 *	Clear NAN bit on BSUN exceptions to prevent infinite SIGFPE's.
 */
fpecatch(sig,code)
int sig;
int code;
{
	signal(SIGFPE,fpecatch);	/* rearm */
	Got_SIGFPE++;
#ifdef SUN
	if( sig != SIGFPE ){
		error("Unexpected signal in fpecatch");
		exit(1);
		}
	switch(code){
		case FPE_FLTBSUN_TRAP:  Got_bsun++; clear_nan(); break;
		case FPE_FLTINEX_TRAP:  Got_inex++; break;
		case FPE_FLTDIV_TRAP:   Got_dz++; break;
		case FPE_FLTUND_TRAP:   Got_unfl++; break;
		case FPE_FLTOPERR_TRAP: Got_operr++; break;
		case FPE_FLTOVF_TRAP:   Got_ovfl++; break;
		case FPE_FLTNAN_TRAP:   Got_nan++; break;
		default:
			printf("??? %d ??? ",code);
			error("Unexpected SIGFPE code in fpecatch");
			exit(1);
		}
#endif
#ifdef R3V6
	if( sig != SIGFPE ){
		error("Unexpected signal in fpecatch");
		exit(1);
		}
	switch(code){
		case KFPBSUN:  Got_bsun++; clear_nan(); break;
		case KFPINEX:  Got_inex++; break;
		case KFPDZ:    Got_dz++; break;
		case KFPUNFL:  Got_unfl++; break;
		case KFPOPER:  Got_operr++; break;
		case KFPOVFL:  Got_ovfl++; break;
		case KFPSNAN:  Got_nan++; break;
		default:
			printf("??? %d ??? ",code);
			error("Unexpected SIGFPE code in fpecatch");
			exit(1);
		}
#endif
}

#endif

/*
 *	illcatch --- SIGILL catcher
 *
 *	Print a message and die
 */
illcatch()
{
	printf("Illegal instruction on: %s\n",Vector);
	exit(1);
}

main(argc,argv)
int argc;
char **argv;
{
	initialize();
#ifdef STANDALONE
	Testno = 0;
	Filename = "";
	dofile();
	_exit();
#else
	signal(SIGFPE,SIG_IGN);
	signal(SIGILL,illcatch);
	if( argc==1 ){
		Testno = 0;
		Filename = "stdin";
		Fp = stdin;
		dofile();
		}
	else while(--argc){
		Testno = 0;
		Filename = *++argv;
		Fp = fopen(Filename,"r");
		if( Fp != NULL ){
			dofile();
			fclose(Fp);
			}
		else
			error("Can't open");
		}
	exit(Errcount);
#endif
}

/*
 *	initialize --- setup globals
 *
 *	Since the program may run standalone, we need to reinit these
 *	variables every time the program starts.
 */
initialize()
{
	Errcount = 0;
	Verbose  = 0;
	Logging  = 0;
	Local    = 0;
	Ulps_s   = 2;
	Ulps_d   = 2;
	Ulps_x   = 2048;

	/* set Fp_known to a QNAN value */
	Fp_known[0]  = 0x7F;
	Fp_known[1]  = 0xFF;
	Fp_known[2]  = 0x00;
	Fp_known[3]  = 0x00;
	Fp_known[4]  = 0xC0;
	Fp_known[5]  = 0xFF;
	Fp_known[6]  = 0xEE;
	Fp_known[7]  = 0xFA;
	Fp_known[8]  = 0xCA;
	Fp_known[9]  = 0xDE;
	Fp_known[10] = 0xC0;
	Fp_known[11] = 0xDA;
}

/*
 *	dofile --- run tests from a file
 *
 *	The file has already been opened.  Call getvector() until
 *	it detects EOF.  Each line is a test to run.
 */
dofile()
{
	while( getvector() ){
		clear_all();	/* start with a default setting */
		if( parse_line(Vector) ){
			setup_sig();
			if(Logging)
				logit();
			dotest();
			collect_sig();
			if( Equal_seen )
				check_all();
			else
				show_all();
			}
#ifdef STANDALONE
		printf("-\n");	/* indicate test has finished */
#endif
		}
}

getvector()
{
	while( fgets(Vector,MAXBUF,Fp) != NULL ){
		Testno++;
		low_string(Vector);
		if( Vector[0] == COMMENT || Vector[0] == '\n')
			printf("%s",Vector);	/* comment or empty line */
		else if( strcmp(Vector,"verbose\n")==0)
			Verbose=1;	/* verbose on */
		else if( strcmp(Vector,"~verbose\n")==0)
			Verbose=0;	/* verbose off */
		else if( strcmp(Vector,"log\n")==0)
			Logging=1;	/* logging on */
		else if( strcmp(Vector,"~log\n")==0)
			Logging=0;	/* logging off */
		else if( strcmp(Vector,"quit\n") == 0 )
			return(0);	/* fake EOF */
		else if( strcmp(Vector,"local\n") == 0 )
			Local = 1;	/* echo on */
		else if( strcmp(Vector,"remote\n") == 0 )
			Local = 0;	/* echo off */
		else{
			Vector[strlen(Vector)-1]='\0';	/* zap newline */
			return(1);	/* probably a test vector */
			}
#ifdef STANDALONE
		printf("-\n");	/* indicate we saw the line */
#endif
		}
	return(0);
}

/*
 *	clear_all --- set all input and output variables to reasonable values
 */
clear_all()
{
	register int i,j;

	Fpcr = Fpsr = Fpiar = 0;
	for(i=0;i<FSIZE;i++)
		In1[i]=In2[i]=Out1[i]=Out2[i]=Expect1[i]=Expect2[i]=0;
	Statick = Dynamick = -1;
	N = Z = I = NAN = Qbyte = -1;
	BSUN = SNAN = OPERR = OVFL = UNFL = DZ = INEX1 = INEX2 = -1;
	AIOP = AOVFL = AUNFL = ADZ = AINEX = -1;
	Template = NULL;
	In_c = Out_c = Expect_c = 0;
	In_s = Out_s = Expect_s = 'x';	/* default assumed size of things */
	Direction = IN;
	Dn = -1;
	Equal_seen = 0;
	Expect_SIGFPE = -1;
	Expect_bsun = Expect_inex = Expect_dz = Expect_unfl = -1;
	Expect_operr = Expect_ovfl = Expect_nan = Expect_unimp = -1;
	Expect_unsup = -1;
	for(i=0;i<8;i++)
		for(j=0;j<FSIZE;j++)
			Fp_reg[i][j] = Fp_known[j];
	TAKEN = -1;
	Branch_taken = 0;
}

/*
 *	setup_sig --- initialize all signal catching variables
 *
 *	If non-STANDALONE, enable SIGFPE's, otherwise clear the
 *	external exception counters.  Always clear the Got_* counters.
 */
setup_sig()
{
#ifdef STANDALONE
	int *ep = (int *)EBASE;
	int i;

	/* clear external counters */
	for(i=0;i<9;i++)
		*ep++ = 0;
#else
	signal(SIGFPE,fpecatch);
#endif
	Got_SIGFPE = 0;
	Got_bsun = Got_inex = Got_dz = Got_unfl = 0;
	Got_operr = Got_ovfl = Got_nan = Got_unimp = Got_unsup = 0;
}

/*
 * 	collect_sig --- gather any exceptions that were generated
 *
 *	If STANDALONE, copy the external counters into the Got_*
 *	variables and sets Got_SIGFPE to the sum of all counters.
 *	The addresses of the external counters depend on the way
 *	the floating point emulation package is assembled.
 *
 *	For non-STANDALONE operation, ignore further SIGFPE's that
 *	may be generated when the results are printed.
 */
collect_sig()
{
#ifdef STANDALONE
	int	*ep = (int *)EBASE;

	Got_unimp  = *ep++;
	Got_bsun   = *ep++;
	Got_inex   = *ep++;
	Got_dz     = *ep++;
	Got_unfl   = *ep++;
	Got_ovfl   = *ep++;
	Got_operr  = *ep++;
	Got_nan    = *ep++;
	Got_unsup  = *ep++;
	Got_SIGFPE = Got_unimp + Got_bsun + Got_inex + Got_dz + Got_unfl +
			Got_ovfl + Got_operr + Got_nan + Got_unsup;
#else
	signal(SIGFPE,SIG_IGN);
#endif
}

/*
 *	parse_line --- break input into tokens, assign to relevant variables
 */
parse_line(s)
char *s;
{
	char	op[MAXBUF];
	int	opseen=0;
	char	item[MAXBUF];
	char	*cptr = In1;

	while( (s=getitem(s,item)) ){
		if( strcmp(item,"=")==0 ){
			Equal_seen=1;
			cptr=Expect1;
			}
		else if( iskeyword(item) )
			;
		else if( isconstant(item,cptr) ){
			if( cptr == In1 ){
				In_c=1;
				cptr = In2;
				}
			else if( cptr == In2 )
				In_c=2;
			if( cptr == Expect1 ){
				Expect_c=1;
				cptr = Expect2;
				}
			else if( cptr == Expect2 )
				Expect_c=2;
			}
		else{
			if( opseen ){
				error("Unknown item");
				return(0);
				}
			strcpy(op,item);	/* remember till end */
			opseen = 1;
			}
		}
	if( !opseen ){
		error("No testname");
		return(0);
		}
	Template = isop(op);
	if( Template == NULL ){
		error("Unknown testname");
		return(0);
		}
	return(1);
}

struct template *
lookop(s)
register char *s;
{
	register struct template *p;
	extern struct template Allops[];

	for(p=Allops; p->t_type != T_END; p++)
		if( strcmp( p->t_name, s ) == 0)
			return(p);
	return(NULL);
}

/*
 *	isop --- lookup name in table, checking for special cases
 *
 *	There are a few cases that require special handling:
 *
 *	By default, any fmove is assumed to be a move in, the float register
 *	is the destination, and 'size' is the size of the 'memory'
 *	resident source.  To select the fmove out case, the test vector
 *	will contain the keyword 'out'.  If this is detected, then
 *	we rescan Allops for tests with the name 'fmoveo.?'
 *
 *	If we see 'fmove.p out' then we switch to one of the
 *	static or dynamic k-factor templates.  They are named
 *	fmoveps.%x and fmovepd.%x (%x=00-7f).
 *
 *	fmovecr.x with a single byte argument will be turned into
 *	a template name of fmovecr.%x (%x=00-3f).
 *
 *	fmove.[bwls] out with 'DN' keyword selects Dn as a destination.
 *	Change the testname to fmoveod.[bwls] in this case.
 *
 *	If the testname is one of the 68040-only opcodes and the
 *	template table doesn't include them (because we're generating
 *	an fpcheck for an '881/'882 machine) then rewrite the test
 *	to one that will work on the '881/'882.
 *
 *	If none of the above cases are detected, the name is expected
 *	to be found by lookop().
 */
struct template *
isop(name)
char *name;
{
	struct template *t;
	char newop[30];

	if( strcmp(name,"fmovecr.x")==0 )
		sprintf(newop,"fmovecr.%x",In1[FSIZE-1]&0x7F);
	else if(Direction==OUT && strncmp(name,"fmove.",6)==0 ){
		switch(name[6]){
		case 'p':
			if( Statick >= 0 )
				sprintf(newop,"fmoveps.%x",Statick);
			else if( Dynamick >= 0 )
				sprintf(newop,"fmovepd.%x",Dynamick);
			else{
				error("fmove.p(out) must have K#:nn or KD:nn");
				return(NULL);
				}
			break;
		case 'b':
		case 'w':
		case 'l':
		case 's':	
			/* Use data register as destination? */
			if( Dn>=0 && Dn<=7 ){
				strcpy(newop,"fmoveod.?");
				newop[8] = name[6];
				break;
				}
		default:
			/* the non-packed move out case */
			strcpy(newop,"fmoveo.?");
			newop[7] = name[6];	/* keep same size */
			break;
			}
		}
	else if(Direction==OUT && 
		(strncmp(name,"fsmove.",6)==0 || strncmp(name,"fdmove.",6)==0) )
		return(NULL);	/* illegal combination */
	else if( (t=lookop(name)) != NULL)
		return(t);
	else if( is040name(name) ){
		/* failed to find F[sd]OP in templates, so fake it */
		newop[0] = 'f';
		strcpy(&newop[1],&name[2]);
		switch(name[1]){
		case 's':
			Fpcr=(Fpcr&(~Prec_mask))|S_mode;
			break;
		case 'd':
			Fpcr=(Fpcr&(~Prec_mask))|D_mode;
			break;
			}
		}
	else
		return(NULL);

	return(lookop(newop));
}

/*
 *	is040name --- is this a 68040 only floating point instruction?
 */
is040name(s)
register char *s;
{
	if( *s++ != 'f' ) return(0);
	if( (*s!='s') && (*s!='d') )return(0);
	s++;
	if( strncmp(s,"add.",4)==0 )return(1);
	if( strncmp(s,"sub.",4)==0 )return(1);
	if( strncmp(s,"mul.",4)==0 )return(1);
	if( strncmp(s,"div.",4)==0 )return(1);
	if( strncmp(s,"abs.",4)==0 )return(1);
	if( strncmp(s,"neg.",4)==0 )return(1);
	if( strncmp(s,"sqrt.",5)==0 )return(1);
	if( strncmp(s,"move.",5)==0 && Direction!=OUT )return(1);
	return(0);
}

/*
 *	iskeyword --- check item as possible reserved keyword flag
 *
 *	If any match is found, set the appropriate bits.  The global
 *	Equal_seen is set if the '=' sign has been seen.  This
 *	is used to decide how to handle reserved words that can
 *	appear on either side of the '='.
 */
iskeyword(s)
char *s;
{
	int qtmp;

	if(Equal_seen){
		     if( strcmp(s,"bsun")==0 ){ BSUN=1; }
		else if( strcmp(s,"~bsun")==0 ){ BSUN=0; }
		else if( strcmp(s,"snan")==0 ){ SNAN=1; }
		else if( strcmp(s,"~snan")==0 ){ SNAN=0; }
		else if( strcmp(s,"operr")==0 ) { OPERR=1; }
		else if( strcmp(s,"~operr")==0 ){ OPERR=0; }
		else if( strcmp(s,"ovfl")==0 ) { OVFL=1; }
		else if( strcmp(s,"~ovfl")==0 ){ OVFL=0; }
		else if( strcmp(s,"unfl")==0 ) { UNFL=1; }
		else if( strcmp(s,"~unfl")==0 ){ UNFL=0; }
		else if( strcmp(s,"dz")==0 ) { DZ=1; }
		else if( strcmp(s,"~dz")==0 ){ DZ=0; }
		else if( strcmp(s,"inex2")==0 ) { INEX2=1; }
		else if( strcmp(s,"~inex2")==0 ){ INEX2=0; }
		else if( strcmp(s,"inex1")==0 ) { INEX1=1; }
		else if( strcmp(s,"~inex1")==0 ){ INEX1=0; }
		else if( strcmp(s,"n")==0 ) { N=1; }
		else if( strcmp(s,"~n")==0 ){ N=0; }
		else if( strcmp(s,"z")==0 ) { Z=1; }
		else if( strcmp(s,"~z")==0 ){ Z=0; }
		else if( strcmp(s,"i")==0 ) { I=1; }
		else if( strcmp(s,"~i")==0 ){ I=0; }
		else if( strcmp(s,"nan")==0 ) { NAN=1; }
		else if( strcmp(s,"~nan")==0 ){ NAN=0; }
		else if( strcmp(s,"aiop")==0 ){ AIOP=1; }
		else if( strcmp(s,"~aiop")==0 ){ AIOP=0; }
		else if( strcmp(s,"aovfl")==0 ){ AOVFL=1; }
		else if( strcmp(s,"~aovfl")==0 ){ AOVFL=0; }
		else if( strcmp(s,"aunfl")==0 ){ AUNFL=1; }
		else if( strcmp(s,"~aunfl")==0 ){ AUNFL=0; }
		else if( strcmp(s,"adz")==0 ){ ADZ=1; }
		else if( strcmp(s,"~adz")==0 ){ ADZ=0; }
		else if( strcmp(s,"ainex")==0 ){ AINEX=1; }
		else if( strcmp(s,"~ainex")==0 ){ AINEX=0; }
		else if( strcmp(s,"sigfpe")==0 ){ Expect_SIGFPE=1; }
		else if( strcmp(s,"~sigfpe")==0 ){ Expect_SIGFPE=0; }
		else if( strcmp(s,"tbsun")==0 ){ Expect_bsun=1; }
		else if( strcmp(s,"~tbsun")==0 ){ Expect_bsun=0; }
		else if( strcmp(s,"tinex")==0 ){ Expect_inex=1; }
		else if( strcmp(s,"~tinex")==0 ){ Expect_inex=0; }
		else if( strcmp(s,"tdz")==0 ){ Expect_dz=1; }
		else if( strcmp(s,"~tdz")==0 ){ Expect_dz=0; }
		else if( strcmp(s,"tunfl")==0 ){ Expect_unfl=1; }
		else if( strcmp(s,"~tunfl")==0 ){ Expect_unfl=0; }
		else if( strcmp(s,"toperr")==0 ){ Expect_operr=1; }
		else if( strcmp(s,"~toperr")==0 ){ Expect_operr=0; }
		else if( strcmp(s,"tovfl")==0 ){ Expect_ovfl=1; }
		else if( strcmp(s,"~tovfl")==0 ){ Expect_ovfl=0; }
		else if( strcmp(s,"tnan")==0 ){ Expect_nan=1; }
		else if( strcmp(s,"~tnan")==0 ){ Expect_nan=0; }
		else if( strcmp(s,"tunimp")==0 ){ Expect_unimp=1; }
		else if( strcmp(s,"~tunimp")==0 ){ Expect_unimp=0; }
		else if( strcmp(s,"tunsup")==0 ){ Expect_unsup=1; }
		else if( strcmp(s,"~tunsup")==0 ){ Expect_unsup=0; }
		else if( strncmp(s,"q:",2)==0 ){ sscanf(s+2,"%x",&Qbyte); }
		else if( strcmp(s,"t")==0 ){ TAKEN=1; }
		else if( strcmp(s,"~t")==0 ){ TAKEN=0; }
		else if( strcmp(s,"f")==0 ){ TAKEN=0; }	/* alias for ~t */
		else return(0);
		}
	else{
		     if( strcmp(s,"rn")==0 ) Fpcr=(Fpcr&(~Round_mask))|RN_mode; 
		else if( strcmp(s,"rz")==0 ) Fpcr=(Fpcr&(~Round_mask))|RZ_mode;
		else if( strcmp(s,"rm")==0 ) Fpcr=(Fpcr&(~Round_mask))|RM_mode;
		else if( strcmp(s,"rp")==0 ) Fpcr=(Fpcr&(~Round_mask))|RP_mode;
		else if( strcmp(s,"x")==0 )  Fpcr=(Fpcr&(~Prec_mask))|X_mode;
		else if( strcmp(s,"d")==0 )  Fpcr=(Fpcr&(~Prec_mask))|D_mode;
		else if( strcmp(s,"s")==0 )  Fpcr=(Fpcr&(~Prec_mask))|S_mode;
		else if( strcmp(s,"bsun")==0 ) Fpcr |= BSUN_bit;
		else if( strcmp(s,"~bsun")==0 );	/* ignore */
		else if( strcmp(s,"snan")==0 ) Fpcr |= SNAN_bit;
		else if( strcmp(s,"~snan")==0 );	/* ignore */
		else if( strcmp(s,"operr")==0 ) Fpcr |= OPERR_bit;
		else if( strcmp(s,"~operr")==0 );	/* ignore */
		else if( strcmp(s,"ovfl")==0 ) Fpcr |= OVFL_bit;
		else if( strcmp(s,"~ovfl")==0 );	/* ignore */
		else if( strcmp(s,"unfl")==0 ) Fpcr |= UNFL_bit;
		else if( strcmp(s,"~unfl")==0 );	/* ignore */
		else if( strcmp(s,"dz")==0 ) Fpcr |= DZ_bit;
		else if( strcmp(s,"~dz")==0 );		/* ignore */
		else if( strcmp(s,"inex2")==0 ) Fpcr |= INEX2_bit;
		else if( strcmp(s,"~inex2")==0 );	/* ignore */
		else if( strcmp(s,"inex1")==0 ) Fpcr |= INEX1_bit;
		else if( strcmp(s,"~inex1")==0 );	/* ignore */

		else if( strcmp(s,"n")==0 ) Fpsr |= N_bit;
		else if( strcmp(s,"~n")==0 );	/* ignore */
		else if( strcmp(s,"z")==0 ) Fpsr |= Z_bit;
		else if( strcmp(s,"~z")==0 );	/* ignore */
		else if( strcmp(s,"i")==0 ) Fpsr |= I_bit;
		else if( strcmp(s,"~i")==0 );	/* ignore */
		else if( strcmp(s,"nan")==0 ) Fpsr |= NAN_bit;
		else if( strcmp(s,"~nan")==0 );	/* ignore */

		else if( strcmp(s,"out")==0 ) Direction=OUT; 
		else if( strcmp(s,"in")==0 ) Direction=IN; 
		else if( strcmp(s,"dn")==0 ) Dn=0; /* default Dn is D0 */
		else if( strcmp(s,"d0")==0 ) Dn=0; 
		else if( strcmp(s,"d1")==0 ) Dn=1; 
		else if( strcmp(s,"d2")==0 ) Dn=2; 
		else if( strcmp(s,"d3")==0 ) Dn=3; 
		else if( strcmp(s,"d4")==0 ) Dn=4; 
		else if( strcmp(s,"d5")==0 ) Dn=5; 
		else if( strcmp(s,"d6")==0 ) Dn=6; 
		else if( strcmp(s,"d7")==0 ) Dn=7; 
		else if( strncmp(s,"k#:",3)==0 || strncmp(s,"ks:",3)==0 ){
			sscanf(s+3,"%d",&Statick);
			Statick &= 0x7F;	/* chop to 7 bits */
			}
		else if( strncmp(s,"kd:",3)==0 ){
			sscanf(s+3,"%d",&Dynamick);
			Dynamick &= 0x7F;	/* chop to 7 bits */
			}
		else if( strncmp(s,"q:",2)==0 ){
			sscanf(s+2,"%x",&qtmp);
			Fpsr |= (qtmp&0xFF)<<16;
			}
		else return(0);
		}
	return(1);

}

/*
 *	isconstant --- scan item for valid hex constant
 *
 *	'result' is a pointer to a place big enough to hold an FSIZE byte
 *	value.  The constant will be right justified in the result
 *	array.
 *
 *	In order to allow using the output from a previous run to be used
 *	as input, a string of the pattern (.*) is skipped.  '_' is also
 *	ignored to allow breaking long constants into more readable pieces.
 *
 *	A leading sequence of [S] is detected and is used to convert
 *	the constant to extended precision. S is one of 'bwldsxp'.
 */
isconstant(s,result)
register char *s;
char *result;
{
	int scantmp[FSIZE*4];	/* for each nybble of the converted constant */
	int next = FSIZE*2;
	char cflag = '\0';
	register int i;

	for(i=0;i<(FSIZE*4);i++)
		scantmp[i] = 0;

	if(*s=='['){
		s++;
		cflag = *s++;
		if( *s++ != ']' ){
			error("Bad []");
			return(0);
			}
		}

	while(*s){
		if( *s == '(' ){
			while( *s && *s++ != ')' )
				;
			continue;
			}
		if( *s == '_' ){
			s++;
			continue;
			}
		if( next >= (FSIZE*4) ){
			error("Too many digits for constant");
			return(0);
			}
		if( isdigit(*s) )
			scantmp[next++] = *s++ - '0';
		else if( *s <= 'f' && *s >= 'a' )
			scantmp[next++] = (*s++ - 'a')+10;
		else
			return(0);
		}
	next -= FSIZE*2;
	if(next==0){
		error("Empty constant");
		return(0);	/* can't happen ??? */
		}
	/* now combine the nybbles and put them into 'result' */
	for(i=0;i<FSIZE;i++){
		*result++ = (scantmp[next]<<4) + scantmp[next+1];
		next += 2;
		}

	/* convert from smaller size to extended if we were told to */
	result -= FSIZE;
	switch(cflag){
	case 'b': btox(result); break;
	case 'w': wtox(result); break;
	case 'l': ltox(result); break;
	case 's': stox(result); break;
	case 'd': dtox(result); break;
	case 'x': break;
	case 'p': ptox(result); break;
	case '\0': break;
	default:
		error("Unknown size for []");
		return(0);
		}
	return(1);
}

/*
 *	getitem --- scan next item from string and put in 'item'
 *
 *	Skips leading whitespace and collects everything into the
 *	item array.  Returns a pointer to just after the end of the
 *	scanned item, or NULL if the end of string has been reached.
 *
 *	The '=' case is special.  No whitespace is required before or
 *	after it.
 */
char *getitem(s,item)
register char *s;
register char *item;
{
	while( s!=NULL && *s && isspace(*s) )	/* skip leading whitespace */
		s++;
	if( s==NULL || *s == '\0' ){	/* end of string reached */
		strcpy(item,"");
		return(NULL);
		}
	if( *s == '=' ){	/* special case, equal sign is whitespace too */
		strcpy(item,"=");
		return(s+1);
		}
	while( *s && !isspace(*s) && *s != '=' )
		*item++ = *s++;
	*item = '\0';
	return(s);
}

/*
 *	dotest --- run the test
 *
 *	Everything is ready to go.  Just need to call the appropriate
 *	test template.
 */
dotest()
{
	register int idx;
	char size;

	size = Template->t_name[strlen(Template->t_name)-1];

	switch(size){
		case 'b': idx = 11; break;
		case 'w': idx = 10; break;
		case 'l':
		case 's': idx = 8;  break;
		case 'd': idx = 4;  break;
		case 'x':
		case 'p':
		default: idx = 0;  break;
		}
	switch(Template->t_type){
	case T_MONADIC:
		In_s = size;
		(*Template->t_func)(In1+idx);
		copyx(Fp_reg[TESTREG],Out1);
		copyx(Fp_known,Fp_reg[TESTREG]);
		Out_c = 1;
		break;
	case T_DYADIC:
		In_s = size;
		copyx(In1,Fp_reg[TESTREG]);
		(*Template->t_func)(In2+idx);
		copyx(Fp_reg[TESTREG],Out1);
		copyx(Fp_known,Fp_reg[TESTREG]);
		Out_c = 1;
		break;
	case T_SINCOS:
		In_s = size;
		(*Template->t_func)(In1+idx);
		copyx(Fp_reg[TESTREG],Out1);
		copyx(Fp_reg[TESTREG+1],Out2);
		copyx(Fp_known,Fp_reg[TESTREG]);
		copyx(Fp_known,Fp_reg[TESTREG+1]);
		Out_c = 2;
		break;
	case T_MOVEO:
		In_s = 'x';
		copyx(In1,Fp_reg[TESTREG]);
		(*Template->t_func)(Out1+idx);
		copyx(Fp_known,Fp_reg[TESTREG]);
		Out_s = size;
		Out_c = 1;
		break;
	case T_MOVEOD:
		In_s = 'x';
		copyx(In1,Fp_reg[TESTREG]);
		(*Template->t_func)(Out1+idx,Dn);
		*(int *)&Out1[FSIZE-4] = Gen_out[Dn];	/* copy Dn to result */
		Gen_out[Dn] = Gen_in[Dn]; /* restore to avoid error message */
		copyx(Fp_known,Fp_reg[TESTREG]);
		Out_s = size;
		Out_c = 1;
		break;
	case T_MOVEPS:
		In_s = 'x';
		copyx(In1,Fp_reg[TESTREG]);
		(*Template->t_func)(Out1);
		copyx(Fp_known,Fp_reg[TESTREG]);
		Out_s = 'p';
		Out_c = 1;
		break;
	case T_MOVEPD:
		In_s = 'x';
		copyx(In1,Fp_reg[TESTREG]);
		(*Template->t_func)(Out1);
		copyx(Fp_known,Fp_reg[TESTREG]);
		Out_s = 'p';
		Out_c = 1;
		break;
	case T_MOVECR:
		In_s = 'b';	/* sort of */
		(*Template->t_func)();
		copyx(Fp_reg[TESTREG],Out1);
		copyx(Fp_known,Fp_reg[TESTREG]);
		Out_c = 1;
		break;
	case T_TST:
		In_s = size;
		(*Template->t_func)(In1+idx);
		Out_c = 0;
		break;
	case T_BCC:
		In_s = size;
		(*Template->t_func)();
		Out_c = 0;
		break;
	default:
		error("Unknown template type");
		break;
	}
}

/*
 *	copyx --- copy extended precision value
 */
copyx(from,to)
register char *from;
register char *to;
{
	register int i = FSIZE;

	while(i--)
		*to++ = *from++;
}

/*
 *	check_all --- verify test results
 *
 *	Check includes tests for results, FPSR bits and exception counters.
 *	The FP regs should all be equal to the 'Fp_known' value too.
 *	Only the SIGFPE exception counter is checked if the program
 *	was compiled for native SYSV mode.
 */
check_all()
{
	Vec_print=0;
	if( Expect_c > 0 && Out_c > 0)
		check_result(Expect1,Out1);
		
	if( Expect_c > 1 && Out_c > 1)
		check_result(Expect2,Out2);

	check_fpsr(N,N_bit,"N","~N");
	check_fpsr(Z,Z_bit,"Z","~Z");
	check_fpsr(I,I_bit,"I","~I");
	check_fpsr(NAN,NAN_bit,"NAN","~NAN");
	check_fpsr(BSUN,BSUN_bit,"BSUN","~BSUN");
	check_fpsr(SNAN,SNAN_bit,"SNAN","~SNAN");
	check_fpsr(OPERR,OPERR_bit,"OPERR","~OPERR");
	check_fpsr(OVFL,OVFL_bit,"OVFL","~OVFL");
	check_fpsr(UNFL,UNFL_bit,"UNFL","~UNFL");
	check_fpsr(DZ,DZ_bit,"DZ","~DZ");
	check_fpsr(INEX2,INEX2_bit,"INEX2","~INEX2");
	check_fpsr(INEX1,INEX1_bit,"INEX1","~INEX1");
	check_fpsr(AIOP,AIOP_bit,"AIOP","~AIOP");
	check_fpsr(AOVFL,AOVFL_bit,"AOVFL","~AOVFL");
	check_fpsr(AUNFL,AUNFL_bit,"AUNFL","~AUNFL");
	check_fpsr(ADZ,ADZ_bit,"ADZ","~ADZ");
	check_fpsr(AINEX,AINEX_bit,"AINEX","~AINEX");
	check_count(Qbyte,(Fpsr>>16)&0xFF,"Quotient Byte");

	check_count(TAKEN,Branch_taken,"Conditional Branch");
	check_count(Expect_SIGFPE,Got_SIGFPE,"SIGFPE");
#if SUN || STANDALONE || R3V6
	check_count(Expect_bsun,Got_bsun,"TBSUN");
	check_count(Expect_inex,Got_inex,"TINEX");
	check_count(Expect_dz,Got_dz,"TDZ");
	check_count(Expect_unfl,Got_unfl,"TUNFL");
	check_count(Expect_operr,Got_operr,"TOPERR");
	check_count(Expect_ovfl,Got_ovfl,"TOVFL");
	check_count(Expect_nan,Got_nan,"TNAN");
	check_count(Expect_unimp,Got_unimp,"TUNIMP");
	check_count(Expect_unsup,Got_unsup,"TUNSUP");
#endif
	checkx(Fp_known,Fp_reg[0],"FP0");
	checkx(Fp_known,Fp_reg[1],"FP1");
	checkx(Fp_known,Fp_reg[2],"FP2");
	checkx(Fp_known,Fp_reg[3],"FP3");
	checkx(Fp_known,Fp_reg[4],"FP4");
	checkx(Fp_known,Fp_reg[5],"FP5");
	checkx(Fp_known,Fp_reg[6],"FP6");
	checkx(Fp_known,Fp_reg[7],"FP7");

	checkg(Gen_in[0],Gen_out[0],"D0");
	checkg(Gen_in[1],Gen_out[1],"D1");
	checkg(Gen_in[2],Gen_out[2],"D2");
	checkg(Gen_in[3],Gen_out[3],"D3");
	checkg(Gen_in[4],Gen_out[4],"D4");
	checkg(Gen_in[5],Gen_out[5],"D5");
	checkg(Gen_in[6],Gen_out[6],"D6");
	checkg(Gen_in[7],Gen_out[7],"D7");
	checkg(Gen_in[8],Gen_out[8],"A0");
	checkg(Gen_in[9],Gen_out[9],"A1");
	checkg(Gen_in[10],Gen_out[10],"A2");
	checkg(Gen_in[11],Gen_out[11],"A3");
	checkg(Gen_in[12],Gen_out[12],"A4");
	checkg(Gen_in[13],Gen_out[13],"A5");
	checkg(Gen_in[14],Gen_out[14],"A6");
	checkg(Gen_in[15],Gen_out[15],"A7");
	checkg(Ccr_in,Ccr_out,"CCR");

	if(Vec_print)
		printf("\n");
}

/*
 *	check_fpsr --- verify fpsr bit is set/not set
 *
 *	If flag is 1 the bit should be set.  If flag is zero the bit
 *	should be clear.  Any other value of flag will skip the check.
 */
check_fpsr(flag,bit,name,notname)
int flag;
int bit;
char *name;
char *notname;
{
	if( flag==1 && (bit&Fpsr)==0 )check_fail(name);
	else if( flag==0 && (bit&Fpsr)!=0 )check_fail(notname);
}

/*
 *	check_count --- compare expected counter value with actual
 *
 *	A negative expected count never fails.
 */
check_count(expect,got,name)
int expect;
int got;
char *name;
{
	if( expect >=0 && expect != got )
		check_fail(name);
}

/*
 *	checkx --- compare 2 extended precision numbers for exact equality 
 */
checkx(expect,got,name)
register unsigned int *expect;
register unsigned int *got;
char *name;
{
	register int i = FSIZE;
	register char *a = (char *)expect;
	register char *b = (char *)got;

	while(i--)
		if( *a++ != *b++ ){
			check_fail(name);
			printf("\t\t%08x_%08x_%08x(expected) != %08x_%08x_%08x(got)\n",
				*expect,*(expect+1),*(expect+2),*got,*(got+1),*(got+2));
			break;
			}
}

/*
 *	checkg --- compare 2 integers for exact equality 
 */
checkg(expect,got,name)
register int expect;
register int got;
char *name;
{
	if( expect != got ){
		check_fail(name);
		printf("\t\t%08x(expected) != %08x(got)\n",expect,got);
		}
}

/*
 *	check_result --- compare the expected result with the actual
 *
 *	The check passes if the difference in the 2 numbers is
 *	less than or equal to the Ulps value.  For exact compares,
 *	Ulps should be zero.  To check extended precision results to
 *	double precision, set Ulps=2048.
 */
check_result(e,a)
register char *e;	/* expected */
register char *a;	/* actual output */
{
	register int diff;
	int eint,aint;	/* expected/actual as integers */
	int ulps = 0;	/* default ulps is for exact compare */
	unsigned int e_hi,e_lo;	/* mantissa of expected */
	unsigned int a_hi,a_lo;	/* mantissa of actual */
	int e_exp,a_exp;	/* exponents of expected and actual */

	switch(Out_s){
	case 'b':
		eint = *(e+11);
		aint = *(a+11);
		eint &= 0xFF;
		aint &= 0xFF;
		diff = eint - aint;
		break;
	case 'w':
		eint = *(short int *)(e+10);
		aint = *(short int *)(a+10);
		eint &= 0xFFFF;
		aint &= 0xFFFF;
		diff = eint - aint;
		break;
	case 'l':
		eint = *(int *)(e+8);
		aint = *(int *)(a+8);
		diff = eint - aint;
		break;
	case 's':
		diff = sub96(e,a);
		ulps = Ulps_s;
		break;
	case 'd':
		diff = sub96(e,a);
		ulps = Ulps_d;
		break;
	case 'x':
		e_exp= (*(short int *)(e))&0x7FFF;
		e_hi = *(unsigned int *)(e+4);
		e_lo = *(unsigned int *)(e+8);
		a_exp= (*(short int *)(a))&0x7FFF;
		a_hi = *(unsigned int *)(a+4);
		a_lo = *(unsigned int *)(a+8);
		switch(Fpcr&Prec_mask){
		case X_mode:
			ulps = Ulps_x;
			break;
		case D_mode:
			/* shift mantissas right by 11 */
			a_lo >>= 11; a_lo |= (a_hi&0x7ff)<<21; a_hi >>= 11;
			e_lo >>= 11; e_lo |= (e_hi&0x7ff)<<21; e_hi >>= 11;
			ulps = Ulps_d;
			break;
		case S_mode:
			/* shift mantissas right by 40 */
			a_lo = a_hi>>8; a_hi = 0;
			e_lo = e_hi>>8; e_hi = 0;
			ulps = Ulps_s;
			break;
			}

		switch(e_exp-a_exp){
		case 0:
			break;
		case 1:
			/* shift actual right by one */
			a_lo >>= 1;
			if( a_hi&1 )
				a_lo |= 0x80000000;
			a_hi >>= 1;
			break;
		case -1:
			/* shift expected right by one */
			e_lo >>= 1;
			if( e_hi&1 )
				e_lo |= 0x80000000;
			e_hi >>= 1;
			break;
		default:
			/* force out of range comparison */
			e_hi = e_lo = a_hi = 0;
			a_lo = (1<<31)-1;
			break;
			}
		diff = diff64(e_hi,e_lo,a_hi,a_lo);
		break;
	case 'p':
		diff = sub96(e,a);
		ulps = 1;
		break;
		}

	if(Template->t_exact)
		ulps = 0;	/* if exact flag is set, force an exact match */

	if( diff<0 )
		diff = -diff;
	if( diff>ulps )
		bad_result(e,a,diff);
}

/*
 *	diff64 --- subtract two 64-bit integers, return difference
 *
 *	After subtracting, the function returns an integer
 *	which is the difference.  If the
 *	difference is more than will fit in an integer, a large
 *	positive value is returned that should cause the program
 *	to print a 'Check failed: Result' message. 
 */
diff64(ahi,alo,bhi,blo)
register int ahi;
register int alo;
register int bhi;
register int blo;
{
	int	result[2];

	sub64(ahi,alo,bhi,blo,result);	/* assembly language 64-bit subtract */
	if( result[0]==0 || result[0]== -1)
		return(result[1]);
	return((1<<31)-1);
}

/*
 *	sub96 --- subtract two 96-bit integers
 *
 *	After subtracting, the function returns an integer
 *	which is the absolute value of the difference.  If the
 *	difference is more than will fit in an integer, MAXINT
 *	is returned.
 */
sub96(a,b)
register char *a;
register char *b;
{
	int	c[FSIZE+1];	/* NOTE: int, not char array */
	int	result[3];	/* compacted result */
	register int i;

	c[FSIZE] = 0;
	for(i=FSIZE-1;i>=0;i--){
		c[i] = (a[i]&0xFF)-(b[i]&0xFF);
		if( c[i+1] < 0 )
			c[i]--;
		}
	/* pack c[] into result[] */
	result[0]  = (c[0]&0xFF)<<24;
	result[0] += (c[1]&0xFF)<<16;
	result[0] += (c[2]&0xFF)<<8;
	result[0] += (c[3]&0xFF);
	result[1]  = (c[4]&0xFF)<<24;
	result[1] += (c[5]&0xFF)<<16;
	result[1] += (c[6]&0xFF)<<8;
	result[1] += (c[7]&0xFF);
	result[2]  = (c[8]&0xFF)<<24;
	result[2] += (c[9]&0xFF)<<16;
	result[2] += (c[10]&0xFF)<<8;
	result[2] += (c[11]&0xFF);
		
	/* check for negative result */
	if( result[0] == -1 && result[1] == -1 && result[2] < 0 )
		return(-result[2]);
	if( result[0] == 0 && result[1] == 0 && result[2] >=0 )
		return(result[2]);
	return((1<<31)-1);
}

bad_result(expect,actual,diff)
register char *expect;
register char *actual;
int diff;
{
	check_fail("Result");
	if( In_c > 0 ){
		printf("\t\t");
		if( In_c == 2 )	/* 1st operator of DYADIC is always extended */
			show_val(In1,'x');
		else
			show_val(In1,In_s);
		printf(" (Input)\n");
		}
	if( In_c > 1 ){
		printf("\t\t");
		show_val(In2,In_s);
		printf(" (Input2)\n");
		}
	printf("\t\t");
	show_val(expect,Out_s);
	printf(" (Expected)\n");
	printf("\t\t");
	show_val(actual,Out_s);
	printf(" (Actual)\n");
	printf("\t\t%d (Diff)\n",diff);
}

check_fail(s)
char *s;
{
	if( Vec_print==0 ){
		show_all();
		Vec_print++;
		}
	printf("\t*** Check failed: %s\n",s);
	Errcount++;
}

show_all()
{
	printf("%s",Vector);
	if(Equal_seen)
		printf("\n\t= ");
	else
		printf(" = ");
	if( Out_c > 0 )
		show_val(Out1,Out_s);
	if( Out_c > 1 ){
		printf(" ");
		show_val(Out2,Out_s);
		}
	show_fpsr();
	printf("\n");
}

show_fpsr()
{
	int quotient;

	show_bit(N_bit,"N");
	show_bit(Z_bit,"Z");
	show_bit(I_bit,"I");
	show_bit(NAN_bit,"NAN");
	show_bit(BSUN_bit,"BSUN");
	show_bit(SNAN_bit,"SNAN");
	show_bit(OPERR_bit,"OPERR");
	show_bit(OVFL_bit,"OVFL");
	show_bit(UNFL_bit,"UNFL");
	show_bit(DZ_bit,"DZ");
	show_bit(INEX2_bit,"INEX2");
	show_bit(INEX1_bit,"INEX1");
	show_bit(AIOP_bit,"AIOP");
	show_bit(AOVFL_bit,"AOVFL");
	show_bit(AUNFL_bit,"AUNFL");
	show_bit(ADZ_bit,"ADZ");
	show_bit(AINEX_bit,"AINEX");
	quotient = (Fpsr>>16)&0xFF;
	if(quotient>0 || Verbose)printf(" Q:%02x",quotient);
	show_count(Branch_taken,"T");
	show_count(Got_SIGFPE,"SIGFPE");
	show_count(Got_bsun,"TBSUN");
	show_count(Got_inex,"TINEX");
	show_count(Got_dz,"TDZ");
	show_count(Got_unfl,"TUNFL");
	show_count(Got_operr,"TOPERR");
	show_count(Got_ovfl,"TOVFL");
	show_count(Got_nan,"TNAN");
	show_count(Got_unimp,"TUNIMP");
	show_count(Got_unsup,"TUNSUP");
}

show_bit(mask,name)
int mask;
char *name;
{
	if( mask&Fpsr )
		printf(" %s",name);
	else if(Verbose)
		printf(" ~%s",name);
}

show_count(val,name)
int val;
char *name;
{
	if( val > 0 ){
		if( val>10)
			printf(" %s(x%d)",name,val);
		else
			while(val--)
				printf(" %s",name);
		}
	else if(Verbose)
		printf(" ~%s",name);
}

show_val(x,s)
char *x;
char s;
{
	register int i;
#ifndef STANDALONE
	char	xtmp[FSIZE];
	extern double xtod();
#endif

	switch(s){
	case 'b':
		printf("%02x",(*(x+11))&0xFF);
		printf("(%d)",*(x+11));
		break;
	case 'w':
		printf("%04x",(*(short int *)(x+10))&0xFFFF);
		printf("(%d)",*(short int *)(x+10));
		break;
	case 'l':
		printf("%08x",(*(int *)(x+8)));
		printf("(%d)",*(int *)(x+8));
		break;
	case 's':
		for(i=8;i<FSIZE;i++)
			printf("%02x",(*(x+i))&0xFF);
#ifndef STANDALONE
		printf("(%g)",*(float *)(x+8));
#endif
		break;
	case 'd':
		for(i=4;i<FSIZE;i++)
			printf("%02x",(*(x+i))&0xFF);
#ifndef STANDALONE
		printf("(%g)",*(double *)(x+4));
#endif
		break;
	case 'x':
		for(i=0;i<FSIZE;i++)
			printf("%02x",(*(x+i))&0xFF);
#ifndef STANDALONE
		for(i=0;i<FSIZE;i++)
			xtmp[i] = x[i];
		xtod(xtmp);
		printf("(%g)",*(double *)(xtmp+4));
#endif
		break;
	case 'p':
		for(i=0;i<FSIZE;i++)
			printf("%02x",(*(x+i))&0xFF);
		pack_print(x);
		break;
		}
}

/*
 *	pack_print --- print packed decimal
 */
pack_print(p)
char *p;
{
	char	sm = '+';
	char	se = '+';
	int	exponent;
	short int	*etmp;
	int	*ftmp;
	char	fraction[18];
	char	*fp;

	if( *p & 0x80 )
		sm = '-';
	if( *p & 0x40 )
		se = '-';
	etmp = (short int *)p;
	ftmp = (int *)(p+4);
	exponent = (etmp[0]&0xFFF) + (etmp[1]&0xF000);
	if((*p & 0x30) == 0x30 ){	/* Infinity or NaN */
		if( ftmp[0]==0 && ftmp[1]==0 )
			printf("(%cINF)",sm);
		else if( p[4]&0x40 )
			printf("(%cNAN)",sm);
		else
			printf("(%cSNAN)",sm);
		
		}
	else if( ftmp[0]==0 && ftmp[1]==0 && (p[3]&0xF)==0 )
		printf("(%cZERO)",sm);
	else{	/* inrange number */
		sprintf(fraction,"%08x%08x",ftmp[0],ftmp[1]);
		fp = fraction + strlen(fraction)-1;
		while( (fp>fraction) && (*fp)=='0'){
			*fp='\0';
			fp--;
			}
		printf("(%c%x.%se%c%x)",
			sm,p[3]&0xF,fraction,se,exponent);
		}
}

/*
 *	low_string --- convert string to all lower case
 */
low_string(s)
register char *s;
{
	while(*s){
		if( isupper(*s) ) *s = tolower(*s);
		s++;
		}
}

/*
 *	error --- report an error
 */
error(s)
char *s;
{
	printf("Error in %s Line %d: %s\n",Filename,Testno,s);
	Errcount++;
}

/*
 *	logit --- write current test vector to a file
 */
logit()
{
	FILE *fp = fopen("LASTTEST","w");

	if(fp!=NULL){
		fprintf(fp,"%s\n",Vector);
		fclose(fp);
		sync();
		}
}

