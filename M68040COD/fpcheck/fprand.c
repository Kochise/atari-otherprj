#include <stdio.h>

/*
 *	random --- generate random fpcheck test cases
 */

#define	MONADIC	0
#define	DYADIC	1
#define	MOVECR	2
#define	MOVEO	3	/* all move outs except packed */
#define	MOVEOD	4	/* move out to data register */
#define	MOVEPS	5	/* static k-factor packed move out */
#define	MOVEPD	6	/* dyanmic k-factor packed moved out */
#define	BCC	7	/* conditional branches */

#define	ALL_SIZES	"bwlssddxxp"	/* all sizes, slight bias for s,d,x */

struct testcase {
	int	t_type;		/* one of the above test types */
	char	*t_name;	/* test name */
	char	*t_size;	/* allowed sizes */
	};

struct testcase Tests[] = {
	{MONADIC,"fabs",ALL_SIZES},
	{MONADIC,"fsabs",ALL_SIZES},
	{MONADIC,"fdabs",ALL_SIZES},
	{MONADIC,"facos",ALL_SIZES},
	{MONADIC,"fasin",ALL_SIZES},
	{MONADIC,"fatan",ALL_SIZES},
	{MONADIC,"fatanh",ALL_SIZES},
	{MONADIC,"fcos",ALL_SIZES},
	{MONADIC,"fcosh",ALL_SIZES},
	{MONADIC,"fetox",ALL_SIZES},
	{MONADIC,"fetoxm1",ALL_SIZES},
	{MONADIC,"fgetexp",ALL_SIZES},
	{MONADIC,"fgetman",ALL_SIZES},
	{MONADIC,"fint",ALL_SIZES},
	{MONADIC,"fintrz",ALL_SIZES},
	{MONADIC,"flog10",ALL_SIZES},
	{MONADIC,"flog2",ALL_SIZES},
	{MONADIC,"flogn",ALL_SIZES},
	{MONADIC,"flognp1",ALL_SIZES},
	{MONADIC,"fmove",ALL_SIZES},
	{MONADIC,"fsmove",ALL_SIZES},
	{MONADIC,"fdmove",ALL_SIZES},
	{MONADIC,"fneg",ALL_SIZES},
	{MONADIC,"fsneg",ALL_SIZES},
	{MONADIC,"fdneg",ALL_SIZES},
	{MONADIC,"fsin",ALL_SIZES},
	{MONADIC,"fsinh",ALL_SIZES},
	{MONADIC,"fsqrt",ALL_SIZES},
	{MONADIC,"fssqrt",ALL_SIZES},
	{MONADIC,"fdsqrt",ALL_SIZES},
	{MONADIC,"ftan",ALL_SIZES},
	{MONADIC,"ftanh",ALL_SIZES},
	{MONADIC,"ftentox",ALL_SIZES},
	{MONADIC,"ftwotox",ALL_SIZES},
	{MONADIC,"fsincos",ALL_SIZES},
	{MONADIC,"ftst",ALL_SIZES},
	{DYADIC,"fadd",ALL_SIZES},
	{DYADIC,"fsadd",ALL_SIZES},
	{DYADIC,"fdadd",ALL_SIZES},
#if 0
	{DYADIC,"fcmp",ALL_SIZES},
#endif
	{DYADIC,"fdiv",ALL_SIZES},
	{DYADIC,"fsdiv",ALL_SIZES},
	{DYADIC,"fddiv",ALL_SIZES},
	{DYADIC,"fmod",ALL_SIZES},
	{DYADIC,"fmul",ALL_SIZES},
	{DYADIC,"fsmul",ALL_SIZES},
	{DYADIC,"fdmul",ALL_SIZES},
	{DYADIC,"frem",ALL_SIZES},
	{DYADIC,"fscale",ALL_SIZES},
	{DYADIC,"fsub",ALL_SIZES},
	{DYADIC,"fssub",ALL_SIZES},
	{DYADIC,"fdsub",ALL_SIZES},
	{DYADIC,"fsglmul",ALL_SIZES},
	{DYADIC,"fsgldiv",ALL_SIZES},
	{MOVECR,"fmovecr","x"},
	{MOVEO,"fmove","bwlssddxx"},
	{MOVEOD,"fmove","bwlss"},
	{MOVEPS,"fmove","p"},
	{MOVEPD,"fmove","p"}
#if 0
	{BCC,"fbge","wl"},
	{BCC,"fboge","wl"},
	{BCC,"fbgl","wl"},
	{BCC,"fbogl","wl"},
	{BCC,"fbgle","wl"},
	{BCC,"fbor","wl"},
	{BCC,"fbgt","wl"},
	{BCC,"fbogt","wl"},
	{BCC,"fble","wl"},
	{BCC,"fbole","wl"},
	{BCC,"fblt","wl"},
	{BCC,"fbolt","wl"},
	{BCC,"fbnge","wl"},
	{BCC,"fbuge","wl"},
	{BCC,"fbngl","wl"},
	{BCC,"fbueq","wl"},
	{BCC,"fbngle","wl"},
	{BCC,"fbun","wl"},
	{BCC,"fbngt","wl"},
	{BCC,"fbugt","wl"},
	{BCC,"fbnle","wl"},
	{BCC,"fbule","wl"},
	{BCC,"fbnlt","wl"},
	{BCC,"fbult","wl"},
	{BCC,"fbseq","wl"},
	{BCC,"fbeq","wl"},
	{BCC,"fbsne","wl"},
	{BCC,"fbne","wl"},
	{BCC,"fbsf","wl"},
	{BCC,"fbf","wl"},
	{BCC,"fbst","wl"},
	{BCC,"fbt","wl"}
#endif
	};
#define	NUMTESTS	(sizeof(Tests)/sizeof(Tests[0]))

char Okconsts[] = { 	/* legal values for fmovecr.x */
	0x00,0x0B,0x0C,0x0D,0x0E,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
	0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F
	};

extern double drand48();
unsigned rndlong();

main(argc,argv)
int argc;
char **argv;
{
	int	gencount = 100000000;
	int	i;
	
	srand48(time(0));	/* random number initialization */

	if(argc>1)
		gencount = atoi(argv[1]);
	
#ifdef LEGAL_ONLY
	printf("# %d Random (legal) tests\n",gencount);
#else
	printf("# %d Random tests\n",gencount);
#endif
	printf("verbose\n");
	for(i=0;i<gencount;i++){
		if( (i%100)==99 )
			printf("# Test #%d\n",i+1);
		gentest();
		}
	exit(0);
}

gentest()
{
	register struct testcase *t = &Tests[rnd(NUMTESTS)];
	char size;

	size = t->t_size[rnd(strlen(t->t_size))];
	printf("%s.%c",t->t_name,size);

	switch(t->t_type){
	case MONADIC:
		genop(size);
		break;
	case DYADIC:
		genop('x');
		genop(size);
		break;
	case MOVECR:
#ifdef LEGAL_ONLY
		printf(" %02x",Okconsts[rnd(sizeof(Okconsts))]);
#else
		printf(" %02x",rnd(64));
#endif
		break;
	case MOVEO:
		printf(" out");
		genop('x');
		break;
	case MOVEOD:
		printf(" out d%d",rnd(8));
		genop('x');
		break;
	case MOVEPS:
		printf(" out k#:%d",rnd(128)-64);
		genop('x');
		break;
	case MOVEPD:
		printf(" out kd:%d",rnd(128)-64);
		genop('x');
		break;
	case BCC:
		genfpsr();
		break;
	default:
		fatal("Bad test type");
		}
	genfpcr();
	printf("\n");
}

genop(size)
char size;
{
	switch(size){
	case 'b':
		printf(" %02x",rnd(0x100));
		break;
	case 'w':
		printf(" %04x",rnd(0x10000));
		break;
	case 'l':
		printf(" %08x",rndlong());
		break;
	case 's':
		printf(" %08x",rndlong());
		break;
	case 'd':
		printf(" %08x_%08x",rndlong(),rndlong());
		break;
	case 'x':
#ifdef LEGAL_ONLY
		printf(" %08x_%08x_%08x",
			rndlong()&0xFFFF0000,rndlong(),rndlong());
#else
		printf(" %08x_%08x_%08x",rndlong(),rndlong(),rndlong());
#endif
		break;
	case 'p':
#ifdef LEGAL_ONLY
		switch(rnd(10)){
		case 0:	/* infinity */
			printf(" %cFFF0000_00000000_00000000",rnd(2)?'7':'F');
			break;
		case 1:	/* NAN */
			printf(" %cFFF0000_%08x_%08x",
				rnd(2)?'7':'F',rndlong(),rndlong());
			break;
		case 2:	/* ZERO */
			printf(" %x%03d0000_00000000_00000000",
				rnd(16)&0xC,rnd(1000));
			break;
		default: /* in-range numbers */
			printf(" %x%03d%04d_%08d_%08d",
				rnd(16)&0xC,rnd(1000),rnd(10),
				rndlong()%100000000,
				rndlong()%100000000);
		}
#else
		printf(" %08x_%08x_%08x",rndlong(),rndlong(),rndlong());
#endif
		break;
	default:
		fatal("Bad size in genop");
	}
}

genfpsr()
{
	if( rnd(2)) printf(" n");
	if( rnd(2)) printf(" z");
	if( rnd(2)) printf(" i");
	if( rnd(2)) printf(" nan");
}

genfpcr()
{
	static char *round[4] = {"rn","rz","rm","rp"};
	static char *prec[3] = {"x","d","s"};

	printf(" %s %s",round[rnd(4)],prec[rnd(3)]);
	if(rnd(2)){	/* 50-50 whether we enable any exceptions */
		if(rnd(2)) printf(" bsun");
		if(rnd(2)) printf(" snan");
		if(rnd(2)) printf(" operr");
		if(rnd(2)) printf(" ovfl");
		if(rnd(2)) printf(" unfl");
		if(rnd(2)) printf(" dz");
		if(rnd(2)) printf(" inex2");
		if(rnd(2)) printf(" inex1");
		}
}

/*
 *	rnd --- return int from 0 to range-1
 */
rnd(range)
int range;
{
	return ((int)((range)*drand48()));
}

/*
 *	rndlong --- return random 32-bit integer
 */
unsigned
rndlong()
{
	return((unsigned) (rnd(0x10000)<<16)|rnd(0x10000));
}

fatal(s)
{
	printf("%s\n");
	exit(1);
}
