		output		E:\PROGRAMS\ANALYSIS\ANALYSIS.BIN

;		Equates.
		incdir		E:\ASSEMBLE\EQUS\
; EQUs g‚n‚raux.
		include		GENERAL.S

;		Macros du systŠme d'exploitation.
		incdir		E:\ASSEMBLE\MACROS\
; TOS.
		include		GEMDOS.S
		include		BIOS.S
		include		XBIOS.S
; GEM.
;		include		AES.S
;		include		VDI.S

;****************************************************************
;		Routines assembleur pour l'analyse de sons
;		en GFABASIC
;****************************************************************


;****************************************************************
;		Gestion des appels.
;****************************************************************
Call:		lea		(4.w,a7),a6 ; Adresse des paramŠtres.

		moveq		#-1,d0 ; Erreur.
		move.w		(a6)+,d1 ; Num‚ro de la routine.
		bmi.s		Call_err
		cmpi.w		#rout_nb,d1
		bge.s		Call_err

		move.l		(Rout_tbl.b,pc,d1.w*4),d1
		jsr		(Rout_tbl.b,pc,d1.l)
Call_err:	rts

rout_nb		equ		9
Rout_tbl:	dc.l		Calc_FFT-Rout_tbl
		dc.l		Draw_signal-Rout_tbl
		dc.l		Draw_spec-Rout_tbl

		dc.l		Cls-Rout_tbl
		dc.l		Clr_area
		dc.l		Line-Rout_tbl

		dc.l		Cv_8m_2_16s-Rout_tbl
		dc.l		Cv_8s_2_16s-Rout_tbl
		dc.l		Cv_16m_2_16s-Rout_tbl
;****************************************************************


;****************************************************************
;		Calcul de la FFT.
;****************************************************************
Calc_FFT:	movea.l		(fft_pb.signal_addr.w,a6),a0
		movea.l		(fft_pb.spec_addr.w,a6),a1
		movea.l		(fft_pb.window_addr.w,a6),a2
		lea		$a204.w,a3 ; Adresse de transfer du HOST.
		lea		$a202.w,a4 ; Adresse du registre de status.

Send_host_lp0:	btst		#1,(a4)
		beq.s		Send_host_lp0
		move.l		#1,(a3) ; Commande d'envoie au DSP.

;		Envoie du signal au DSP.
Send_host_lp1:	btst		#1,(a4)
		beq.s		Send_host_lp1

		move.w		#fft_point_nb/16-1,d7
		tst.w		(fft_pb.fg.w,a6) ; Si le flag n'est pas mis, on
		beq		Calc_FFT_ok0 ; utilise pas de fenetre.
; Utilisation d'une fenetre.
		moveq		#0,d2
		moveq		#16+2,d1 ; D‚calage.
Calc_FFT_lp0:	rept		16
		moveq		#0,d0
		move.w		(a0),d0
		addq.l		#4,a0
		muls.l		(a2)+,d0 ; Coeficient de la fenetre.
		asr.l		d1,d0 ; Signal sur 14 bits pour eviter les d‚bordements.
		move.l		d0,(a3) ; Envoie au DSP.
		clr.l		(a3) ; Partie imaginaire nulle.
		endr
		dbf		d7,Calc_FFT_lp0
		bra		Calc_FFT_ok1
; Pas d'utilisation d'une fenetre.
Calc_FFT_ok0:	moveq		#0,d2
Calc_FFT_lp1:	rept		16
		move.w		(a0),d0
		addq.l		#4,a0
		ext.l		d0
		asr.l		#2,d0 ; Signal sur 14 bits pour ‚viter les d‚bordements.
		move.l		d0,(a3) ; Envoie au DSP.
		clr.l		(a3) ; Partie imaginaire nulle.
		endr
		dbf		d7,Calc_FFT_lp1
Calc_FFT_ok1:	

;		R‚ception du spectre.
		moveq		#0,d6
		move.w		#fft_point_nb/2-1,d7
Recv_host_lp0:	btst		#0,(a4)
		beq.s		Recv_host_lp0

Calc_FFT_lp2:	move.l		(a3),d0 ; xr: partie r‚elle sur 24 bits.
		lsr.l		#8,d0 ; Se met sur 16 bits.
		muls.w		d0,d0 ; xr^2.
		move.l		(a3),d1 ; xi: partie imaginaire sur 24 bits.
		lsr.l		#8,d1 ; Se met sur 16 bits.
		muls.w		d1,d1 ; xi^2.
		add.l		d1,d0
		asr.l		#8,d0

;		Calcul de la racine carr‚ de M=xr^2+xi^2.
; Calcul de x(0) avec M sur 32 bits.
		moveq		#0,d1
		bfffo		d0{0:0},d2 ; Cherche 1er bit … 1: n=INT(ln(M)/ln(2)).
		beq.s		Calc_FFT_ok2 ; Si nul, pas besoin de calcul.
		neg.w		d2 ; bfffo donne le d‚placement par rapport au bit 31, il faut donc le
		addi.w		#31+1,d2 ; calculer par rapport au bit 0. +1->nombre de bits.
		lsr.w		#1,d2 ; La moiti‚ des bits.
		move.l		d0,d1 ; M.
		lsr.l		d2,d1 ; x(0)=M/(2^(INT(ln(M)/ln(2))/2))
		addq.l		#1,d1 ; x(0)=M/(2^(INT(ln(M)/ln(2))/2))+1

		rept		4
		move.l		d0,d2 ; M.
		divu.l		d1,d2 ; M/x(n).
		add.l		d2,d1 ; x(n)+M/x(n)
		lsr.l		#1,d1 ; x(n+1)=(x(n)+M/x(n))/2.
		endr

;		Sauve la valeur.
Calc_FFT_ok2:	move.l		d1,(a1)+
		add.l		d1,d6
		dbf		d7,Calc_FFT_lp2

		move.l		d6,d0 ; Puissance totale du signal.
		rts
;****************************************************************


;****************************************************************
;		Routines d'affichage.
;****************************************************************
;----------------------------------------------------------------
;		Affichage du signal.
;----------------------------------------------------------------
Draw_signal:	movea.l		(signal_pb.signal_addr.w,a6),a0 ; Adresse du signal.
		movea.l		(signal_pb.scr_addr.w,a6),a1
		lea		(Ln_tbl.w,pc),a4

;		Pr‚calculs pour l'‚cran.
		move.w		(signal_pb.disp_y.w,a6),d0
;		mulu.w		#80,d0
;		adda.l		d0,a1 ; Adresse de la ligne d'affichage du signal.
		adda.l		(a4,d0.w*4),a1 ; Adresse de la ligne d'affichage du signal.

		moveq		#0,d0
		move.w		(signal_pb.disp_x.w,a6),d0
		move.l		d0,d1
		lsr.w		#4,d1 ; Offset du bloc de 16 contenant le premier pixel.
		add.l		d1,d1 ; *2: offset en mots.
		adda.l		d1,a1 ; Adresse du bloc de 16 pixels contenant le premier.

		andi.w		#$f,d0 ; Modulo 16.
		move.w		#$8000,d5
		lsr.w		d0,d5 ; Masque pour le premier pixel.

;		Pr‚calculs pour l'affichage.
		move.l		(signal_pb.l_limit.w,a6),d0
		move.l		(signal_pb.r_limit.w,a6),d1
		cmp.l		d0,d1 ; V‚rifie que l_limit<r_limit.
		bge.s		Draw_signal_ok0
		exg		d0,d1 ; Echange sinon.
Draw_signal_ok0: sub.l		d0,d1 ; width-1.
		addq.l		#1,d1 ; width.
		
		lsl.l		#2,d0 ; *4.
		adda.l		d0,a0 ; Adresse du d‚but de la partie … afficher.

		move.w		(signal_pb.accuracy.w,a6),d3 ; accuracy.
		move.w		d3,d2
		subq.w		#1,d3
		movea.w		d3,a3 ; accuracy-1.

		move.w		(signal_pb.disp_w.w,a6),d7 ; disp_w.
		lsl.w		#4,d2 ; 16*accuracy: nombre de boucle par colonne.
		mulu.w		d7,d2 ; disp_w*16*accuracy.
		divul.l		d2,d0:d1 ; stp=width/(disp_w*16*accuracy) (partie entiŠre).
		moveq		#0,d3
		divu.l		d2,d0:d3
		movea.l		d3,a2 ; stp (partie d‚cimale).

;		Affichage.
		moveq		#0,d2 ; n=0.
		moveq		#0,d3
		move.w		(signal_pb.zoom_v.w,a6),d4
		bra		Draw_signal_lp0_ok
Draw_signal_lp0: move.w		a3,d6 ; nlp-1.

Draw_signal_lp1: rept		16
		move.w		(a0,d3.l*4),d0
		neg.w		d0 ; L'axe des ordonn‚es est orient‚ vers le bas.
		asr.w		d4,d0 ; R‚duit la hauteur.
;		muls.w		#80,d0
		move.l		(a4,d0.w*4),d0
		or.w		d5,(a1,d0.l) ; Affiche pixel.
		add.l		a2,d2
		addx.l		d1,d3 ; n=n+stp.
		endr
		dbf		d6,Draw_signal_lp1

		ror.w		#1,d5 ; Colonne suivante.
		bcc.s		Draw_signal_ok1
		addq.l		#2,a1 ; On passe au prochain mot.
Draw_signal_ok1:

Draw_signal_lp0_ok: dbf		d7,Draw_signal_lp0
		rts
Ln_tbl0:	incbin		E:\ASSEMBLE\DATAS\LN_TBL.BIN
Ln_tbl		equ		Ln_tbl0+512*4

;----------------------------------------------------------------
;		Affichage du spectre.
;----------------------------------------------------------------
Draw_spec:	movea.l		(spec_pb.spec_addr.w,a6),a0
		movea.l		(spec_pb.scr_addr.w,a6),a1

;		Pr‚calculs pour l'‚cran.
		move.w		(spec_pb.disp_y.w,a6),d0
		move.w		(spec_pb.disp_h.w,a6),d6
		add.w		d6,d0
		subq.w		#1,d0
		mulu.w		#80,d0
		adda.l		d0,a1 ; Adresse de la ligne d'affichage du signal.

		moveq		#0,d0
		move.w		(spec_pb.disp_x.w,a6),d0
		move.l		d0,d1
		lsr.w		#4,d1 ; Offset en mot du bloc de 16 contenant le premier pixel.
		add.l		d1,d1 ; *2: offset en octet.
		adda.l		d1,a1 ; Adresse du bloc de 16 pixels contenant le premier.

		andi.w		#$f,d0 ; Modulo 16.
		move.w		#$8000,d5
		lsr.w		d0,d5 ; Masque pour le premier pixel.

;		Pr‚calculs pour l'affichage.
		move.w		d6,d4
		mulu.w		(spec_pb.zoom.w,a6),d4
		moveq		#10,d3 ; 2^10=1024.

;		Affichage.
		movea.w		#80,a3
		moveq		#$3f,d2 ; Masque modulo 64.
		move.w		#512-1,d7
Draw_spec_lp0:

; Calcul de la hauteur de la colonne.
		move.l		(a0)+,d0
		mulu.l		d4,d0 ; h&=px%(i&)*spec_h&*spec_zoom&.
		lsr.l		d3,d0 ; h&=px%(i&)*spec_h&*spec_zoom&/1024.
		cmp.w		d6,d0 ; Si h&>spec_h&,
		ble.s		Draw_spec_ok0
		move.w		d6,d0 ; alors h&=spec_h&.
Draw_spec_ok0:

; Affichage d'une colonne.
		lea		(a1),a2
		move.w		d0,d1
		lsr.w		#6,d0 ; /64.
		and.w		d2,d1
		neg.w		d1
		jmp		(Draw_spec_lp1_ok.w,pc,d1.w*4)
Draw_spec_lp1:	rept		64
		or.w		d5,(a2)
		suba.l		a3,a2
		endr
Draw_spec_lp1_ok: dbf		d0,Draw_spec_lp1

; Colonne suivante.
		ror.w		#1,d5 ; Colonne suivante.
		bcc.s		Draw_spec_ok1
		addq.l		#2,a1 ; On passe au prochain mot.
Draw_spec_ok1:

		dbf		d7,Draw_spec_lp0
		rts
;****************************************************************


;****************************************************************
;		Routines graphiques.
;****************************************************************
;----------------------------------------------------------------
;		Effacement de l'‚cran 640*400 en monochrome.
; La routine s'execute en environ: 12+8+12+(11*4)+4+(37*(18*(12+(12*8))+12))+(12+(8*8))
;soit 72528 cycles(trŠs approximatif et par large excŠs).
;----------------------------------------------------------------
Cls:		move.l		(cls_pb.scr_addr.w,a6),a0 ; Adresse de l'‚cran.
		lea		(32000.w,a0),a0 ; Adresse de la fin de l'‚cran.
		move.l		(cls_pb.col.w,a6),d0
		move.l		d0,d1
		move.l		d0,d2
		move.l		d0,d3
		move.l		d0,d4
		move.l		d0,d5
		move.l		d0,d6
		movea.l		d0,a1
		movea.l		d0,a2
		movea.l		d0,a3
		movea.l		d0,a4
		movea.l		d0,a5

		moveq		#37-1,d7 ; INT(32000/(12*4))/18=37
Cls_lp:		rept		18
		movem.l		d0-d6/a1-a5,-(a0) ; 12*4=48 octets.
		endr
		dbf		d7,Cls_lp
		movem.l		d0-d6/a1,-(a0) ; 32000 MOD (12*4)=32=8*4
		rts

;----------------------------------------------------------------
;		Effacage d'une zone rectangulaire.
;----------------------------------------------------------------
Clr_area:
		rts

;----------------------------------------------------------------
;		Tracage de lignes en monochrome.
;----------------------------------------------------------------
Line:		bsr		Ln_clip
		tst.l		d0
		bmi		Line_end

		movea.l		(ln_pb.scr_addr.w,a6),a0
		movem.w		(ln_pb.x1.w,a6),d4-d7
		movea.w		#80,a1 ; Taille d'une ligne en octets.

; x1 doit etre inferieur a x2.
		cmp.w		d6,d4
		ble.s		Line_ok00
		exg		d4,d6
Line_ok00:

; Calcul de dx et dy.
		sub.w		d4,d6 ; dx=x2-x1
		sub.w		d5,d7 ; dy=y2-y1
		bpl.s		Line_ok01
		neg.w		d7
		movea.w		#-80,a1
Line_ok01:

; Adresse du premier pixel.
		muls.w		#80,d5
		adda.l		d5,a0 ; Adresse de la ligne du premier sommet du segment.

		moveq		#0,d0
		move.w		d4,d0
		lsr.w		#4,d0 ; /16.
		add.w		d0,d0 ; *2.
		add.l		d0,a0 ; Adresse du plan du premier sommet du segment.

		move.w		d4,d0
		andi.w		#$f,d0
		moveq		#1,d3
		lsr.w		d0,d3 ; Masque.

; Cas particuliers: ligne verticale ou horizontale.
		tst.w		d6
		beq.s		Line_v
		tst.w		d7
		beq.s		Line_h

; Si dx>dy.
		cmp.w		d6,d7
		blt.s		Line2

; Cas ou dx<dy.
		moveq		#0,d1
		sub.w		d7,d1
		bra.s		Line_lp_ok
Line_lp:	add.w		d6,d1
		bpl.s		Line_ok1
		sub.w		d7,d1
		ror.w		#1,d3
		bcc.s		Line_ok1
		addq.l		#2,a0
Line_ok1:

Line_lp_ok:	or.w		d3,(a0)
		adda.l		a1,a0
		dbf		d7,Line_lp
Line_end:	rts

; Cas ou dx>dy.
Line2:		moveq		#0,d1
		sub.w		d6,d1
		bra.s		Line2_lp_ok
Line2_lp:	add.w		d7,d1
		bpl.s		Line2_ok0
		sub.w		d6,d1
		adda.l		a1,a0
Line2_ok0:

Line2_lp_ok:	or.w		d3,(a0)
		ror.w		#1,d3
		bcc.s		Line2_ok1
		addq.l		#2,a0
Line2_ok1:
		dbf		d7,Line_lp
		rts

;		Ligne verticale.
Line_v:		bra.s		Line_v_lp_ok
Line_v_lp:	adda.l		a1,a0
Line_v_lp_ok:	or.w		d3,(a0)
		dbf		d7,Line_v_lp_ok
		rts

;		Ligne horizontale.
Line_h:		moveq		#$f,d7
; Masque de d‚but.
		moveq		#-1,d0
		move.w		d4,d2
		and.w		d7,d2
		lsr.w		d2,d0

; Masque de fin.
		moveq		#-1,d1
		add.w		d4,d6
		move.w		d6,d2
		and.w		d7,d2
		lsl.w		d2,d1
		not.w		d1

		add.w		d7,d4 ; Arrondi … 16 sup‚rieur.
		lsr.w		#4,d4
		lsr.w		#4,d6
		sub.w		d4,d6
		ble.s		Line_h2

		or.w		d0,(a0)+
		moveq		#-1,d0
		bra.s		Line_h_lp_ok
Line_h_lp:	move.w		d0,(a0)+
Line_h_lp_ok:	dbf		d6,Line_h_lp
		or.w		d1,(a0)
		rts
Line_h2:	and.w		d1,d0
		or.w		d0,(a0)
		rts

;----------------------------------------------------------------
;		Clipping d'une ligne par l'algorithme de Sutherland et Cohen.
;----------------------------------------------------------------
Ln_clip:	movem.w		(ln_pb.x1.w,a6),d0-d3
		cmp.w		d0,d2 ; Si x1=x2,
		beq		Ln_clip_v ; la ligne est verticale.
		cmp.w		d1,d3 ; Si y1=y2,
		beq		Ln_clip_h ; la ligne est horizontale.

		moveq		#0,d6
		move.w		d2,d6
		sub.w		d0,d6
		moveq		#0,d7
		move.w		d1,d7
		sub.w		d3,d7
		swap		d7
		divs.l		d6,d7

		bsr		Ln_clip_cohen
		tst.l		d6
		beq		Ln_clip_ok
		bmi		Ln_clip_out
		moveq		#0,d5
		move.w		(ln_pb.clip_xmin.w,a6),d5
		sub.w		d0,d5
		muls.l		d7,d5
		swap		d5
		add.w		d1,d5
		move.w		(ln_pb.clip_xmin.w,a6),d0
		move.w		d5,d1

		bsr		Ln_clip_cohen
		tst.l		d6
		beq.s		Ln_clip_ok
		bmi.s		Ln_clip_out
		moveq		#0,d5
		move.w		(ln_pb.clip_xmax.w,a6),d5
		sub.w		d0,d5
		muls.l		d7,d5
		swap		d5
		add.w		d1,d5
		move.w		(ln_pb.clip_xmax.w,a6),d0
		move.w		d5,d1

		bsr		Ln_clip_cohen
		tst.l		d6
		beq.s		Ln_clip_ok
		bmi.s		Ln_clip_out
		moveq		#0,d5
		move.w		(ln_pb.clip_ymin.w,a6),d5
		sub.w		d1,d5
		swap		d5
		divs.l		d7,d5
		add.w		d0,d5
		move.w		d5,d0
		move.w		(ln_pb.clip_ymin.w,a6),d1

		bsr		Ln_clip_cohen
		tst.l		d6
		beq.s		Ln_clip_ok
		bmi.s		Ln_clip_out
		moveq		#0,d5
		move.w		(ln_pb.clip_ymax.w,a6),d5
		sub.w		d1,d5
		swap		d5
		divs.l		d7,d5
		add.w		d0,d5
		move.w		d5,d0
		move.w		(ln_pb.clip_ymax.w,a6),d1

Ln_clip_ok:	move.w		d0,(ln_pb.x1.w,a6)
		move.w		d1,(ln_pb.y1.w,a6)
		move.w		d2,(ln_pb.x2.w,a6)
		move.w		d3,(ln_pb.y2.w,a6)
		moveq		#0,d0 ; OK.
		rts
; La ligne est invisible.
Ln_clip_out:	moveq		#-1,d0
		rts

;		D‚termination de la visibilit‚ d'un segment.
Ln_clip_cohen:
; Code extr‚mit‚ du premier sommet.
		moveq		#0,d4
		cmp.w		(ln_pb.clip_xmin.w,a6),d0 ; Si x1<clip_xmin
		bge.s		Ln_clip_cohen_ok00
		bset		#0,d4
		bra.s		Ln_clip_cohen_ok01
Ln_clip_cohen_ok00: cmp.w	(ln_pb.clip_xmax.w,a6),d0 ; Si x1>clip_xmax
		ble.s		Ln_clip_cohen_ok01
		bset		#1,d4
Ln_clip_cohen_ok01: cmp.w	(ln_pb.clip_ymin.w,a6),d1 ; Si y1>clip_ymin
		ble.s		Ln_clip_cohen_ok02
		bset		#2,d4
		bra.s		Ln_clip_cohen_ok03
Ln_clip_cohen_ok02: cmp.w	(ln_pb.clip_ymax.w,a6),d1 ; Si y1>clip_ymax
		ble.s		Ln_clip_cohen_ok03
		bset		#3,d4
Ln_clip_cohen_ok03:

; Code extr‚mit‚ du deuxiŠme sommet.
		moveq		#0,d5
		cmp.w		(ln_pb.clip_xmin.w,a6),d2 ; Si x1<clip_xmin
		bge.s		Ln_clip_cohen_ok10
		bset		#0,d5
		bra.s		Ln_clip_cohen_ok11
Ln_clip_cohen_ok10: cmp.w	(ln_pb.clip_xmax.w,a6),d2; Si x1>clip_xmax
		ble.s		Ln_clip_cohen_ok11
		bset		#1,d5
Ln_clip_cohen_ok11: cmp.w	(ln_pb.clip_ymin.w,a6),d3 ; Si y1>clip_ymin
		ble.s		Ln_clip_cohen_ok12
		bset		#2,d5
		bra.s		Ln_clip_cohen_ok13
Ln_clip_cohen_ok12: cmp.w	(ln_pb.clip_ymax.w,a6),d3 ; Si y1>clip_ymax
		ble.s		Ln_clip_cohen_ok13
		bset		#3,d5
Ln_clip_cohen_ok13:

; .
		move.b		d4,d6
		or.b		d5,d6
		beq.s		Ln_clip_cohen_vis

		and.b		d4,d5
		bne.s		Ln_clip_cohen_unvis

; Si le premier sommet est ext‚rieur, on permute.
		tst.b		d4
		beq.s		Ln_clip_cohen_ok2
		exg		d0,d2
		exg		d1,d3
Ln_clip_cohen_ok2:

		moveq		#1,d6
		rts

Ln_clip_cohen_vis: moveq	#0,d6 ; Segment visible.
		rts
Ln_clip_cohen_unvis: moveq	#-1,d6 ; Segment invisible.
		rts

;		Clipping d'une ligne verticale.
Ln_clip_v:
; V‚rifie si l'abscisse de la ligne est inclus dans la zone de clipping.
		cmp2.w		(ln_pb.clip_xmin.w,a6),d0 ; Compare les limites.
		bcs		Ln_clip_out

		move.w		(ln_pb.clip_ymin.w,a6),d4 ; clip_ymin.
		move.w		(ln_pb.clip_ymax.w,a6),d5 ; clip_ymax.

; y1 doit etre inf‚rieur … y2.
		moveq		#0,d7
		cmp.w		d1,d3 ; Si y1>y2,
		bge.s		Ln_clip_v_ok0
		exg		d1,d3 ; alors EXG y1,y2.
		moveq		#1,d7
Ln_clip_v_ok0:

; V‚rifie si la ligne est au moins partielement a l'int‚rieur de la zone de
;clipping.
		cmp.w		d5,d1 ; Si y1>clip_ymax,
		bgt		Ln_clip_out ; alors invisible.
		cmp.w		d4,d3 ; Si y2<clip_ymin,
		blt		Ln_clip_out

; Calcule les nouvelles coordonn‚es.
		cmp.w		d4,d1 ; Si y1<clip_ymin,
		bge.s		Ln_clip_v_ok1
		move.w		d4,d1 ; alors y1=clip_ymin.
Ln_clip_v_ok1:	cmp.w		d5,d3 ; Si y2>clip_ymax
		ble.s		Ln_clip_v_ok2
		move.w		d5,d3 ; alors y2=clip_ymax
Ln_clip_v_ok2:

; Sauve les nouvelles coordonn‚es.
		tst.l		d7
		beq.s		Ln_clip_v_ok3
		exg		d1,d3
Ln_clip_v_ok3:	move.w		d1,(ln_pb.y1.w,a6)
		move.w		d3,(ln_pb.y2.w,a6)
		moveq		#0,d0 ; OK.
		rts


;		Clipping d'une ligne horizontale.
Ln_clip_h:
; V‚rifie si l'ordonn‚e de la ligne est inclus dans la zone de clipping.
		cmp2.w		(ln_pb.clip_ymin.w,a6),d1 ; Compare les limites.
		bcs		Ln_clip_out

		move.w		(ln_pb.clip_xmin.w,a6),d4 ; clip_xmin.
		move.w		(ln_pb.clip_xmax.w,a6),d5 ; clip_xmax.

; x1 doit etre inf‚rieur … x2.
		moveq		#0,d7
		cmp.w		d0,d2 ; Si x1>x2,
		bge.s		Ln_clip_h_ok0
		exg		d0,d2 ; alors EXG x1,x2.
		moveq		#1,d7
Ln_clip_h_ok0:

; V‚rifie si la ligne est au moins partielement a l'int‚rieur de la zone de
;clipping.
		cmp.w		d5,d0 ; Si x1>clip_xmax,
		bgt		Ln_clip_out ; alors invisible.
		cmp.w		d4,d2 ; Si x2<clip_xmin,
		blt		Ln_clip_out

; Calcule les nouvelles coordonn‚es.
		cmp.w		d4,d0 ; Si x1<clip_xmin,
		bge.s		Ln_clip_h_ok1
		move.w		d4,d0 ; alors x1=clip_xmin.
Ln_clip_h_ok1:	cmp.w		d5,d2 ; Si x2>clip_xmax
		ble.s		Ln_clip_h_ok2
		move.w		d5,d2 ; alors x2=clip_xmax
Ln_clip_h_ok2:

; Sauve les nouvelles coordonn‚es.
		tst.l		d7
		beq.s		Ln_clip_h_ok3
		exg		d0,d2
Ln_clip_h_ok3:	move.w		d0,(ln_pb.x1.w,a6)
		move.w		d2,(ln_pb.x2.w,a6)
		moveq		#0,d0 ; OK.
		rts

;****************************************************************


;****************************************************************
;		Traitements des sons.
;****************************************************************
;----------------------------------------------------------------
;		Conversion d'un sample 8 bits mono en 16 bits stereo.
;----------------------------------------------------------------
Cv_8m_2_16s:	movea.l		(cv_spl_pb.src_addr.w,a6),a0
		movea.l		(cv_spl_pb.dst_addr.w,a6),a1
		move.l		(cv_spl_pb.lgt.w,a6),d7

		move.l		d7,d6
		lsr.l		#6,d7 ; /64.
		andi.l		#$3f,d6 ; Modulo 64.
		neg.l		d6
		jmp		(Cv_8m_2_16s_lp_ok.w,pc,d6.l*8)
Cv_8m_2_16s_lp:	rept		64
		move.b		(a0)+,d0
		lsl.w		#8,d0
		move.w		d0,(a1)+
		move.w		d0,(a1)+
		endr
Cv_8m_2_16s_lp_ok: subq.l	#1,d7
		bcc		Cv_8m_2_16s_lp
		rts

;----------------------------------------------------------------
;		Conversion d'un sample 8 bits stereo en 16 bits stereo.
;----------------------------------------------------------------
Cv_8s_2_16s:	movea.l		(cv_spl_pb.src_addr.w,a6),a0
		movea.l		(cv_spl_pb.dst_addr.w,a6),a1
		move.l		(cv_spl_pb.lgt.w,a6),d7

		move.l		d7,d6
		lsr.l		#6,d7 ; /64.
		andi.w		#$3f,d6 ; Modulo 64.
		muls.w		#-12,d6 ; *-6.
		jmp		(Cv_8s_2_16s_lp_ok.w,pc,d6.l)
Cv_8s_2_16s_lp:	rept		64
		move.b		(a0)+,d0
		lsl.w		#8,d0
		move.w		d0,(a1)+
		move.b		(a0)+,d0
		lsl.w		#8,d0
		move.w		d0,(a1)+
		endr
Cv_8s_2_16s_lp_ok: subq.l	#1,d7
		bcc		Cv_8s_2_16s_lp
		rts

;----------------------------------------------------------------
;		Conversion d'un sample 8 bits mono en 16 bits stereo.
;----------------------------------------------------------------
Cv_16m_2_16s:	movea.l		(cv_spl_pb.src_addr.w,a6),a0
		movea.l		(cv_spl_pb.dst_addr.w,a6),a1
		move.l		(cv_spl_pb.lgt.w,a6),d7

		move.l		d7,d6
		lsr.l		#6,d7 ; /64.
		andi.w		#$3f,d6 ; Modulo 64.
		muls.w		#-6,d6
		jmp		(Cv_16m_2_16s_lp_ok.w,pc,d6.l)
Cv_16m_2_16s_lp: rept		64
		move.w		(a0)+,d0
		move.w		d0,(a1)+
		move.w		d0,(a1)+
		endr
Cv_16m_2_16s_lp_ok: subq.l	#1,d7
		bcc		Cv_16m_2_16s_lp
		rts
;****************************************************************


;****************************************************************
;		D‚finition des structures.
;****************************************************************
fft_point_nb	equ		1024

;		Calcul de la FFT.
fft_pb:		rsstruct
signal_addr:	rs.l		1 ; Adresse du tableau du signal r‚el de 1024W (L).
spec_addr:	rs.l		1 ; Adresse du tableau du spectre de 1024L (L).
window_addr:	rs.l		1 ; Adresse du tableau de la fenetre de 1024L (L).
fg:		rs.w		1 ; Flag indiquent si l'on doit utiliser la fenetre (W).
		rsend

;		Affichage du signal.
signal_pb:	rsstruct
scr_addr:	rs.l		1 ; Adresse de l'‚cran(L).
signal_addr:	rs.l		1 ; Adresse du signal(L).
l_limit:	rs.l		1 ; Position de la limite gauche(L).
r_limit:	rs.l		1 ; Position de la limite droite(L).
disp_x:		rs.w		1 ; Position X sur l'‚cran(W).
disp_y:		rs.w		1 ; Position Y sur l'‚cran(W).
disp_w:		rs.w		1 ; Largeur de l'affichage(W).
zoom_v:		rs.w		1 ; Zoom vertical(W): puissance de 2(1->/2, 2->/4, ...).
accuracy:	rs.w		1 ; Pr‚cision de l'affichage(W).
		rsend

;		Affichage du spectre.
spec_pb:	rsstruct
scr_addr:	rs.l		1 ; Adresse de l'‚cran(L).
spec_addr:	rs.l		1 ; Adresse du spectre de 1024L (L).
disp_x:		rs.w		1 ; Position X sur l'‚cran(W).
disp_y:		rs.w		1 ; Position Y sur l'‚cran(W).
disp_h:		rs.w		1 ; Hauteur de l'affichage(W).
zoom:		rs.w		1 ; Zoom du spectre(W).
		rsend

;		Routine de CLS.
cls_pb:		rsstruct
scr_addr:	rs.l		1 ; Adresse de l'‚cran(L).
col:		rs.l		1 ; Motif(L).
		rsend

;		Affichage d'une ligne.
ln_pb:		rsstruct
scr_addr:	rs.l		1 ; Adresse de l'‚cran(L).
x1:		rs.w		1 ; x1(W).
y1:		rs.w		1 ; y1(W).
x2:		rs.w		1 ; x2(W).
y2:		rs.w		1 ; y2(W).
col:		rs.w		1 ; Couleur(W): 0=efface, 1=affiche, 2=inverse.
clip_xmin:	rs.w		1 ; Clipping xmin(W).
clip_xmax:	rs.w		1 ; Clipping xmax(W).
clip_ymin:	rs.w		1 ; Clipping ymin(W).
clip_ymax:	rs.w		1 ; Clipping ymax(W).
		rsend

;		Conversion des samples.
cv_spl_pb:	rsstruct
src_addr:	rs.l		1 ; Adresse source(L).
dst_addr:	rs.l		1 ; Adresse destination(L).
lgt:		rs.l		1 ; Longueur(L).
		rsend
;****************************************************************

