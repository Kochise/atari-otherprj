;****************************************************************
;		Structures des donn‚es.
;****************************************************************
;----------------------------------------------------------------
;		Description du bloc de paramŠtres.
;----------------------------------------------------------------
;		Structure du bloc de paramŠtres.
Screen_Pb:	rsstruct
bpp:		rs.w		1 ; Nombre de bits par pixel (1,2,4,8,16,24,32)(W).
physical_w:	rs.w		1 ; Largeur physique(W).
physical_h:	rs.w		1 ; Hauteur physique(W).
virtual_w:	rs.w		1 ; Largeur virtuelle(W).
virtual_h:	rs.w		1 ; Hauyeur virtuelle(W).
backbuffer_nb:	rs.w		1 ; Nombre d'ecrans logiques(W).
		rsend

;----------------------------------------------------------------
;		Structure du bloc d'information.
;----------------------------------------------------------------
Screen_Info:	rsstruct
bpp:		rs.w		1 ; Nombre de bits par pixel (1,2,4,8,16,24,32).
physical_w:	rs.w		1 ; Largeur physique de l'‚cran(W).
physical_h:	rs.w		1 ; Hauteur physique de l'‚cran(W).
virtual_w:	rs.w		1 ; Largeur virtuelle de l'‚cran(W).
virtual_h:	rs.w		1 ; Hauteur virtuelle de l'‚cran(W).
backbuffer_nb:	rs.w		1 ; Nombre d'‚crans(W).

;		M‚moire.
scr_addr:	rs.l		1 ; Adresse de d‚but des ‚crans(L).
physic_scr_addr: rs.l		1 ; Adresse de l'‚cran physique (L).
logic_scr_addr:	rs.l		1 ; Adresse de l'‚cran logique (L).
ln_tbl_addr:	rs.l		1 ; Adresse d'une table d'offsets de lignes(L).
ln_size:	rs.l		1 ; Taille d'une ligne en octets(L).
scr_size:	rs.l		1 ; Taille de l'‚cran en octets(L).

;		Scrolling.
scroll_addr:	rs.l		1 ; Offset de l'‚cran physique sur l'‚cran virtuel(L).
scroll_x:	rs.w		1 ; Position X de l'‚cran physique sur l'‚cran virtuel(W).
scroll_y:	rs.w		1 ; Position Y(W).
scroll_xmin:	rs.w		1 ; Position X minimale(W).
scroll_xmax:	rs.w		1 ; Position X maximale(W).
scroll_ymin:	rs.w		1 ; Position Y minimale(W).
scroll_ymax:	rs.w		1 ; Position Y maximale(W).

monitor:	rs.w		1 ; Type de moniteur: 0=mono; 1=couleur; 2=VGA; 3=TV(W).
videl_frq:	rs.l		1 ; Fr‚quence du Videl(L).
horiz_frq:	rs.l		1 ; Fr‚quence horizontale(L).
vert_frq:	rs.l		1 ; Fr‚quence verticale(L).

sizeof:	;	rs.l		1
		rsend
;****************************************************************


;****************************************************************
;		Video hardware.
;****************************************************************
;----------------------------------------------------------------
;		Adresses.
;----------------------------------------------------------------
VIDEO_VBASEHI	equ		$ff8201 ; Video Base Address High(B).
VIDEO_VBASEMID	equ		$ff8203 ; Video Base Address Mid(B).
VIDEO_VBASELO	equ		$ff820d ; Video Base Address Low(B).
VIDEO_VCOUNTHI	equ		$ff8205 ; Video Counter Address High(B).
VIDEO_VCOUNTMID	equ		$ff8207 ; Video Counter Address Mid(B).
VIDEO_VCOUNTLO	equ		$ff8209 ; Video Counter Address Low(B).
VIDEO_SYNCMODE	equ		$ff820a ; Video Shifter Sync Mode(B).
VIDEO_EXTRALINEW equ		$ff820e ; Video Extra Line Width(W,9 bits): number of extra words per scanline.
VIDEL_LINEW	equ		$ff8210 ; Video Line Width(W,10 bits): width of a physic scanline in words.
VIDEO_ST_PAL	equ		$ff8240 ; ST/E Compatible Palette(16W,12 bits).
VIDEL_SHIFTMODE	equ		$ff8260 ; ST Video Shifter Mode(B,2 bits).
VIDEL_HSCROLL	equ		$ff8264 ; Horizontal Scroll Register(B,4 bits).
VIDEL_SPSHIFT	equ		$ff8266 ; SPSHIFT Control Register(W,11 bits).
VIDEL_HHC	equ		$ff8280 ; Horizontal Hold Counter(W,9 bits).
VIDEL_HHT	equ		$ff8282 ; Horizontal Hold Timer(W,9 bits).
VIDEL_HBB	equ		$ff8284 ; Horizontal Border Begin(W,9 bits).
VIDEL_HBE	equ		$ff8286 ; Horizontal Border End(W,9 bits).
VIDEL_HDB	equ		$ff8288 ; Horizontal Display Begin(W,10 bits).
VIDEL_HDE	equ		$ff828a ; Horizontal Display End(W,9 bits).
VIDEL_HSS	equ		$ff828c ; Horizontal Synchro Start(W,9 bits).
VIDEL_HFS	equ		$ff828e ; Horizontal Frequency Start?(W,9 bits).
VIDEL_HEE	equ		$ff8290 ; HEE(W,9 bits).
VIDEL_VFC	equ		$ff82a0 ; Vertical Frequency Counter(W,11 bits).
VIDEL_VFT	equ		$ff82a2 ; Vertical Frequency Timer(W,11 bits).
VIDEL_VBB	equ		$ff82a4 ; Vertical Border Begin(W,11 bits).
VIDEL_VBE	equ		$ff82a6 ; Vertical Border End(W,11 bits).
VIDEL_VDB	equ		$ff82a8 ; Vertical Display Begin(W,11 bits).
VIDEL_VDE	equ		$ff82aa ; Vertical Display End(W,11 bits).
VIDEL_VSS	equ		$ff82ac ; Vertical Synchro Start(W,11 bits).
VIDEL_RCO	equ		$ff82c0 ; RCO(W,8 bits).
VIDEL_VCO	equ		$ff82c2 ; Video Control(W,4 bits).
VIDEL_PAL	equ		$ff9800 ; Palette Falcon 030.

;----------------------------------------------------------------
;		Bits des registres.
;----------------------------------------------------------------
;		SPSHIFT.
vid_spshift_256	equ		4 ; Mode 256 couleurs parmi 262144.
vid_spshift_vsync equ		5 ; Utiliser le signal de synchronisation verticale externe.
vid_spshift_hsync equ		6 ; Utiliser le signal de synchronisation horizontal externe.
vid_spshift_tc16 equ		8 ; Mode True Color 16 bits.
vid_spshift_2	equ		10 ; Mode 2 couleurs parmi 262144.

;		VCO.
vid_vco_ln_dbl	equ		0 ; Active le mode double ligne.
vid_vco_interl	equ		1 ; Active le mode entrelac‚.
vid_vco_2pix	equ		2 ; Mode double pixel.
vid_vco_4pix	equ		3 ; Mode quadruple pixel.

;		RCO.
vid_rco_4pix_fg1 equ		0 ; Doit etre mis … 0 en mode quadruple pixel.
vid_rco_4pix_fg2 equ		1 ; Doit etre mis … 1 en mode quadruple pixel.
vid_rco_freq	equ		2 ; Si mis … 0, le Videl est … 32 Mhz, sinon … 25 Mhz.
vid_rco_interl	equ		3 ; En mode entrelac‚: si mis … 0, les lignes paires/impaires
;s'affichent les unes par dessus les autres. Sinon elles sont d‚cal‚es.
vid_rco_fg3	equ		4 ; Doit etre fixe … 0.
vid_rco_fg4	equ		5 ; Doit etre fixe … 0.
vid_rco_fg5	equ		6 ; Doit etre fixe … 0.
vid_rco_fg6	equ		7 ; Doit etre fixe … 1.
vid_rco_fg7	equ		8 ; Doit etre mis … 0 en mode compatible ST, sinon … 1.
;****************************************************************


;****************************************************************
;		
;****************************************************************
red		equ		2048
green		equ		32
blue		equ		1
red_shift	equ		11
green_shift	equ		5
blue_shift	equ		0
;****************************************************************
