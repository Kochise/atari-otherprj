#include <stdio.h>
#include <varargs.h>

/*
 *		Copyright (C) Motorola, Inc. 1990
 *			All Rights Reserved
 *
 *	For details on the license for this file, please see the
 *	file, README.lic, in this same directory.
 */

#define	DUART	0xf1000003
#define	SRX	0x04
#define	TBX	0x0C
#define	RBX	0x0C

#define	RDRF	0x01
#define	TDRE	0x04

extern int Local;

int environ;	/* to make SVR4 happy */

_exit()
{
#ifdef SUN
        asm("	jmp		0xf2000008");	/* FBUG start address */
#endif
#ifdef SYSV
        asm("	jmp.l	0xf2000008");	/* FBUG start address */
#endif
}

int
_inchar()
{
	char	*duart = (char *)DUART;
	int	c;

	while( (*(duart+SRX) & RDRF)== 0 )
		;
	c = (*(duart+RBX))&0x7F;
	if( c == '\r' )
		c = '\n';
	if( Local )
		_outchar(c);
	return(c);
}

_outchar(c)
char c;
{
	char	*duart = (char *)DUART;

	if( Local && c == '\n' )
		_outchar('\r');
	while( (*(duart+SRX) & TDRE)== 0 )
		;
	*(duart+TBX) = c;
}

/*
 *	printf --- standalone version of printf
 */
printf(va_alist)
va_dcl
{
	va_list	args;
	char *fmt;
	char	outbuf[1024];

	va_start(args);
	fmt = va_arg(args,char *);
	vsprintf(outbuf,fmt,args);
	va_end();

	fmt = outbuf;
	while(*fmt)
		_outchar(*fmt++);
}

/*
 *	fgets --- standalone version of fgets
 */
char *
fgets(ptr,max,fp)
char *ptr;
int max;
int *fp;	/* Actually FILE *, but we don't use it here */
{
	char	*vp;
	int	left;
	char	c;

	vp = ptr;
	left = max-2;
	while( left ){
		c = _inchar();
		if( c=='\0')
			continue;
		if( c == '\n' ){
			*vp++ = '\n';
			break;
			}
		if( c == '\b' && vp>ptr ){
			vp--;
			left++;
			continue;
			}
		*vp++ = c;
		left--;
		}
	*vp = '\0';	/* always null terminate */

	return(ptr);
}
