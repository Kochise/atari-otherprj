#
# aza.s 1.2 1/21/92
#
# add - zero - add case
# passes
	text
	even
	global	main
inf:
	long	0x7f800000
zero:
	long	0x00000000
nan:
	long	0x7fffffff
nm1:	
	long	0x3f800000
dn1:
	long	0x00000000,0x0000ffff,0xffffffff
main:
	link	%fp,&0	
	fmov.s	nm1,%fp1
	fmov.s	zero,%fp3
	fmov.s	nm1,%fp4
	fmov.x	dn1,%fp2 # fp2 is FPa
#
#i) source and dest are extended denorms
#
	fadd.x	dn1,%fp2
#
#ii) fp4 should contain a tag other than *norm*
# notice: fp3 contains a zero, so tag(fp4) should be *zero*
#
	fmov.x	%fp3,%fp4 # fp3 is FPb and fp4 is FPc
#
#iii) source is FPc, tag should be zero, and result should be +1.0
#
	fadd.x	%fp4,%fp1

	fmov.x	%fp1,-(%sp)
        mov.l   &L%17,-(%sp)
	jsr	printf
	add.l	&16,%sp

	fmov.x	%fp2,-(%sp)
        mov.l   &L%17,-(%sp)
	jsr	printf
	add.l	&16,%sp

	fmov.x	%fp3,-(%sp)
        mov.l   &L%17,-(%sp)
	jsr	printf
	add.l	&16,%sp

	fmov.x	%fp4,-(%sp)
        mov.l   &L%17,-(%sp)
	jsr	printf
	add.l	&16,%sp

	unlk	%fp
	rts
L%17:
	byte	'%,'0,'8,'x,'_,'%,'0,'8
	byte	'x,'_,'%,'0,'8,'x,'\n,0x00
