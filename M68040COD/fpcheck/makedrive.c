#include <stdio.h>

/*
 *		Copyright (C) Motorola, Inc. 1990
 *			All Rights Reserved
 *
 *	For details on the license for this file, please see the
 *	file, README.lic, in this same directory.
 */

#define	BUFSIZE	1000

FILE *extdefs;
FILE *body;
FILE *drivers;
char *Platform = "";

/*
 *	makedrive --- generate templates from description file
 *
 *	A single argument is the platform name.  Stdin is a list
 *	of test templates to generate.  The list contains 4 items per
 *	line:  Type, Name, Op and Size.  The Type is one of the
 *	types found in template.h.  Name is the base portion of the
 *	template name.  Op is the mnemonic of the instruction being
 *	generated.  Size is either a string of letters representing
 *	the sizes of the instructions, or it is a range of numbers
 *	(in hex) for some special instructions.
 *
 *	For each Type, a file of the form Platform.Type is expected.
 *	The file contains assembly language code to generate the type
 *	of test specified.  In each template file, the string 'EN' will
 *	be replaced with the Name field, 'OP' with the Op field and
 *	'SZ' with the Size field.  A variant of the SZ field is 'SS'.  It
 *	is the Size field converted to a constant suitable for constructing
 *	a floating point opcode by addition.  This kludge would not
 *	be necessary if the assemblers could truly assemble all floating
 *	point instructions.
 */
main(argc,argv)
int argc;
char **argv;
{
	char	buf[BUFSIZE];
	char	type[BUFSIZE];
	char	name[BUFSIZE];
	char	op[BUFSIZE];
	char	size[BUFSIZE];
	char	exact[BUFSIZE];
	char	tmp[BUFSIZE];
	int	from,to;
	register char *p;

	if( argc != 2)
		exit(1);

	Platform = argv[1];

	extdefs = fopen("extdefs.h","w");
	body = fopen("body.c","w");
	drivers = fopen("drivers.s","w");

	if( extdefs==NULL || body==NULL || drivers==NULL)
		exit(1);

	while( gets(buf) != NULL ){
		sscanf(buf,"%s %s %s %s %s",type,name,op,size,exact);
		printf("Creating %s\n",name);
		if( size[0] == '#' ){	/* size is range of numbers */
			sscanf(&size[1],"%x-%x",&from,&to);
			while(from<=to){
				sprintf(tmp,"%x",from++);
				makeone(name,op,type,tmp,exact);
				}
			}
		else{	/* size is string containing 'bwlsdxp' */
			p = size;
			while(*p){
				tmp[0] = *p++;
				tmp[1] = '\0';
				makeone(name,op,type,tmp,exact);
				}
			}
		}
	fclose(extdefs);
	fclose(body);
	fclose(drivers);
	exit(0);
}

makeone(name,op,type,size,exact)
char *name;
char *op;
char *type;
char *size;
char *exact;
{
	char sbuf[BUFSIZE];
	char *ss;
	int eflag;

#define SEDSTR "sed -e 's/EN/%s/g' -e 's/OP/%s/g' -e 's/SZ/%s/g' -e 's/SS/%s/g'  <%s.%s >>%s"

	     if( strcmp(size,"l")==0)ss = "0";
	else if( strcmp(size,"s")==0)ss = "1024";
	else if( strcmp(size,"x")==0)ss = "2048";
	else if( strcmp(size,"p")==0)ss = "3072";
	else if( strcmp(size,"w")==0)ss = "4096";
	else if( strcmp(size,"d")==0)ss = "5120";
	else if( strcmp(size,"b")==0)ss = "6144";
	else ss = "0";

	eflag = (*exact=='y' && *size!='p') ? 1 : 0;

	fprintf(extdefs,"extern %s_%s();\n",name,size);
	fprintf(body,"{T_%s, \"%s.%s\", %s_%s, %d},\n",
			type,name,size,name,size,eflag);
	sprintf(sbuf,SEDSTR,name,op,size,ss,Platform,type,"drivers.s");
	system(sbuf);
}
