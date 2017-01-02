# quotient bits set in fpsr
# passes
	text
	even
	global	main
nm1:
	long	0x00ff0000
nm2:
	long	0x3f800000
dn1:
	long	0x00000000,0x0000ffff,0xffffffff
main:
	link	%fp,&0	
	fmov.l	nm1,%fpsr
	fintrz.snm2,%fp0
	fmov.l	%fpsr,%d0
	fmov.l	%d0,%fp2 # fp2 is FPa
#
#i) source and dest are extended denorms
#
#

	fmov.x	%fp2,-(%sp)
        mov.l   &L%17,-(%sp)
	jsr	printf
	add.l	&16,%sp

	
	unlk	%fp
	rts
L%17:
	byte	'%,'0,'8,'x,'_,'%,'0,'8
	byte	'x,'_,'%,'0,'8,'x,'\n,0x00
