#include <stdio.h>
#include <ctype.h>

/*
 *		Copyright (C) Motorola, Inc. 1990
 *			All Rights Reserved
 *
 *	For details on the license for this file, please see the
 *	file, README.lic, in this same directory.
 */
#define	CUSTOM	1	/* when defined, do FPSP specific cleanup */

#define MAXBUF  1024    /* input line length */
#define MAXMNE  20      /* space for longest mnemonic */
#define MAXLAB  30      /* longest label */
#define	MAXOP	10	/* max number of operands */

#define	COMMENTCHAR	'#'
#define	OLDCOMMENTCHAR	'*'	/* alternate comment char */

char    Line[MAXBUF] = {0};     /* input line buffer                    */

char    Label[MAXLAB] = {0};    /* label 				*/
char    Mne[MAXMNE] = {0};	/* opcode mnemonic (including .size)	*/
char	Operand[MAXBUF] = {0};	/* operand(s) 				*/
char	Comment[MAXBUF] = {0};	/* comment				*/

char	Op[MAXOP][MAXBUF];	/* used by split_op for difficult conversions */

struct remap {
	char	*old_name;
	char	*new_name;
	};

struct remap Mnemonic[] = {
#include "mnemonics.h"
"",""
};
#define	Mnesize	(sizeof(Mnemonic)/sizeof(Mnemonic[0]))-1

struct remap Opertab[] = {
#include "operands.h"
"",""
};
#define	Opersize (sizeof(Opertab)/sizeof(Opertab[0]))-1

struct remap Zero[] = {
#include "zero.h"
"",""
};
#define	Zerosize (sizeof(Zero)/sizeof(Zero[0]))-1

char mapdn();
char	*getitem();
char	*map_operand();
char	*table_search();

#define	mne_look(s)	table_search(s,Mnemonic,&Mnemonic[ Mnesize-1])
#define	op_look(s)	table_search(s,Opertab,&Opertab[ Opersize-1])
#define	zero_look(s)	table_search(s,Zero,&Zero[ Zerosize-1])


/*
 *      convert Motorola asm file to UNIX as syntax
 */
main(argc,argv)
int     argc;
char    **argv;
{
	FILE	*f;
	FILE	*fopen();

	if( argc==1 )
		asm2as(stdin);
	else
		while(--argc)
			if( (f = fopen(*++argv,"r")) == NULL ){
				printf("%s: ",*argv);
				fatal("Can't open");
				}
			else{
				asm2as(f);
				fclose(f);
				}
	exit(0);
}

/*
 *      asm2as --- read file and convert
 */
asm2as(f)
FILE    *f;
{
	while( fgets(Line,sizeof(Line),f) != NULL ){
		if(Line[0] == OLDCOMMENTCHAR )
			Line[0] = COMMENTCHAR;
		if(Line[0] == '\n' || Line[0]==COMMENTCHAR)
			printf("%s",Line);
		else{
			parse_line();
			process();
			}
		}
}

/*
 *      parse_line --- split input line into label, op and operand
 */
parse_line()
{
	register char *ptrfrm = Line;
	register char *ptrto = Label;
	register int mnelen;
	char	*skip_white();

	while( !isspace(*ptrfrm) )
		*ptrto++ = *ptrfrm++;
	if(*--ptrto != ':')ptrto++;     /* allow trailing : */
	*ptrto = '\0';

	ptrfrm = skip_white(ptrfrm);

	ptrto = Mne;
	while( !isspace(*ptrfrm) )
		*ptrto++ = mapdn(*ptrfrm++);
	*ptrto = '\0';


	/* pickup operands for any mnemonic that needs them (most will) */
	if( zero_look(Mne) == NULL ){
		ptrfrm = skip_white(ptrfrm);
		ptrto = Operand;
		while( !isspace(*ptrfrm) )
			*ptrto++ = *ptrfrm++;
		*ptrto = '\0';
		}
	else
		strcpy(Operand,"");

#ifdef CUSTOM
	/* check for annoying leading comment strings and remove them. */
	/* all output comments will start with COMMENTCHAR */
	ptrfrm = skip_white(ptrfrm);
	while(*ptrfrm==';')
		ptrfrm++;
	if( strlen(ptrfrm)>3 && strncmp(ptrfrm,"...",3)==0)
		ptrfrm+=3;
#endif

	ptrfrm = skip_white(ptrfrm);
	ptrto = Comment;
	while( *ptrfrm != '\n' )
		*ptrto++ = *ptrfrm++;
	*ptrto = '\0';

}

process()
{
	char	mtmp[MAXMNE];	/* mnemonic convert buffer */
	char	otmp[MAXBUF];	/* operand convert buffer */
	char	*opscan;
	char	*newmne;
	char	*newop;
	char	*p;
	int	mnelen = strlen(Mne);
	int	bfflag;
	char	bfbuf[20];
	char	*sect_map();

	if( *Label && strcmp(Mne,"idnt")!=0 ){
		if( strcmp(Mne,"equ")!=0 && strcmp(Mne,"fequ")!=0)
			printf("%s:\n",Label);
		if(mnelen==0)
			return;
		}

	if(mnelen==0)
		return;
	/*
	 * Convert mnemonic.  First, try looking up the entire Mne string
	 * in the table.  If not found, then try again if the string
	 * currently ends in a '.x' by replacing the . with a '\0'.
	 * If found, the new Mne field is the table value plus the original
	 * '.x' end of the instruction.  If none of this works, pass the
	 * unmodified Mne string through.
	 */
	if( (newmne = mne_look(Mne)) == NULL){
		if( mnelen>2 && Mne[mnelen-2]=='.'){
			Mne[mnelen-2] = '\0';
			if((newmne = mne_look(Mne))!=NULL){
				strcpy(mtmp,newmne);
				strcat(mtmp,".");
				strcat(mtmp,&Mne[mnelen-1]);
				newmne = mtmp;
				}
			else{
				Mne[mnelen-2] = '.';
				newmne = Mne;
				}
			}
		else
			newmne = Mne;
		}
	
	if( strlen(newmne)==0){	/* an untranslatable mnemonic */
		printf("%c\t%s\t%s\t%s\n",COMMENTCHAR,Mne,Operand,Comment);
		return;
		}

	/* special case for fequ/equ.  The Label field needs to move */
	/* to the beginning of the operand field */
	if( strcmp(newmne,"set")==0){
		strcpy(otmp,Label);
		strcat(otmp,",");
		}
	else
		strcpy(otmp,"");

	if(*Operand){
		opscan = Operand;
		newop = &otmp[strlen(otmp)];
		bfflag = 0;
		while( (opscan=getitem(opscan,newop)) != NULL ){
			if( (p = op_look(newop))!=NULL)
				strcpy(newop,p);

			/* check for bit field items.  Insert '&' for static */
			if(bfflag && *newop!='%'){
				strcpy(bfbuf,newop);
				strcpy(newop,"&");
				strcat(newop,bfbuf);
				}
			if( *newop=='{' || *newop==':')
				bfflag=1;
			else
				bfflag=0;
			newop += strlen(newop);
			}
		}

	/* That was the easy part.  Now check for the difficult cases. */
	if( strcmp(newmne,"set")==0){
		}
	else if( strncmp(newmne,"movm",4)==0 )
		rlist_to_mask(otmp);	/* must convert list to mask */
	else if( strncmp(newmne,"fmovm",5)==0 )
		flist_to_mask(otmp);	/* must convert list to mask */
	else if( strncmp(newmne,"cmp",3)==0 || strncmp(newmne,"fcmp",4)==0 )
		swap_oper(otmp);	/* reverse operands on cmp/fcmp */
	else if( strcmp(newmne,"section")==0 ){
		newmne = sect_map(otmp); /* change section directives */
		otmp[0] = '\0';
		Comment[0] = '\0';
		}
	else if( strncmp(newmne,"btst.",5)==0 ||
		 strncmp(newmne,"bclr.",5)==0 ||
		 strncmp(newmne,"bchg.",5)==0 ||
		 strncmp(newmne,"bset.",5)==0 )
			*(newmne+4)='\0';
		
		
	print_line(newmne,otmp,Comment);
}

/*
 *	sect_map --- convert 'section nnn' to text/data/bss
 *
 *	The mapping is arbitrary.  It matches the FPSP usage.
 */
char *
sect_map(s)
char *s;
{
	if( strcmp(s,"15")==0 )
		return("data");
	if( strcmp(s,"8")==0 )
		return("text");
	if( strcmp(s,"9")==0 )
		return("bss");	/* not used by FPSP any more? */
	fatal("sect_map");
}

/*
 *	rlist_to_mask --- convert register list to mask
 *
 *	The UNIX assembler lacks creature features like human readable
 *	register lists since the compiler always keeps track of what
 *	registers are used by a function.
 */
rlist_to_mask(o)
char *o;
{
	char *rltm();

	if( split_op(o) != 2 )
		fatal("rlist_to_mask");
	if( strncmp(Op[1],"%d",2)==0 || 
	    strncmp(Op[1],"%a",2)==0 ||
	    strncmp(Op[1],"%sp",3)==0){
		strcpy(o,rltm(Op[1],Op[2]));
		strcat(o,",");
		strcat(o,Op[2]);
		strcat(Comment," {");
		strcat(Comment,Op[1]);	/* show what was converted */
		strcat(Comment,"}");
		}
	else if( strncmp(Op[2],"%d",2)==0 || 
	    strncmp(Op[2],"%a",2)==0 ||
	    strncmp(Op[2],"%sp",3)==0){
		strcpy(o,Op[1]);
		strcat(o,",");
		strcat(o,rltm(Op[2],""));
		strcat(Comment," {");
		strcat(Comment,Op[2]);	/* show what was converted */
		strcat(Comment,"}");
		}
	else
		fatal("rlist_to_mask2");
}

/*
 *	rltm --- rlist to mask (low level conversion)
 *
 *	If the ea argument begins with '-(' then predecrement addressing
 *	is being used and the mask bits will need to be reversed.
 */
char *rltm(s,ea)
register char *s;
register char *ea;
{
	static	char	mskbuf[20];
	int	rmask = 0;
	int	rreg,lastrreg;
	int	i;
	char	*scanr();

	s = scanr(s,&lastrreg);	/* already checked for at least one? */
	rmask = 1<<lastrreg;

	while(*s){
		if( *s=='/' ){
			s = scanr(s+1,&rreg);
			rmask |= 1<<rreg;
			lastrreg = rreg;
			}
		else if( *s == '-' ){
			s = scanr(s+1,&rreg);
			if( lastrreg <= rreg )
				for(i=lastrreg;i<=rreg;i++)
					rmask |= 1<<i;
			else
				for(i=rreg;i<=lastrreg;i++)
					rmask |= 1<<i;
			lastrreg = rreg;
			}
		else
			fatal("rltm");
		}
	if( strncmp(ea,"-(",2)==0)
		rmask = (reverse(rmask)>>16)&0xFFFF;
	sprintf(mskbuf,"&0x%x",rmask);
	return(mskbuf);
}

char *scanr(s,p)
char *s;
int *p;
{
	int rnum;

	if( strncmp(s,"%d",2)==0){
		rnum = *(s+2);
		if( rnum>='0' && rnum <='7'){
			*p = rnum-'0';
			return(s+3);
			}
		else
			fatal("scanr");
		}
	if( strncmp(s,"%a",2)==0){
		rnum = *(s+2);
		if( rnum>='0' && rnum <='7'){
			*p = (rnum-'0')+8;
			return(s+3);
			}
		else
			fatal("scanr2");
		}
	if( strncmp(s,"%sp",3)==0){
		*p = 15;
		return(s+3);
		}
	fatal("scanr3");
}

/*
 *	flist_to_mask --- convert floating point register list to mask
 *
 *	For control register moves, replace the '/' separating the
 *	register list with commas because that's the way the
 *	assembler expects it. 
 */
flist_to_mask(o)
char *o;
{
	char *fltm();
	register char *p;

	if( split_op(o) != 2 )
		fatal("flist_to_mask");
	if( strncmp(Op[1],"%fp",3)==0){
		strcpy(o,fltm(Op[1],Op[2]));
		strcat(o,",");
		strcat(o,Op[2]);
		strcat(Comment," {");
		strcat(Comment,Op[1]);	/* show what was converted */
		strcat(Comment,"}");
		}
	else if( strncmp(Op[2],"%fp",3)==0){
		strcpy(o,Op[1]);
		strcat(o,",");
		strcat(o,fltm(Op[2],""));
		strcat(Comment," {");
		strcat(Comment,Op[2]);	/* show what was converted */
		strcat(Comment,"}");
		}
	else if( strncmp(Op[1],"%control",8)==0 ||
		 strncmp(Op[1],"%status",7)==0 ||
		 strncmp(Op[1],"%iaddr",6)==0 ){
			for(p=Op[1];*p;p++)
				if(*p=='/')
					*p = ',';
			strcpy(o,Op[1]);
			strcat(o,",");
			strcat(o,Op[2]);
		}
	else if( strncmp(Op[2],"%control",8)==0 ||
		 strncmp(Op[2],"%status",7)==0 ||
		 strncmp(Op[2],"%iaddr",6)==0 ){
			for(p=Op[2];*p;p++)
				if(*p=='/')
					*p = ',';
			strcpy(o,Op[1]);
			strcat(o,",");
			strcat(o,Op[2]);
		}
}

/*
 *	fltm --- flist to mask (low level conversion)
 *
 *	If the ea argument begins with '-(' then predecrement addressing
 *	is being used and the mask bits will need to be reversed.
 */
char *fltm(s,ea)
register char *s;
register char *ea;
{
	static	char	mskbuf[20];
	int	fmask = 0;
	int	freg,lastfreg;
	int	i;
	char	*scanfp();

	s = scanfp(s,&lastfreg);	/* already checked for at least one? */
	fmask = 1<<(7-lastfreg);

	while(*s){
		if( *s=='/' ){
			s = scanfp(s+1,&freg);
			fmask |= 1<<(7-freg);
			lastfreg = freg;
			}
		else if( *s == '-' ){
			s = scanfp(s+1,&freg);
			if( lastfreg<=freg )
				for(i=lastfreg;i<=freg;i++)
					fmask |= 1<<(7-i);
			else
				for(i=freg;i<=lastfreg;i++)
					fmask |= 1<<(7-i);
			lastfreg = freg;
			}
		else
			fatal("fltm");
		}
	if( strncmp(ea,"-(",2)==0)
		fmask = (reverse(fmask)>>24)&0xFF;
	sprintf(mskbuf,"&0x%x",fmask);
	return(mskbuf);
}

char *scanfp(s,p)
char *s;
int *p;
{
	int fnum;

	if( strncmp(s,"%fp",3)==0){
		fnum = *(s+3);
		if( fnum>='0' && fnum <='7'){
			*p = fnum-'0';
			return(s+4);
			}
		else
			fatal("scanfp");
		}
	fatal("scanfp2");
}

/*
 *	swap_oper --- swap operands
 *
 *	Special case for cmp.  The UNIX assembler wants its operands
 *	in the opposite order for some reason.
 */
swap_oper(o)
char *o;
{
	int numops;

	numops = split_op(o);	/* split and count operands */
	if( numops != 2 )
		fatal("swap_oper");
	strcpy(o,Op[2]);
	strcat(o,",");
	strcat(o,Op[1]);
}

split_op(o)
register char *o;
{
	int	nestcount = 0;
	register char *curop;
	int	opcount = 0;
	int	i;

	for(i=0;i<MAXOP;i++)
		Op[i][0] = '\0';

	while( *o && opcount<MAXOP ){
		curop = Op[opcount+1];
		while( *o ){
			if( *o == ',' && nestcount==0){
				o++;
				break;
				}
			if( *o == '(' || *o == '[' )
				nestcount++;
			if( *o == ')' || *o == ']' )
				nestcount--;
			*curop++ = *o++;
			}
		*curop = '\0';
		opcount++;
		}
	return(opcount);
}

print_line(mne,op,com)
char *mne;
char *op;
char *com;
{
	int compad;

	printf("\t%s",mne);
	if(*op){
		if(strlen(mne)<8)
			printf("\t");
		printf("\t%s",op);
		compad = (23-strlen(op))/8;
		}
	else
		compad = (39-strlen(mne))/8;
	if(compad<1)
		compad=1;

	if(*com){
		while(compad--)
			printf("\t");
		printf("%c %s",COMMENTCHAR,com);
		}
	printf("\n");
}

/*
 *	getitem --- scan next item from string
 *
 *	Alphanumeric strings are considered one item.  Except for the
 *	special sequence '#:' everything else is an item of length 1.
 *	We need to return '#:' as a single item so that the ':' can be
 *	identified as the leading part of a floating point constant. 
 *	The other use of ':' is within a bit field specification and
 *	for that we need to leave it alone.
 */
char *
getitem(ip,result)
register char *ip;
register char *result;
{
	if( ip==NULL || *ip == '\0')
		return(NULL);
	if( alphan(*ip) ){
		while( alphan(*ip) )
			*result++ = *ip++;
		}
	else{
		if( (*result++ = *ip++) == '#' && *ip==':')
			*result++ = *ip++;
		}
	*result = '\0';
	return(ip);
}

/*
 *      table_search --- binary tree search a remap structure
 *
 *      Return pointer to remapped string, or NULL if not found.
 */
char *
table_search(str,low,high)
char    *str;
register struct remap *low;
register struct remap *high;
{
	register struct remap *mid;
	int     cond;

	while (low <= high){
		mid = low + (high-low)/2;
		if( ( cond = strcmp(str,mid->old_name)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else
			return(mid->new_name);
	}
	return(NULL);
}

/*
 *      fatal --- fatal error handler
 */
fatal(str)
char    *str;
{
	printf("%s\n",str);
	exit(1);
}

/*
 *      skip_white --- move pointer to next non-whitespace char
 */
char *skip_white(ptr)
char    *ptr;
{
	while(*ptr==' ' || *ptr=='\t')
		ptr++;
	return(ptr);
}

/*
 *      mapdn --- convert A-Z to a-z
 */
char mapdn(c)
char c;
{
	return( isupper(c) ? tolower(c) : c);
}

/*
 *      alphan --- is character a legal letter or digit
 */
alphan(c)
char c;
{
	return( isalpha(c) || isdigit(c) || c== '_');
}

/*
 *      reverse --- reverse the bits in an int
 *
 *      Algorithm from Dr. Dobbs Journal #46, June/July 1980, p.48
 *      Original by C. Strachey [CACM 4:3 961 p.146]
 */
reverse(val)
int val;
{
	static int mask[] = { 0x55555555, 0x33333333,
			      0x0F0F0F0F, 0x00FF00FF, 0x0000FFFF };
	register int i = val;
	register int j = 16;
	register int k = 4;

	while(j){
		i = ((i&mask[k])<<j)|((i>>j)&mask[k]);
		j >>= 1;
		k--;
		}
	return(i);
}

