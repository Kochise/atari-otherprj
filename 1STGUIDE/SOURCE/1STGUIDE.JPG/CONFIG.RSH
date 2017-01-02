/* GEM Resource C Source */

static TEDINFO rs_tedinfo[] =
{ "21",
  "Ger\204tenummer: __",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 17,
  " ABBRUCH D. AUSGABE MIT L.SHIFT-R.SHIFT ",
  "",
  "",
  SMALL, 0, TE_LEFT , 0x1180, 0, 0, 41, 1,
  "11",
  "Linker Rand: __ Zeichen",
  "99",
  IBM  , 0, TE_CNTR , 0x11A3, 0, -1, 3, 24,
  "QUALIT\216T:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 10, 1,
  "X-SKALIERUNG:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 14, 1,
  "Y-SKALIERUNG:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 14, 1,
  "HORIZONTALE AUSRICHTUNG:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 25, 1,
  "VERTIKALE AUSRICHTUNG:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 23, 1,
  "STANDARD-AUSGABE:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 18, 1,
  "FULLER-STANDARD:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 17, 1,
  "TEXT-FONTEINSTELLUNG:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 22, 1,
  "EXTERNE CONTROL-P-AUSGABE:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 27, 1,
  "DOPPELKLICK-STANDARD:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 22, 1,
  "CALCLOCK",
  " ________ ",
  "xxxxxxxx",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 9, 11,
  "IMAGE-DITHERING:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 17, 1,
  "FARBMODELL:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 12, 1,
  "FARBSTUFEN:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 12, 1,
  "DITHERMETHODE:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 15, 1,
  "(FALCON-DSP)",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 13, 1,
  0,
  "__________________________________________________________",
  "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 59, 59,
  "RES: XX BIT",
  "",
  "",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 12, 1,
  "XXXX * XXXX",
  "",
  "",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 12, 1,
  "MEM: XXXXXX K",
  "",
  "",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 14, 1,
  "TOT: XXXXXX K",
  "",
  "",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 14, 1
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 TREE1 ****************************************************/
	 0,	   1,	    99, G_BOX	  |0x8900,   /* Object 0  */
  NONE, OUTLINED, 0x00011152L,
  0x0000, 0x0000, 0x0042, 0x0014,
  OUTPAR  ,	   2,	     2, G_BOX	  ,   /* Object 1  */
  NONE, OUTLINED, 0x00001100L,
  0x0002, 0x0001, 0x003E, 0x0001,
	 1,	  -1,	    -1, G_BUTTON  ,   /* Object 2  */
  NONE, SHADOWED, "1STGUIDE: Ausgabe-Konfiguration \275 Digital Design, " __DATE__,
  0x0000, 0x0000, 0x003E, 0x0001,
  ALLPAR  , NUMBER  ,	    36, G_BOX	  ,   /* Object 3 OUTPAR */
  NONE, NORMAL, 0x00FE1152L,
  0x0002, 0x0003, 0x003E, 0x000D,
	 5,	  -1,	    -1, G_FBOXTEXT,   /* Object 4 NUMBER */
  EDITABLE, NORMAL, &rs_tedinfo[0],
  0x0000, 0x0000, 0x001E, 0x0001,
  TOALLPAR,	  -1,	    -1, G_TEXT	  ,   /* Object 5  */
  NONE, NORMAL, &rs_tedinfo[1],
  0x0000, 0x0001, 0x001E, 0x0001,
	 7,	  -1,	    -1, G_BUTTON  |0x0C00,   /* Object 6 TOALLPAR */
  SELECTABLE|EXIT, NORMAL, "Allgemeine Parameter...",
  0x0020, 0x000C, 0x001E, 0x0001,
	16,	   8,	    12, G_BOX	  ,   /* Object 7  */
  NONE, NORMAL, 0x00FF1100L,
  0x0000, 0x0002, 0x001E, 0x0007,
  RAND	  ,	  -1,	    -1, G_STRING  ,   /* Object 8  */
  NONE, NORMAL, "Text",
  0x000D, 0x0000, 0x0004, 0x0001,
  FORMFEED,	  -1,	    -1, G_FBOXTEXT,   /* Object 9 RAND */
  EDITABLE, NORMAL, &rs_tedinfo[2],
  0x0002, 0x0001, 0x001A, 0x0001,
	11,	  -1,	    -1, G_BUTTON  ,   /* Object 10 FORMFEED */
  SELECTABLE, SELECTED, "Seitenvorschub nach Datei",
  0x0002, 0x0003, 0x001A, 0x0001,
	12,	  -1,	    -1, G_TEXT	  ,   /* Object 11  */
  NONE, NORMAL, &rs_tedinfo[3],
  0x0002, 0x0004, 0x001A, 0x0001,
	 7, QNORMAL ,	    15, G_BOX	  ,   /* Object 12  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0005, 0x001A, 0x0001,
	14,	  -1,	    -1, G_BUTTON  ,   /* Object 13 QNORMAL */
  SELECTABLE|RBUTTON, SELECTED, "normal",
  0x0000, 0x0000, 0x0008, 0x0001,
	15,	  -1,	    -1, G_BUTTON  |0x0400,   /* Object 14  */
  SELECTABLE|RBUTTON, NORMAL, "Draft",
  0x0009, 0x0000, 0x0008, 0x0001,
	12,	  -1,	    -1, G_BUTTON  ,   /* Object 15  */
  SELECTABLE|RBUTTON, NORMAL, "Letter",
  0x0012, 0x0000, 0x0008, 0x0001,
	22,	  17,	    18, G_BOX	  ,   /* Object 16  */
  NONE, NORMAL, 0x00FF1100L,
  0x0000, 0x000A, 0x001E, 0x0003,
	18,	  -1,	    -1, G_STRING  ,   /* Object 17  */
  NONE, NORMAL, "Meta",
  0x000D, 0x0000, 0x0004, 0x0001,
	16,	  19,	    21, G_BOX	  ,   /* Object 18  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0001, 0x001A, 0x0001,
  PASSEND ,	  -1,	    -1, G_STRING  ,   /* Object 19  */
  NONE, NORMAL, "Gr\224\341e",
  0x0001, 0x0000, 0x0005, 0x0001,
	21,	  -1,	    -1, G_BUTTON  |0x0700,   /* Object 20 PASSEND */
  SELECTABLE|RBUTTON, NORMAL, "passend",
  0x0007, 0x0000, 0x0009, 0x0001,
	18,	  -1,	    -1, G_BUTTON  |0x0300,   /* Object 21  */
  SELECTABLE|RBUTTON, SELECTED, "Original",
  0x0011, 0x0000, 0x0009, 0x0001,
	36,	  23,	    33, G_BOX	  ,   /* Object 22  */
  NONE, NORMAL, 0x00FF1100L,
  0x0020, 0x0000, 0x001E, 0x0005,
	24,	  -1,	    -1, G_STRING  ,   /* Object 23  */
  NONE, NORMAL, "Image",
  0x000C, 0x0000, 0x0005, 0x0001,
	28, PIXEL   ,	    27, G_BOX	  ,   /* Object 24  */
  NONE, NORMAL, 0x00FF1123L,
  0x0001, 0x0001, 0x001C, 0x0001,
	26,	  -1,	    -1, G_BUTTON  |0x0300,   /* Object 25 PIXEL */
  SELECTABLE|RBUTTON, NORMAL, "Pixel 1:1",
  0x0000, 0x0000, 0x0009, 0x0001,
	27,	  -1,	    -1, G_BUTTON  ,   /* Object 26  */
  SELECTABLE|RBUTTON, NORMAL, "passend",
  0x000A, 0x0000, 0x0008, 0x0001,
	24,	  -1,	    -1, G_BUTTON  ,   /* Object 27  */
  SELECTABLE|RBUTTON, SELECTED, "Original",
  0x0013, 0x0000, 0x0009, 0x0001,
	29,	  -1,	    -1, G_TEXT	  ,   /* Object 28  */
  NONE, NORMAL, &rs_tedinfo[4],
  0x0001, 0x0002, 0x000D, 0x0001,
	30,	  -1,	    -1, G_TEXT	  ,   /* Object 29  */
  NONE, NORMAL, &rs_tedinfo[5],
  0x0010, 0x0002, 0x000D, 0x0001,
	33, XBRUCH  ,	    32, G_BOX	  ,   /* Object 30  */
  NONE, NORMAL, 0x00FF1123L,
  0x0001, 0x0003, 0x000D, 0x0001,
	32,	  -1,	    -1, G_BUTTON  ,   /* Object 31 XBRUCH */
  SELECTABLE|RBUTTON, NORMAL, "Bruch",
  0x0000, 0x0000, 0x0006, 0x0001,
	30,	  -1,	    -1, G_BUTTON  ,   /* Object 32  */
  SELECTABLE|RBUTTON, SELECTED, "Ganz",
  0x0007, 0x0000, 0x0006, 0x0001,
	22, YBRUCH  ,	    35, G_BOX	  ,   /* Object 33  */
  NONE, NORMAL, 0x00FF1123L,
  0x0010, 0x0003, 0x000D, 0x0001,
	35,	  -1,	    -1, G_BUTTON  |0x0400,   /* Object 34 YBRUCH */
  SELECTABLE|RBUTTON, NORMAL, "Bruch",
  0x0000, 0x0000, 0x0006, 0x0001,
	33,	  -1,	    -1, G_BUTTON  ,   /* Object 35  */
  SELECTABLE|RBUTTON, SELECTED, "Ganz",
  0x0007, 0x0000, 0x0006, 0x0001,
  OUTPAR  ,	  37,	    43, G_BOX	  ,   /* Object 36  */
  NONE, NORMAL, 0x00FF1100L,
  0x0020, 0x0006, 0x001E, 0x0005,
	38,	  -1,	    -1, G_TEXT	  ,   /* Object 37  */
  NONE, NORMAL, &rs_tedinfo[6],
  0x0002, 0x0000, 0x001A, 0x0001,
	42, LINKS   ,	    41, G_BOX	  ,   /* Object 38  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0001, 0x001A, 0x0001,
	40,	  -1,	    -1, G_BUTTON  |0x0400,   /* Object 39 LINKS */
  SELECTABLE|RBUTTON, NORMAL, "Links",
  0x0000, 0x0000, 0x0008, 0x0001,
	41,	  -1,	    -1, G_BUTTON  ,   /* Object 40  */
  SELECTABLE|RBUTTON, SELECTED, "Mitte",
  0x0009, 0x0000, 0x0008, 0x0001,
	38,	  -1,	    -1, G_BUTTON  |0x0400,   /* Object 41  */
  SELECTABLE|RBUTTON, NORMAL, "Rechts",
  0x0012, 0x0000, 0x0008, 0x0001,
	43,	  -1,	    -1, G_TEXT	  ,   /* Object 42  */
  NONE, NORMAL, &rs_tedinfo[7],
  0x0002, 0x0002, 0x001A, 0x0001,
	36, OBEN    ,	    46, G_BOX	  ,   /* Object 43  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0003, 0x001A, 0x0001,
	45,	  -1,	    -1, G_BUTTON  ,   /* Object 44 OBEN */
  SELECTABLE|RBUTTON, SELECTED, "Oben",
  0x0000, 0x0000, 0x0008, 0x0001,
	46,	  -1,	    -1, G_BUTTON  ,   /* Object 45  */
  SELECTABLE|RBUTTON, NORMAL, "Mitte",
  0x0009, 0x0000, 0x0008, 0x0001,
	43,	  -1,	    -1, G_BUTTON  ,   /* Object 46  */
  SELECTABLE|RBUTTON, NORMAL, "Unten",
  0x0012, 0x0000, 0x0008, 0x0001,
	91,	  48, IFILE   , G_BOX	  ,   /* Object 47 ALLPAR */
  NONE, NORMAL, 0x00FE1100L,
  0x0002, 0x0003, 0x003E, 0x000D,
	49,	  -1,	    -1, G_TEXT	  ,   /* Object 48  */
  NONE, NORMAL, &rs_tedinfo[8],
  0x0002, 0x0000, 0x0011, 0x0001,
	50,	  -1,	    -1, G_TEXT	  ,   /* Object 49  */
  NONE, NORMAL, &rs_tedinfo[9],
  0x0015, 0x0000, 0x0013, 0x0001,
	51,	  -1,	    -1, G_TEXT	  ,   /* Object 50  */
  NONE, NORMAL, &rs_tedinfo[10],
  0x002A, 0x0000, 0x0012, 0x0001,
	54, OUTWIN  ,	    53, G_BOX	  ,   /* Object 51  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0001, 0x0011, 0x0001,
	53,	  -1,	    -1, G_BUTTON  ,   /* Object 52 OUTWIN */
  SELECTABLE|RBUTTON, SELECTED, "Fenster",
  0x0000, 0x0000, 0x0008, 0x0001,
	51,	  -1,	    -1, G_BUTTON  ,   /* Object 53  */
  SELECTABLE|RBUTTON, NORMAL, "Ger\204t",
  0x000A, 0x0000, 0x0007, 0x0001,
	57, FULLDEF ,	    56, G_BOX	  ,   /* Object 54  */
  NONE, NORMAL, 0x00FF1123L,
  0x0015, 0x0001, 0x0013, 0x0001,
	56,	  -1,	    -1, G_BUTTON  ,   /* Object 55 FULLDEF */
  SELECTABLE|RBUTTON, NORMAL, "Desktop",
  0x0000, 0x0000, 0x0008, 0x0001,
	54,	  -1,	    -1, G_BUTTON  ,   /* Object 56  */
  SELECTABLE|RBUTTON, SELECTED, "Dokument",
  0x000A, 0x0000, 0x0009, 0x0001,
	60, FONTDEF ,	    59, G_BOX	  ,   /* Object 57  */
  NONE, NORMAL, 0x00FF1123L,
  0x002A, 0x0001, 0x0012, 0x0001,
	59,	  -1,	    -1, G_BUTTON  ,   /* Object 58 FONTDEF */
  SELECTABLE|RBUTTON, SELECTED, "Normal",
  0x0000, 0x0000, 0x0008, 0x0001,
	57,	  -1,	    -1, G_BUTTON  ,   /* Object 59  */
  SELECTABLE|RBUTTON, NORMAL, "VA_FILE",
  0x000A, 0x0000, 0x0008, 0x0001,
	61,	  -1,	    -1, G_TEXT	  ,   /* Object 60  */
  NONE, NORMAL, &rs_tedinfo[11],
  0x0002, 0x0002, 0x0016, 0x0001,
	62,	  -1,	    -1, G_TEXT	  ,   /* Object 61  */
  NONE, NORMAL, &rs_tedinfo[12],
  0x001A, 0x0002, 0x0022, 0x0001,
	65, EXTSPOOL, OUTEXT  , G_BOX	  ,   /* Object 62  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0003, 0x0016, 0x0001,
  OUTEXT  ,	  -1,	    -1, G_FTEXT   ,   /* Object 63 EXTSPOOL */
  EDITABLE, NORMAL, &rs_tedinfo[13],
  0x0000, 0x0000, 0x000A, 0x0001,
	62,	  -1,	    -1, G_BUTTON  |0x0200,   /* Object 64 OUTEXT */
  SELECTABLE, NORMAL, "benutzen",
  0x000C, 0x0000, 0x000A, 0x0001,
	68, DOVER   ,	    67, G_BOX	  ,   /* Object 65  */
  NONE, NORMAL, 0x00FF1123L,
  0x001A, 0x0003, 0x0022, 0x0001,
	67,	  -1,	    -1, G_BUTTON  |0x0B00,   /* Object 66 DOVER */
  SELECTABLE|RBUTTON, SELECTED, "Neues Fenster",
  0x0000, 0x0000, 0x0010, 0x0001,
	65,	  -1,	    -1, G_BUTTON  |0x0600,   /* Object 67  */
  SELECTABLE|RBUTTON, NORMAL, "\232berschreiben",
  0x0012, 0x0000, 0x0010, 0x0001,
	69,	  -1,	    -1, G_TEXT	  ,   /* Object 68  */
  NONE, NORMAL, &rs_tedinfo[14],
  0x0002, 0x0004, 0x001B, 0x0001,
	70,	  -1,	    -1, G_TEXT	  ,   /* Object 69  */
  NONE, NORMAL, &rs_tedinfo[15],
  0x001F, 0x0004, 0x000C, 0x0001,
	71,	  -1,	    -1, G_TEXT	  ,   /* Object 70  */
  NONE, NORMAL, &rs_tedinfo[16],
  0x002E, 0x0004, 0x000E, 0x0001,
	75, USEDITH ,	    74, G_BOX	  ,   /* Object 71  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0005, 0x001B, 0x0001,
	73,	  -1,	    -1, G_BUTTON  |0x0400,   /* Object 72 USEDITH */
  SELECTABLE|RBUTTON, NORMAL, "Niemals",
  0x0000, 0x0000, 0x0008, 0x0001,
	74,	  -1,	    -1, G_BUTTON  |0x0500,   /* Object 73  */
  SELECTABLE|RBUTTON, SELECTED, "Bedarf",
  0x000A, 0x0000, 0x0008, 0x0001,
	71,	  -1,	    -1, G_BUTTON  ,   /* Object 74  */
  SELECTABLE|RBUTTON, NORMAL, "Immer",
  0x0014, 0x0000, 0x0007, 0x0001,
	78, DITHMOD ,	    77, G_BOX	  ,   /* Object 75  */
  NONE, NORMAL, 0x00FF1123L,
  0x001F, 0x0005, 0x000C, 0x0001,
	77,	  -1,	    -1, G_BUTTON  ,   /* Object 76 DITHMOD */
  SELECTABLE|RBUTTON, NORMAL, "Grau",
  0x0000, 0x0000, 0x0005, 0x0001,
	75,	  -1,	    -1, G_BUTTON  ,   /* Object 77  */
  SELECTABLE|RBUTTON, SELECTED, "RGB",
  0x0007, 0x0000, 0x0005, 0x0001,
	81, DITHCOL ,	    80, G_BOX	  ,   /* Object 78  */
  NONE, NORMAL, 0x00FF1123L,
  0x002E, 0x0005, 0x000E, 0x0001,
	80,	  -1,	    -1, G_BUTTON  ,   /* Object 79 DITHCOL */
  SELECTABLE|RBUTTON, NORMAL, "2/8",
  0x0000, 0x0000, 0x0006, 0x0001,
	78,	  -1,	    -1, G_BUTTON  ,   /* Object 80  */
  SELECTABLE|RBUTTON, SELECTED, "6/216",
  0x0008, 0x0000, 0x0006, 0x0001,
	82,	  -1,	    -1, G_TEXT	  ,   /* Object 81  */
  NONE, NORMAL, &rs_tedinfo[17],
  0x0002, 0x0006, 0x0022, 0x0001,
	83,	  -1,	    -1, G_TEXT	  ,   /* Object 82  */
  NONE, NORMAL, &rs_tedinfo[18],
  0x0026, 0x0006, 0x0016, 0x0001,
  USEDSP  , DITHTYP ,	    85, G_BOX	  ,   /* Object 83  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0007, 0x0022, 0x0001,
	85,	  -1,	    -1, G_BUTTON  ,   /* Object 84 DITHTYP */
  SELECTABLE|RBUTTON, NORMAL, "Ordered Dither",
  0x0000, 0x0000, 0x0010, 0x0001,
	83,	  -1,	    -1, G_BUTTON  ,   /* Object 85  */
  SELECTABLE|RBUTTON, SELECTED, "Floyd-Steinberg",
  0x0012, 0x0000, 0x0010, 0x0001,
  IFSEL   ,	  -1,	    -1, G_BUTTON  ,   /* Object 86 USEDSP */
  SELECTABLE, SELECTED, "JPEG-Treiber benutzen",
  0x0026, 0x0007, 0x0016, 0x0001,
  TOSNDPAR,	  -1,	    -1, G_BUTTON  ,   /* Object 87 IFSEL */
  SELECTABLE|EXIT, OUTLINED|SHADOWED, "Indexdatei:",
  0x0002, 0x0009, 0x000C, 0x0001,
  TOOUTPAR,	  -1,	    -1, G_BUTTON  |0x0800,   /* Object 88 TOSNDPAR */
  SELECTABLE|EXIT, DISABLED, "Sound-Parameter...",
  0x0013, 0x0009, 0x0013, 0x0001,
  IFILE   ,	  -1,	    -1, G_BUTTON  |0x0800,   /* Object 89 TOOUTPAR */
  SELECTABLE|EXIT, NORMAL, "Ger\204te-Parameter...",
  0x0028, 0x0009, 0x0014, 0x0001,
  ALLPAR  ,	  -1,	    -1, G_FBOXTEXT,   /* Object 90 IFILE */
  EDITABLE, OUTLINED, &rs_tedinfo[19],
  0x0002, 0x000B, 0x003A, 0x0001,
  LOAD	  ,	  92,	    92, G_BOX	  ,   /* Object 91  */
  NONE, OUTLINED, 0x00004100L,
  0x0002, 0x0011, 0x000C, 0x0002,
	91, RES1    , RES2    , G_BOX	  ,   /* Object 92  */
  NONE, SHADOWED, 0x00FF4100L,
  0x0000, 0x0000, 0x000C, 0x0002,
  RES2	  ,	  -1,	    -1, G_TEXT	  ,   /* Object 93 RES1 */
  NONE, NORMAL, &rs_tedinfo[20],
  0x0000, 0x0000, 0x000C, 0x0001,
	92,	  -1,	    -1, G_TEXT	  ,   /* Object 94 RES2 */
  NONE, NORMAL, &rs_tedinfo[21],
  0x0000, 0x0001, 0x000C, 0x0001,
  SAVE	  ,	  -1,	    -1, G_BUTTON  |0x0100,   /* Object 95 LOAD */
  SELECTABLE|EXIT, NORMAL, "Laden",
  0x0010, 0x0011, 0x0006, 0x0002,
  OK	  ,	  -1,	    -1, G_BUTTON  |0x0100,   /* Object 96 SAVE */
  SELECTABLE|EXIT, NORMAL, "Sichern",
  0x0017, 0x0011, 0x0008, 0x0002,
  ABBRUCH ,	  -1,	    -1, G_BUTTON  ,   /* Object 97 OK */
  SELECTABLE|DEFAULT|EXIT, NORMAL, "OK",
  0x0021, 0x0011, 0x0006, 0x0002,
	99,	  -1,	    -1, G_BUTTON  ,   /* Object 98 ABBRUCH */
  SELECTABLE|EXIT, NORMAL, "Abbruch",
  0x0028, 0x0011, 0x0008, 0x0002,
	 0,	 100,	   100, G_BOX	  ,   /* Object 99  */
  NONE, OUTLINED, 0x00004100L,
  0x0032, 0x0011, 0x000E, 0x0002,
	99, MEM     , TOT     , G_BOX	  ,   /* Object 100  */
  NONE, SHADOWED, 0x00FF4100L,
  0x0000, 0x0000, 0x000E, 0x0002,
  TOT	  ,	  -1,	    -1, G_TEXT	  ,   /* Object 101 MEM */
  NONE, NORMAL, &rs_tedinfo[22],
  0x0000, 0x0000, 0x000E, 0x0001,
       100,	  -1,	    -1, G_TEXT	  ,   /* Object 102 TOT */
  LASTOB, NORMAL, &rs_tedinfo[23],
  0x0000, 0x0001, 0x000E, 0x0001,
       100,	  -1,	    -1, G_TEXT	  ,   /* Object 102 TOT */
  LASTOB, NORMAL, &rs_tedinfo[23],
  0x0000, 0x0001, 0x000E, 0x0001
};

#define NUM_OBS 103
