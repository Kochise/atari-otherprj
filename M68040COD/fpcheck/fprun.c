#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>

/*
 *		Copyright (C) Motorola, Inc. 1990
 *			All Rights Reserved
 *
 *	For details on the license for this file, please see the
 *	file, README.lic, in this same directory.
 */

/*
 *	fprun --- run floating point test cases on standalone board
 *
 *	Assumes a serial port connected to some hardware running
 *	the standalone version of fpcheck.   The standalone version
 *	always prints a line beginning with '-' at the end of each
 *	test.
 */

#define	MAXBUF	1024
#define	RESPMAX	50000
#define	PATIENCE 10	/* how long to wait before giving up */


/* The following #defines are system dependent.  Change them for your system */
#ifdef SUN
#define	PORTNAME "/dev/ttyb"
#define	PORTSTTY "9600 -echo raw >"
#endif

#ifdef SYSV
#define	PORTNAME "/dev/tty38"
#define	PORTSTTY "9600 -echo raw <"
#endif

char	*Filename;
FILE	*Fp;
char	Vector[MAXBUF];
int	Testno;
int	Sport = -1;

char	*Rnext;
char	Response[RESPMAX];	/* response buffer */

catch_alarm()
{
	strcpy(Rnext,"<Timeout>\n");
	printf("%s",Response);
	exit(1);
}

main(argc,argv)
int argc;
char **argv;
{
	char buf[100];

	/* open and stty the standalone serial port */
	Sport = open(PORTNAME,O_RDWR);
	if(Sport<0){
		perror("Failed serial port open");
		exit(1);
		}
	sprintf(buf,"stty %s%s",PORTSTTY,PORTNAME);
	system(buf);

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
			printf("Can't open %s\n",Filename);
		}
}

/*
 *	dofile --- run tests from a file
 */
dofile()
{
	int	rleft;
	int	rcount;

	while( fgets(Vector,MAXBUF,Fp) != NULL ){
		Testno++;
		write(Sport,Vector,strlen(Vector));
		alarm(0);
		signal(SIGALRM,catch_alarm);
		Rnext = Response;
		rleft = RESPMAX-30;	/* leave room for error strings */
		alarm(PATIENCE);	/* timeout guard */
		while(rleft>0){
			rcount = nocr_read(Sport,Rnext,rleft);
			if(rcount<0){
				strcpy(Rnext,"<Port read error>\n");
				break;
				}
			rleft -= rcount;
			Rnext += rcount;
			*Rnext = '\0';
			
			if( rleft<(RESPMAX-2) && strcmp(Rnext-2,"-\n")==0 ){
				*(Rnext-2) = '\0';
				break;
				}
			}
		alarm(0);		/* disarm timeout */
		printf("%s",Response);
		}
}

/*
 *	nocr_read --- read, but compress out '\r' in buffer
 */
nocr_read(fd,buf,count)
int fd;
char *buf;
int count;
{
	int actual;
	int crcount = 0;
	register char *from;
	register char *to;

	actual = read(fd,buf,count);
	if(actual<=0)
		return(actual);

	for(from=buf;from<(buf+actual);from++)
		if(*from=='\r'){
			for(to=from;to<(buf+(actual-1));to++)
				*to = *(to+1);
			crcount++;
			}
	return(actual-crcount);
}
