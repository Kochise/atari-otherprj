	text
#	include	l_fpsp.h	

#	xref	tag	
#	xref	szero	
#	xref	sinf	
#	xref	sopr_inf	
#	xref	sone	
#	xref	spi_2	
#	xref	szr_inf	
#	xref	src_nan	
#	xref	t_operr	
#	xref	t_dz2	
#	xref	snzrinx	
#	xref	ld_pone	
#	xref	ld_pinf	
#	xref	ld_ppi2	
#	xref	ssincosz	
#	xref	ssincosi	
#	xref	ssincosnan	
#	xref	setoxm1i	
#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the facoss and facosx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	sacos	
#	xref	ld_ppi2	
#	xref	t_operr	
#	xref	mon_nan	
#	xref	sacosd	

	global		facoss
facoss:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1012
	bsr		sacos		# normalized (regular) number
	bra.b		L_1016
L_1012:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1013
	bsr		ld_ppi2
	bra.b		L_1016
L_1013:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1014
	bsr		t_operr
	bra.b		L_1016
L_1014:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1015
	bsr		mon_nan
	bra.b		L_1016
L_1015:
	bsr		sacosd		# assuming a denorm...

L_1016:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		facosd
facosd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1017
	bsr		sacos		# normalized (regular) number
	bra.b		L_101B
L_1017:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1018
	bsr		ld_ppi2
	bra.b		L_101B
L_1018:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1019
	bsr		t_operr
	bra.b		L_101B
L_1019:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_101A
	bsr		mon_nan
	bra.b		L_101B
L_101A:
	bsr		sacosd		# assuming a denorm...

L_101B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		facosx
facosx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_101C
	bsr		sacos		# normalized (regular) number
	bra.b		L_101G
L_101C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_101D
	bsr		ld_ppi2
	bra.b		L_101G
L_101D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_101E
	bsr		t_operr
	bra.b		L_101G
L_101E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_101F
	bsr		mon_nan
	bra.b		L_101G
L_101F:
	bsr		sacosd		# assuming a denorm...

L_101G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fasins and fasinx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	sasin	
#	xref	szero	
#	xref	t_operr	
#	xref	mon_nan	
#	xref	sasind	

	global		fasins
fasins:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1022
	bsr		sasin		# normalized (regular) number
	bra.b		L_1026
L_1022:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1023
	bsr		szero
	bra.b		L_1026
L_1023:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1024
	bsr		t_operr
	bra.b		L_1026
L_1024:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1025
	bsr		mon_nan
	bra.b		L_1026
L_1025:
	bsr		sasind		# assuming a denorm...

L_1026:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fasind
fasind:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1027
	bsr		sasin		# normalized (regular) number
	bra.b		L_102B
L_1027:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1028
	bsr		szero
	bra.b		L_102B
L_1028:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1029
	bsr		t_operr
	bra.b		L_102B
L_1029:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_102A
	bsr		mon_nan
	bra.b		L_102B
L_102A:
	bsr		sasind		# assuming a denorm...

L_102B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fasinx
fasinx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_102C
	bsr		sasin		# normalized (regular) number
	bra.b		L_102G
L_102C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_102D
	bsr		szero
	bra.b		L_102G
L_102D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_102E
	bsr		t_operr
	bra.b		L_102G
L_102E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_102F
	bsr		mon_nan
	bra.b		L_102G
L_102F:
	bsr		sasind		# assuming a denorm...

L_102G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fatans and fatanx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	satan	
#	xref	szero	
#	xref	spi_2	
#	xref	mon_nan	
#	xref	satand	

	global		fatans
fatans:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1032
	bsr		satan		# normalized (regular) number
	bra.b		L_1036
L_1032:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1033
	bsr		szero
	bra.b		L_1036
L_1033:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1034
	bsr		spi_2
	bra.b		L_1036
L_1034:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1035
	bsr		mon_nan
	bra.b		L_1036
L_1035:
	bsr		satand		# assuming a denorm...

L_1036:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fatand
fatand:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1037
	bsr		satan		# normalized (regular) number
	bra.b		L_103B
L_1037:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1038
	bsr		szero
	bra.b		L_103B
L_1038:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1039
	bsr		spi_2
	bra.b		L_103B
L_1039:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_103A
	bsr		mon_nan
	bra.b		L_103B
L_103A:
	bsr		satand		# assuming a denorm...

L_103B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fatanx
fatanx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_103C
	bsr		satan		# normalized (regular) number
	bra.b		L_103G
L_103C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_103D
	bsr		szero
	bra.b		L_103G
L_103D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_103E
	bsr		spi_2
	bra.b		L_103G
L_103E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_103F
	bsr		mon_nan
	bra.b		L_103G
L_103F:
	bsr		satand		# assuming a denorm...

L_103G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fatanhs and fatanhx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	satanh	
#	xref	szero	
#	xref	t_operr	
#	xref	mon_nan	
#	xref	satanhd	

	global		fatanhs
fatanhs:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1042
	bsr		satanh		# normalized (regular) number
	bra.b		L_1046
L_1042:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1043
	bsr		szero
	bra.b		L_1046
L_1043:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1044
	bsr		t_operr
	bra.b		L_1046
L_1044:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1045
	bsr		mon_nan
	bra.b		L_1046
L_1045:
	bsr		satanhd		# assuming a denorm...

L_1046:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fatanhd
fatanhd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1047
	bsr		satanh		# normalized (regular) number
	bra.b		L_104B
L_1047:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1048
	bsr		szero
	bra.b		L_104B
L_1048:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1049
	bsr		t_operr
	bra.b		L_104B
L_1049:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_104A
	bsr		mon_nan
	bra.b		L_104B
L_104A:
	bsr		satanhd		# assuming a denorm...

L_104B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fatanhx
fatanhx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_104C
	bsr		satanh		# normalized (regular) number
	bra.b		L_104G
L_104C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_104D
	bsr		szero
	bra.b		L_104G
L_104D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_104E
	bsr		t_operr
	bra.b		L_104G
L_104E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_104F
	bsr		mon_nan
	bra.b		L_104G
L_104F:
	bsr		satanhd		# assuming a denorm...

L_104G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fcoss and fcosx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	scos	
#	xref	ld_pone	
#	xref	t_operr	
#	xref	mon_nan	
#	xref	scosd	

	global		fcoss
fcoss:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1052
	bsr		scos		# normalized (regular) number
	bra.b		L_1056
L_1052:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1053
	bsr		ld_pone
	bra.b		L_1056
L_1053:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1054
	bsr		t_operr
	bra.b		L_1056
L_1054:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1055
	bsr		mon_nan
	bra.b		L_1056
L_1055:
	bsr		scosd		# assuming a denorm...

L_1056:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fcosd
fcosd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1057
	bsr		scos		# normalized (regular) number
	bra.b		L_105B
L_1057:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1058
	bsr		ld_pone
	bra.b		L_105B
L_1058:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1059
	bsr		t_operr
	bra.b		L_105B
L_1059:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_105A
	bsr		mon_nan
	bra.b		L_105B
L_105A:
	bsr		scosd		# assuming a denorm...

L_105B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fcosx
fcosx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_105C
	bsr		scos		# normalized (regular) number
	bra.b		L_105G
L_105C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_105D
	bsr		ld_pone
	bra.b		L_105G
L_105D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_105E
	bsr		t_operr
	bra.b		L_105G
L_105E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_105F
	bsr		mon_nan
	bra.b		L_105G
L_105F:
	bsr		scosd		# assuming a denorm...

L_105G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fcoshs and fcoshx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	scosh	
#	xref	ld_pone	
#	xref	ld_pinf	
#	xref	mon_nan	
#	xref	scoshd	

	global		fcoshs
fcoshs:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1062
	bsr		scosh		# normalized (regular) number
	bra.b		L_1066
L_1062:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1063
	bsr		ld_pone
	bra.b		L_1066
L_1063:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1064
	bsr		ld_pinf
	bra.b		L_1066
L_1064:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1065
	bsr		mon_nan
	bra.b		L_1066
L_1065:
	bsr		scoshd		# assuming a denorm...

L_1066:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fcoshd
fcoshd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1067
	bsr		scosh		# normalized (regular) number
	bra.b		L_106B
L_1067:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1068
	bsr		ld_pone
	bra.b		L_106B
L_1068:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1069
	bsr		ld_pinf
	bra.b		L_106B
L_1069:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_106A
	bsr		mon_nan
	bra.b		L_106B
L_106A:
	bsr		scoshd		# assuming a denorm...

L_106B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fcoshx
fcoshx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_106C
	bsr		scosh		# normalized (regular) number
	bra.b		L_106G
L_106C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_106D
	bsr		ld_pone
	bra.b		L_106G
L_106D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_106E
	bsr		ld_pinf
	bra.b		L_106G
L_106E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_106F
	bsr		mon_nan
	bra.b		L_106G
L_106F:
	bsr		scoshd		# assuming a denorm...

L_106G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fetoxs and fetoxx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	setox	
#	xref	ld_pone	
#	xref	szr_inf	
#	xref	mon_nan	
#	xref	setoxd	

	global		fetoxs
fetoxs:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1072
	bsr		setox		# normalized (regular) number
	bra.b		L_1076
L_1072:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1073
	bsr		ld_pone
	bra.b		L_1076
L_1073:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1074
	bsr		szr_inf
	bra.b		L_1076
L_1074:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1075
	bsr		mon_nan
	bra.b		L_1076
L_1075:
	bsr		setoxd		# assuming a denorm...

L_1076:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fetoxd
fetoxd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1077
	bsr		setox		# normalized (regular) number
	bra.b		L_107B
L_1077:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1078
	bsr		ld_pone
	bra.b		L_107B
L_1078:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1079
	bsr		szr_inf
	bra.b		L_107B
L_1079:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_107A
	bsr		mon_nan
	bra.b		L_107B
L_107A:
	bsr		setoxd		# assuming a denorm...

L_107B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fetoxx
fetoxx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_107C
	bsr		setox		# normalized (regular) number
	bra.b		L_107G
L_107C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_107D
	bsr		ld_pone
	bra.b		L_107G
L_107D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_107E
	bsr		szr_inf
	bra.b		L_107G
L_107E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_107F
	bsr		mon_nan
	bra.b		L_107G
L_107F:
	bsr		setoxd		# assuming a denorm...

L_107G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fetoxm1s and fetoxm1x entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	setoxm1	
#	xref	szero	
#	xref	setoxm1i	
#	xref	mon_nan	
#	xref	setoxm1d	

	global		fetoxm1s
fetoxm1s:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1082
	bsr		setoxm1		# normalized (regular) number
	bra.b		L_1086
L_1082:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1083
	bsr		szero
	bra.b		L_1086
L_1083:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1084
	bsr		setoxm1i
	bra.b		L_1086
L_1084:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1085
	bsr		mon_nan
	bra.b		L_1086
L_1085:
	bsr		setoxm1d	# assuming a denorm...

L_1086:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fetoxm1d
fetoxm1d:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1087
	bsr		setoxm1		# normalized (regular) number
	bra.b		L_108B
L_1087:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1088
	bsr		szero
	bra.b		L_108B
L_1088:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1089
	bsr		setoxm1i
	bra.b		L_108B
L_1089:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_108A
	bsr		mon_nan
	bra.b		L_108B
L_108A:
	bsr		setoxm1d	# assuming a denorm...

L_108B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fetoxm1x
fetoxm1x:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_108C
	bsr		setoxm1		# normalized (regular) number
	bra.b		L_108G
L_108C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_108D
	bsr		szero
	bra.b		L_108G
L_108D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_108E
	bsr		setoxm1i
	bra.b		L_108G
L_108E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_108F
	bsr		mon_nan
	bra.b		L_108G
L_108F:
	bsr		setoxm1d	# assuming a denorm...

L_108G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fgetexps and fgetexpx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	sgetexp	
#	xref	szero	
#	xref	t_operr	
#	xref	mon_nan	
#	xref	sgetexpd	

	global		fgetexps
fgetexps:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1092
	bsr		sgetexp		# normalized (regular) number
	bra.b		L_1096
L_1092:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1093
	bsr		szero
	bra.b		L_1096
L_1093:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1094
	bsr		t_operr
	bra.b		L_1096
L_1094:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1095
	bsr		mon_nan
	bra.b		L_1096
L_1095:
	bsr		sgetexpd	# assuming a denorm...

L_1096:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fgetexpd
fgetexpd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1097
	bsr		sgetexp		# normalized (regular) number
	bra.b		L_109B
L_1097:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1098
	bsr		szero
	bra.b		L_109B
L_1098:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1099
	bsr		t_operr
	bra.b		L_109B
L_1099:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_109A
	bsr		mon_nan
	bra.b		L_109B
L_109A:
	bsr		sgetexpd	# assuming a denorm...

L_109B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fgetexpx
fgetexpx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_109C
	bsr		sgetexp		# normalized (regular) number
	bra.b		L_109G
L_109C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_109D
	bsr		szero
	bra.b		L_109G
L_109D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_109E
	bsr		t_operr
	bra.b		L_109G
L_109E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_109F
	bsr		mon_nan
	bra.b		L_109G
L_109F:
	bsr		sgetexpd	# assuming a denorm...

L_109G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fsins and fsinx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	ssin	
#	xref	szero	
#	xref	t_operr	
#	xref	mon_nan	
#	xref	ssind	

	global		fsins
fsins:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1102
	bsr		ssin		# normalized (regular) number
	bra.b		L_1106
L_1102:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1103
	bsr		szero
	bra.b		L_1106
L_1103:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1104
	bsr		t_operr
	bra.b		L_1106
L_1104:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1105
	bsr		mon_nan
	bra.b		L_1106
L_1105:
	bsr		ssind		# assuming a denorm...

L_1106:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fsind
fsind:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1107
	bsr		ssin		# normalized (regular) number
	bra.b		L_110B
L_1107:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1108
	bsr		szero
	bra.b		L_110B
L_1108:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1109
	bsr		t_operr
	bra.b		L_110B
L_1109:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_110A
	bsr		mon_nan
	bra.b		L_110B
L_110A:
	bsr		ssind		# assuming a denorm...

L_110B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fsinx
fsinx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_110C
	bsr		ssin		# normalized (regular) number
	bra.b		L_110G
L_110C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_110D
	bsr		szero
	bra.b		L_110G
L_110D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_110E
	bsr		t_operr
	bra.b		L_110G
L_110E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_110F
	bsr		mon_nan
	bra.b		L_110G
L_110F:
	bsr		ssind		# assuming a denorm...

L_110G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fsinhs and fsinhx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	ssinh	
#	xref	szero	
#	xref	sinf	
#	xref	mon_nan	
#	xref	ssinhd	

	global		fsinhs
fsinhs:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1112
	bsr		ssinh		# normalized (regular) number
	bra.b		L_1116
L_1112:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1113
	bsr		szero
	bra.b		L_1116
L_1113:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1114
	bsr		sinf
	bra.b		L_1116
L_1114:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1115
	bsr		mon_nan
	bra.b		L_1116
L_1115:
	bsr		ssinhd		# assuming a denorm...

L_1116:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fsinhd
fsinhd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1117
	bsr		ssinh		# normalized (regular) number
	bra.b		L_111B
L_1117:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1118
	bsr		szero
	bra.b		L_111B
L_1118:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1119
	bsr		sinf
	bra.b		L_111B
L_1119:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_111A
	bsr		mon_nan
	bra.b		L_111B
L_111A:
	bsr		ssinhd		# assuming a denorm...

L_111B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fsinhx
fsinhx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_111C
	bsr		ssinh		# normalized (regular) number
	bra.b		L_111G
L_111C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_111D
	bsr		szero
	bra.b		L_111G
L_111D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_111E
	bsr		sinf
	bra.b		L_111G
L_111E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_111F
	bsr		mon_nan
	bra.b		L_111G
L_111F:
	bsr		ssinhd		# assuming a denorm...

L_111G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the ftans and ftanx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	stan	
#	xref	szero	
#	xref	t_operr	
#	xref	mon_nan	
#	xref	stand	

	global		ftans
ftans:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1122
	bsr		stan		# normalized (regular) number
	bra.b		L_1126
L_1122:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1123
	bsr		szero
	bra.b		L_1126
L_1123:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1124
	bsr		t_operr
	bra.b		L_1126
L_1124:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1125
	bsr		mon_nan
	bra.b		L_1126
L_1125:
	bsr		stand		# assuming a denorm...

L_1126:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		ftand
ftand:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1127
	bsr		stan		# normalized (regular) number
	bra.b		L_112B
L_1127:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1128
	bsr		szero
	bra.b		L_112B
L_1128:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1129
	bsr		t_operr
	bra.b		L_112B
L_1129:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_112A
	bsr		mon_nan
	bra.b		L_112B
L_112A:
	bsr		stand		# assuming a denorm...

L_112B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		ftanx
ftanx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_112C
	bsr		stan		# normalized (regular) number
	bra.b		L_112G
L_112C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_112D
	bsr		szero
	bra.b		L_112G
L_112D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_112E
	bsr		t_operr
	bra.b		L_112G
L_112E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_112F
	bsr		mon_nan
	bra.b		L_112G
L_112F:
	bsr		stand		# assuming a denorm...

L_112G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the ftanhs and ftanhx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	stanh	
#	xref	szero	
#	xref	sone	
#	xref	mon_nan	
#	xref	stanhd	

	global		ftanhs
ftanhs:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1132
	bsr		stanh		# normalized (regular) number
	bra.b		L_1136
L_1132:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1133
	bsr		szero
	bra.b		L_1136
L_1133:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1134
	bsr		sone
	bra.b		L_1136
L_1134:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1135
	bsr		mon_nan
	bra.b		L_1136
L_1135:
	bsr		stanhd		# assuming a denorm...

L_1136:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		ftanhd
ftanhd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1137
	bsr		stanh		# normalized (regular) number
	bra.b		L_113B
L_1137:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1138
	bsr		szero
	bra.b		L_113B
L_1138:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1139
	bsr		sone
	bra.b		L_113B
L_1139:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_113A
	bsr		mon_nan
	bra.b		L_113B
L_113A:
	bsr		stanhd		# assuming a denorm...

L_113B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		ftanhx
ftanhx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_113C
	bsr		stanh		# normalized (regular) number
	bra.b		L_113G
L_113C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_113D
	bsr		szero
	bra.b		L_113G
L_113D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_113E
	bsr		sone
	bra.b		L_113G
L_113E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_113F
	bsr		mon_nan
	bra.b		L_113G
L_113F:
	bsr		stanhd		# assuming a denorm...

L_113G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the ftentoxs and ftentoxx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	stentox	
#	xref	ld_pone	
#	xref	szr_inf	
#	xref	mon_nan	
#	xref	stentoxd	

	global		ftentoxs
ftentoxs:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1142
	bsr		stentox		# normalized (regular) number
	bra.b		L_1146
L_1142:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1143
	bsr		ld_pone
	bra.b		L_1146
L_1143:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1144
	bsr		szr_inf
	bra.b		L_1146
L_1144:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1145
	bsr		mon_nan
	bra.b		L_1146
L_1145:
	bsr		stentoxd	# assuming a denorm...

L_1146:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		ftentoxd
ftentoxd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1147
	bsr		stentox		# normalized (regular) number
	bra.b		L_114B
L_1147:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1148
	bsr		ld_pone
	bra.b		L_114B
L_1148:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1149
	bsr		szr_inf
	bra.b		L_114B
L_1149:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_114A
	bsr		mon_nan
	bra.b		L_114B
L_114A:
	bsr		stentoxd	# assuming a denorm...

L_114B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		ftentoxx
ftentoxx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_114C
	bsr		stentox		# normalized (regular) number
	bra.b		L_114G
L_114C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_114D
	bsr		ld_pone
	bra.b		L_114G
L_114D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_114E
	bsr		szr_inf
	bra.b		L_114G
L_114E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_114F
	bsr		mon_nan
	bra.b		L_114G
L_114F:
	bsr		stentoxd	# assuming a denorm...

L_114G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the ftwotoxs and ftwotoxx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	stwotox	
#	xref	ld_pone	
#	xref	szr_inf	
#	xref	mon_nan	
#	xref	stwotoxd	

	global		ftwotoxs
ftwotoxs:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1152
	bsr		stwotox		# normalized (regular) number
	bra.b		L_1156
L_1152:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1153
	bsr		ld_pone
	bra.b		L_1156
L_1153:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1154
	bsr		szr_inf
	bra.b		L_1156
L_1154:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1155
	bsr		mon_nan
	bra.b		L_1156
L_1155:
	bsr		stwotoxd	# assuming a denorm...

L_1156:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		ftwotoxd
ftwotoxd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1157
	bsr		stwotox		# normalized (regular) number
	bra.b		L_115B
L_1157:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1158
	bsr		ld_pone
	bra.b		L_115B
L_1158:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1159
	bsr		szr_inf
	bra.b		L_115B
L_1159:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_115A
	bsr		mon_nan
	bra.b		L_115B
L_115A:
	bsr		stwotoxd	# assuming a denorm...

L_115B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		ftwotoxx
ftwotoxx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_115C
	bsr		stwotox		# normalized (regular) number
	bra.b		L_115G
L_115C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_115D
	bsr		ld_pone
	bra.b		L_115G
L_115D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_115E
	bsr		szr_inf
	bra.b		L_115G
L_115E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_115F
	bsr		mon_nan
	bra.b		L_115G
L_115F:
	bsr		stwotoxd	# assuming a denorm...

L_115G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fgetmans and fgetmanx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	sgetman	
#	xref	szero	
#	xref	t_operr	
#	xref	mon_nan	
#	xref	sgetmand	

	global		fgetmans
fgetmans:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1162
	bsr		sgetman		# normalized (regular) number
	bra.b		L_1166
L_1162:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1163
	bsr		szero
	bra.b		L_1166
L_1163:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1164
	bsr		t_operr
	bra.b		L_1166
L_1164:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1165
	bsr		mon_nan
	bra.b		L_1166
L_1165:
	bsr		sgetmand	# assuming a denorm...

L_1166:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fgetmand
fgetmand:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1167
	bsr		sgetman		# normalized (regular) number
	bra.b		L_116B
L_1167:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1168
	bsr		szero
	bra.b		L_116B
L_1168:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1169
	bsr		t_operr
	bra.b		L_116B
L_1169:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_116A
	bsr		mon_nan
	bra.b		L_116B
L_116A:
	bsr		sgetmand	# assuming a denorm...

L_116B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fgetmanx
fgetmanx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_116C
	bsr		sgetman		# normalized (regular) number
	bra.b		L_116G
L_116C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_116D
	bsr		szero
	bra.b		L_116G
L_116D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_116E
	bsr		t_operr
	bra.b		L_116G
L_116E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_116F
	bsr		mon_nan
	bra.b		L_116G
L_116F:
	bsr		sgetmand	# assuming a denorm...

L_116G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the flogns and flognx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	sslogn	
#	xref	t_dz2	
#	xref	sopr_inf	
#	xref	mon_nan	
#	xref	sslognd	

	global		flogns
flogns:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1172
	bsr		sslogn		# normalized (regular) number
	bra.b		L_1176
L_1172:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1173
	bsr		t_dz2
	bra.b		L_1176
L_1173:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1174
	bsr		sopr_inf
	bra.b		L_1176
L_1174:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1175
	bsr		mon_nan
	bra.b		L_1176
L_1175:
	bsr		sslognd		# assuming a denorm...

L_1176:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		flognd
flognd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1177
	bsr		sslogn		# normalized (regular) number
	bra.b		L_117B
L_1177:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1178
	bsr		t_dz2
	bra.b		L_117B
L_1178:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1179
	bsr		sopr_inf
	bra.b		L_117B
L_1179:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_117A
	bsr		mon_nan
	bra.b		L_117B
L_117A:
	bsr		sslognd		# assuming a denorm...

L_117B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		flognx
flognx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_117C
	bsr		sslogn		# normalized (regular) number
	bra.b		L_117G
L_117C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_117D
	bsr		t_dz2
	bra.b		L_117G
L_117D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_117E
	bsr		sopr_inf
	bra.b		L_117G
L_117E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_117F
	bsr		mon_nan
	bra.b		L_117G
L_117F:
	bsr		sslognd		# assuming a denorm...

L_117G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the flog2s and flog2x entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	sslog2	
#	xref	t_dz2	
#	xref	sopr_inf	
#	xref	mon_nan	
#	xref	sslog2d	

	global		flog2s
flog2s:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1182
	bsr		sslog2		# normalized (regular) number
	bra.b		L_1186
L_1182:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1183
	bsr		t_dz2
	bra.b		L_1186
L_1183:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1184
	bsr		sopr_inf
	bra.b		L_1186
L_1184:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1185
	bsr		mon_nan
	bra.b		L_1186
L_1185:
	bsr		sslog2d		# assuming a denorm...

L_1186:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		flog2d
flog2d:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1187
	bsr		sslog2		# normalized (regular) number
	bra.b		L_118B
L_1187:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1188
	bsr		t_dz2
	bra.b		L_118B
L_1188:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1189
	bsr		sopr_inf
	bra.b		L_118B
L_1189:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_118A
	bsr		mon_nan
	bra.b		L_118B
L_118A:
	bsr		sslog2d		# assuming a denorm...

L_118B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		flog2x
flog2x:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_118C
	bsr		sslog2		# normalized (regular) number
	bra.b		L_118G
L_118C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_118D
	bsr		t_dz2
	bra.b		L_118G
L_118D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_118E
	bsr		sopr_inf
	bra.b		L_118G
L_118E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_118F
	bsr		mon_nan
	bra.b		L_118G
L_118F:
	bsr		sslog2d		# assuming a denorm...

L_118G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the flog10s and flog10x entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	sslog10	
#	xref	t_dz2	
#	xref	sopr_inf	
#	xref	mon_nan	
#	xref	sslog10d	

	global		flog10s
flog10s:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1192
	bsr		sslog10		# normalized (regular) number
	bra.b		L_1196
L_1192:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1193
	bsr		t_dz2
	bra.b		L_1196
L_1193:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1194
	bsr		sopr_inf
	bra.b		L_1196
L_1194:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1195
	bsr		mon_nan
	bra.b		L_1196
L_1195:
	bsr		sslog10d	# assuming a denorm...

L_1196:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		flog10d
flog10d:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1197
	bsr		sslog10		# normalized (regular) number
	bra.b		L_119B
L_1197:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1198
	bsr		t_dz2
	bra.b		L_119B
L_1198:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1199
	bsr		sopr_inf
	bra.b		L_119B
L_1199:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_119A
	bsr		mon_nan
	bra.b		L_119B
L_119A:
	bsr		sslog10d	# assuming a denorm...

L_119B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		flog10x
flog10x:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_119C
	bsr		sslog10		# normalized (regular) number
	bra.b		L_119G
L_119C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_119D
	bsr		t_dz2
	bra.b		L_119G
L_119D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_119E
	bsr		sopr_inf
	bra.b		L_119G
L_119E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_119F
	bsr		mon_nan
	bra.b		L_119G
L_119F:
	bsr		sslog10d	# assuming a denorm...

L_119G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the flognp1s and flognp1x entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	sslognp1	
#	xref	szero	
#	xref	sopr_inf	
#	xref	mon_nan	
#	xref	slognp1d	

	global		flognp1s
flognp1s:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1202
	bsr		sslognp1	# normalized (regular) number
	bra.b		L_1206
L_1202:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1203
	bsr		szero
	bra.b		L_1206
L_1203:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1204
	bsr		sopr_inf
	bra.b		L_1206
L_1204:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1205
	bsr		mon_nan
	bra.b		L_1206
L_1205:
	bsr		slognp1d	# assuming a denorm...

L_1206:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		flognp1d
flognp1d:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1207
	bsr		sslognp1	# normalized (regular) number
	bra.b		L_120B
L_1207:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1208
	bsr		szero
	bra.b		L_120B
L_1208:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1209
	bsr		sopr_inf
	bra.b		L_120B
L_1209:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_120A
	bsr		mon_nan
	bra.b		L_120B
L_120A:
	bsr		slognp1d	# assuming a denorm...

L_120B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		flognp1x
flognp1x:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_120C
	bsr		sslognp1	# normalized (regular) number
	bra.b		L_120G
L_120C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_120D
	bsr		szero
	bra.b		L_120G
L_120D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_120E
	bsr		sopr_inf
	bra.b		L_120G
L_120E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_120F
	bsr		mon_nan
	bra.b		L_120G
L_120F:
	bsr		slognp1d	# assuming a denorm...

L_120G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fints and fintx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	l_sint	
#	xref	szero	
#	xref	sinf	
#	xref	mon_nan	
#	xref	l_sintd	

	global		fints
fints:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1212
	bsr		l_sint		# normalized (regular) number
	bra.b		L_1216
L_1212:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1213
	bsr		szero
	bra.b		L_1216
L_1213:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1214
	bsr		sinf
	bra.b		L_1216
L_1214:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1215
	bsr		mon_nan
	bra.b		L_1216
L_1215:
	bsr		l_sintd		# assuming a denorm...

L_1216:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fintd
fintd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1217
	bsr		l_sint		# normalized (regular) number
	bra.b		L_121B
L_1217:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1218
	bsr		szero
	bra.b		L_121B
L_1218:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1219
	bsr		sinf
	bra.b		L_121B
L_1219:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_121A
	bsr		mon_nan
	bra.b		L_121B
L_121A:
	bsr		l_sintd		# assuming a denorm...

L_121B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fintx
fintx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_121C
	bsr		l_sint		# normalized (regular) number
	bra.b		L_121G
L_121C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_121D
	bsr		szero
	bra.b		L_121G
L_121D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_121E
	bsr		sinf
	bra.b		L_121G
L_121E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_121F
	bsr		mon_nan
	bra.b		L_121G
L_121F:
	bsr		l_sintd		# assuming a denorm...

L_121G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fintrzs and fintrzx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	l_sintrz	
#	xref	szero	
#	xref	sinf	
#	xref	mon_nan	
#	xref	snzrinx	

	global		fintrzs
fintrzs:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1222
	bsr		l_sintrz	# normalized (regular) number
	bra.b		L_1226
L_1222:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1223
	bsr		szero
	bra.b		L_1226
L_1223:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1224
	bsr		sinf
	bra.b		L_1226
L_1224:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1225
	bsr		mon_nan
	bra.b		L_1226
L_1225:
	bsr		snzrinx		# assuming a denorm...

L_1226:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fintrzd
fintrzd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1227
	bsr		l_sintrz	# normalized (regular) number
	bra.b		L_122B
L_1227:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1228
	bsr		szero
	bra.b		L_122B
L_1228:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1229
	bsr		sinf
	bra.b		L_122B
L_1229:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_122A
	bsr		mon_nan
	bra.b		L_122B
L_122A:
	bsr		snzrinx		# assuming a denorm...

L_122B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fintrzx
fintrzx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_122C
	bsr		l_sintrz	# normalized (regular) number
	bra.b		L_122G
L_122C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_122D
	bsr		szero
	bra.b		L_122G
L_122D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_122E
	bsr		sinf
	bra.b		L_122G
L_122E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_122F
	bsr		mon_nan
	bra.b		L_122G
L_122F:
	bsr		snzrinx		# assuming a denorm...

L_122G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	DYADIC.GEN 1.2 4/30/91
#
#	DYADIC.GEN --- generic DYADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in USER_FPCR(a6) so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete updating of the fpsr if you only care about
#		   the result.
#		4. Remove the frems and fremx entry points if your compiler
#		   treats everything as doubles.
#		5. Move the result to d0/d1 if the compiler is that old.
#

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	prem	
#	xref	tag	

	global		frems
frems:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.s		12(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		prem

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

	global		fremd
fremd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.d		16(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		prem

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

	global		fremx
fremx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.x		20(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		prem

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

#
#	DYADIC.GEN 1.2 4/30/91
#
#	DYADIC.GEN --- generic DYADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in USER_FPCR(a6) so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete updating of the fpsr if you only care about
#		   the result.
#		4. Remove the fmods and fmodx entry points if your compiler
#		   treats everything as doubles.
#		5. Move the result to d0/d1 if the compiler is that old.
#

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	pmod	
#	xref	tag	

	global		fmods
fmods:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.s		12(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		pmod

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

	global		fmodd
fmodd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.d		16(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		pmod

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

	global		fmodx
fmodx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.x		20(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		pmod

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

#
#	DYADIC.GEN 1.2 4/30/91
#
#	DYADIC.GEN --- generic DYADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in USER_FPCR(a6) so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete updating of the fpsr if you only care about
#		   the result.
#		4. Remove the fscales and fscalex entry points if your compiler
#		   treats everything as doubles.
#		5. Move the result to d0/d1 if the compiler is that old.
#

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	pscale	
#	xref	tag	

	global		fscales
fscales:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.s		12(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		pscale

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

	global		fscaled
fscaled:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.d		16(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		pscale

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

	global		fscalex
fscalex:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.x		20(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		pscale

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fabss and fabsx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	sabs	
#	xref	sabs	
#	xref	sabs	
#	xref	sabs	
#	xref	sabs	

	global		fabss
fabss:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1262
	bsr		sabs		# normalized (regular) number
	bra.b		L_1266
L_1262:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1263
	bsr		sabs
	bra.b		L_1266
L_1263:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1264
	bsr		sabs
	bra.b		L_1266
L_1264:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1265
	bsr		sabs
	bra.b		L_1266
L_1265:
	bsr		sabs		# assuming a denorm...

L_1266:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fabsd
fabsd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1267
	bsr		sabs		# normalized (regular) number
	bra.b		L_126B
L_1267:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1268
	bsr		sabs
	bra.b		L_126B
L_1268:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1269
	bsr		sabs
	bra.b		L_126B
L_1269:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_126A
	bsr		sabs
	bra.b		L_126B
L_126A:
	bsr		sabs		# assuming a denorm...

L_126B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fabsx
fabsx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_126C
	bsr		sabs		# normalized (regular) number
	bra.b		L_126G
L_126C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_126D
	bsr		sabs
	bra.b		L_126G
L_126D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_126E
	bsr		sabs
	bra.b		L_126G
L_126E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_126F
	bsr		sabs
	bra.b		L_126G
L_126F:
	bsr		sabs		# assuming a denorm...

L_126G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fnegs and fnegx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	sneg	
#	xref	sneg	
#	xref	sneg	
#	xref	sneg	
#	xref	sneg	

	global		fnegs
fnegs:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1272
	bsr		sneg		# normalized (regular) number
	bra.b		L_1276
L_1272:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1273
	bsr		sneg
	bra.b		L_1276
L_1273:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1274
	bsr		sneg
	bra.b		L_1276
L_1274:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1275
	bsr		sneg
	bra.b		L_1276
L_1275:
	bsr		sneg		# assuming a denorm...

L_1276:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fnegd
fnegd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1277
	bsr		sneg		# normalized (regular) number
	bra.b		L_127B
L_1277:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1278
	bsr		sneg
	bra.b		L_127B
L_1278:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1279
	bsr		sneg
	bra.b		L_127B
L_1279:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_127A
	bsr		sneg
	bra.b		L_127B
L_127A:
	bsr		sneg		# assuming a denorm...

L_127B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fnegx
fnegx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_127C
	bsr		sneg		# normalized (regular) number
	bra.b		L_127G
L_127C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_127D
	bsr		sneg
	bra.b		L_127G
L_127D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_127E
	bsr		sneg
	bra.b		L_127G
L_127E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_127F
	bsr		sneg
	bra.b		L_127G
L_127F:
	bsr		sneg		# assuming a denorm...

L_127G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	MONADIC.GEN 1.5 5/18/92
#
#
#	MONADIC.GEN 1.4 1/16/92
#
#	MONADIC.GEN 1.3 4/30/91
#
#	MONADIC.GEN --- generic MONADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in d1 so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete handling of the fpsr if you only care about
#		   the result.  
#		4. Some (most?) C compilers convert all float arguments
#		   to double, and provide no support at all for extended
#		   precision so remove the fsqrts and fsqrtx entry points.
#		5. Move the result to d0/d1 if the compiler is that old.

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	tag	
#	xref	ssqrt	
#	xref	ssqrt	
#	xref	ssqrt	
#	xref	ssqrt	
#	xref	ssqrt	

	global		fsqrts
fsqrts:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1282
	bsr		ssqrt		# normalized (regular) number
	bra.b		L_1286
L_1282:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1283
	bsr		ssqrt
	bra.b		L_1286
L_1283:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1284
	bsr		ssqrt
	bra.b		L_1286
L_1284:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_1285
	bsr		ssqrt
	bra.b		L_1286
L_1285:
	bsr		ssqrt		# assuming a denorm...

L_1286:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fsqrtd
fsqrtd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_1287
	bsr		ssqrt		# normalized (regular) number
	bra.b		L_128B
L_1287:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_1288
	bsr		ssqrt
	bra.b		L_128B
L_1288:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_1289
	bsr		ssqrt
	bra.b		L_128B
L_1289:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_128A
	bsr		ssqrt
	bra.b		L_128B
L_128A:
	bsr		ssqrt		# assuming a denorm...

L_128B:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

	global		fsqrtx
fsqrtx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmov.l		%status,USER_FPSR(%a6)
	fmov.l		%control,USER_FPCR(%a6)
	fmov.l		%control,%d1	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input argument
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)
	tst.b		%d0
	bne.b		L_128C
	bsr		ssqrt		# normalized (regular) number
	bra.b		L_128G
L_128C:
	cmp.b		%d0,&0x20	# zero?
	bne.b		L_128D
	bsr		ssqrt
	bra.b		L_128G
L_128D:
	cmp.b		%d0,&0x40	# infinity?
	bne.b		L_128E
	bsr		ssqrt
	bra.b		L_128G
L_128E:
	cmp.b		%d0,&0x60	# NaN?
	bne.b		L_128F
	bsr		ssqrt
	bra.b		L_128G
L_128F:
	bsr		ssqrt		# assuming a denorm...

L_128G:
	fmov.l		%status,%d0	# update status register
	or.b		USER_FPSR+3(%a6),%d0	# add previously accrued exceptions
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	fmov.l		USER_FPCR(%a6),%control	# fpcr restored
	unlk		%a6
	rts

#
#	DYADIC.GEN 1.2 4/30/91
#
#	DYADIC.GEN --- generic DYADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in USER_FPCR(a6) so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete updating of the fpsr if you only care about
#		   the result.
#		4. Remove the fadds and faddx entry points if your compiler
#		   treats everything as doubles.
#		5. Move the result to d0/d1 if the compiler is that old.
#

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	sadd	
#	xref	tag	

	global		fadds
fadds:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.s		12(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		sadd

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

	global		faddd
faddd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.d		16(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		sadd

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

	global		faddx
faddx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.x		20(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		sadd

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

#
#	DYADIC.GEN 1.2 4/30/91
#
#	DYADIC.GEN --- generic DYADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in USER_FPCR(a6) so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete updating of the fpsr if you only care about
#		   the result.
#		4. Remove the fsubs and fsubx entry points if your compiler
#		   treats everything as doubles.
#		5. Move the result to d0/d1 if the compiler is that old.
#

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	ssub	
#	xref	tag	

	global		fsubs
fsubs:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.s		12(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		ssub

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

	global		fsubd
fsubd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.d		16(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		ssub

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

	global		fsubx
fsubx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.x		20(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		ssub

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

#
#	DYADIC.GEN 1.2 4/30/91
#
#	DYADIC.GEN --- generic DYADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in USER_FPCR(a6) so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete updating of the fpsr if you only care about
#		   the result.
#		4. Remove the fmuls and fmulx entry points if your compiler
#		   treats everything as doubles.
#		5. Move the result to d0/d1 if the compiler is that old.
#

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	smul	
#	xref	tag	

	global		fmuls
fmuls:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.s		12(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		smul

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

	global		fmuld
fmuld:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.d		16(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		smul

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

	global		fmulx
fmulx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.x		20(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		smul

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

#
#	DYADIC.GEN 1.2 4/30/91
#
#	DYADIC.GEN --- generic DYADIC template
#
#	This version saves all registers that will be used by the emulation
#	routines and restores all but FP0 on exit.  The FPSR is
#	updated to reflect the result of the operation.  Return value
#	is placed in FP0 for single, double and extended results.
#	
#	The package subroutines expect the incoming FPCR to be zeroed
#	since they need extended precision to work properly.  The
#	'final' FPCR is expected in USER_FPCR(a6) so that the calculated result
#	can be properly sized and rounded.  Also, if the incoming FPCR
#	has enabled any exceptions, the exception will be taken on the
#	final fmovem in this template.
#
#	Customizations:  
#		1. Remove the movem.l at the entry and exit of
#		   each routine if your compiler treats those 
#		   registers as scratch.
#		2. Likewise, don't save FP0/FP1 if they are scratch
#		   registers.
#		3. Delete updating of the fpsr if you only care about
#		   the result.
#		4. Remove the fdivs and fdivx entry points if your compiler
#		   treats everything as doubles.
#		5. Move the result to d0/d1 if the compiler is that old.
#

#		Copyright (C) Motorola, Inc. 1991
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README, in this same directory.

#	xref	sdiv	
#	xref	tag	

	global		fdivs
fdivs:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.s		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.s		12(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		sdiv

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

	global		fdivd
fdivd:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.d		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.d		16(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		sdiv

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

	global		fdivx
fdivx:
	link		%a6,&-LOCAL_SIZE
	movm.l		&0x303,USER_DA(%a6)	#  {%d0-%d1/%a0-%a1}
	fmovm.x		&0xf0,USER_FP0(%a6)	#  {%fp0-%fp3}
	fmovm.l		%status,%control,USER_FPSR(%a6)	# user's rounding mode/precision
	fmov.l		&0,%control	# force rounding mode/prec to extended,rn
#
#	copy, convert and tag input arguments
#
	fmov.x		8(%a6),%fp0
	fmov.x		%fp0,FPTEMP(%a6)
	lea		FPTEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,DTAG(%a6)

	fmov.x		20(%a6),%fp0
	fmov.x		%fp0,ETEMP(%a6)
	lea		ETEMP(%a6),%a0
	bsr		tag
	mov.b		%d0,STAG(%a6)

	bsr		sdiv

	fmov.l		%status,%d0	# update status register
	or.b		FPSR_AEXCEPT(%a6),%d0	# add previously accrued exceptions
	swap.w		%d0
	or.b		FPSR_QBYTE(%a6),%d0	# pickup sign of quotient byte
	swap.w		%d0
	fmov.l		%d0,%status
#
#	Result is now in FP0
#
	movm.l		USER_DA(%a6),&0x303	#  {%d0-%d1/%a0-%a1}
	fmovm.x		USER_FP1(%a6),&0x70	# note: FP0 not restored {%fp1-%fp3}
	unlk		%a6
	rts

#	end		
