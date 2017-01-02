
/*
 *		Copyright (C) Motorola, Inc. 1990
 *			All Rights Reserved
 *
 *	For details on the license for this file, please see the
 *	file, README.lic, in this same directory.
 */

/* t_types */
#define	T_MONADIC	1
#define	T_DYADIC	2
#define	T_SGLDIV	T_DYADIC /* aliased */
#define	T_SGLMUL	T_DYADIC /* aliased */
#define	T_SINCOS	3
#define	T_MOVEO		4	/* all move out's except fmove.p */
#define	T_MOVEOD	5	/* move out to data register (.bwls only) */
#define	T_MOVEPS	6	/* fmove.p out with static k-factor */
#define	T_MOVEPD	7	/* fmove.p out with dynamic k-factor */
#define	T_MOVECR	8
#define	T_TST		9
#define	T_BCC		10
#define	T_END		0	/* last entry in table */

struct template {
	int	t_type;		/* monadic, dyadic, sincos, ... */
	char	*t_name;
	int	(*t_func)();	/* routine that will do it */
	int	t_exact;	/* if set, check results for exact equality */
	};

