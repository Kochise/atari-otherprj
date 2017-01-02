# add - nan - add packed decimal case
# fails by fp1 destination operand
	text
	even
	global	main
nm2:
	long	0x40000000
nm1:	
	long	0x00000001,0x00000000,0x00000000,0x00000000
dn1:
	long	0x00000000,0x0000ffff,0xffffffff
main:
	link	%fp,&0	
	fmov.s	nm2,%fp1
#	fmov.p	nm1,%fp3
#
#i) source and dest are (1.0 + 2.0)
#
	fadd.p	nm1,%fp1
#

	fmov.x	%fp1,-(%sp)
        mov.l   &L%17,-(%sp)
	jsr	printf
	add.l	&16,%sp

#	fmov.x	%fp2,-(%sp)
#       mov.l   &L%17,-(%sp)
#	jsr	printf
#	add.l	&16,%sp

#	fmov.x	%fp3,-(%sp)
#       mov.l   &L%17,-(%sp)
#	jsr	printf
#	add.l	&16,%sp

#	fmov.x	%fp4,-(%sp)
#       mov.l   &L%17,-(%sp)
#	jsr	printf
#	add.l	&16,%sp

	unlk	%fp
	rts
L%17:
	byte	'%,'0,'8,'x,'_,'%,'0,'8
	byte	'x,'_,'%,'0,'8,'x,'\n,0x00
