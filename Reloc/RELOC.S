********************************************
*                                          *
* << Relocation routine : >>               *
* --------------------------               *
*                                          *
* << Code : GT Turbo >>                    *
*                                          *
*                                          *
*                                          *
*                                          *
*   Cerebral Vortex Software Developpment  *
*                                          *
*                                          *
*              http://Cerebral-Vortex.net  *
*                                          *
********************************************


	 TEXT 


;**********************************************
; Reloc routine
;
; a0.l : Adress of file to relocate.
;

Reloc:
	movem.l	a0-a1/d0-d2,-(a7) 

	movea.l	a0,a1 
	adda.l	2(a0),a1
	adda.l	6(a0),a1
	adda.l	14(a0),a1 
	adda.l	#$1c,a1 
	adda.l	#$1c,a0 
	move.l	a0,d2 
	move.l	(a1)+,d0
	beq.s	Reloc_end 

	adda.l	d0,a0 
	add.l	d2,(a0) 
	move.w	#$fe,d1 
	moveq	#0,d0 
On_continue:
	move.b	(a1)+,d0
	beq.s	Reloc_end 

	cmp.b	#1,d0 
	bne.s	Reloc_seulement
	adda.l	d1,a0 
	bra.s	On_continue

Reloc_seulement:

	adda.l	d0,a0 
	add.l	d2,(a0) 
	bra.s	On_continue

Reloc_end:
	movem.l	(a7)+,a0-a1/d0-d2 
	rts
