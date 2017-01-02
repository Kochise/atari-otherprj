/* GEM Resource C Source */

static TEDINFO rs_tedinfo[] =
{
  "21",
  "Ger\204tenummer: __",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 17,
  " ABBRUCH D. AUSGABE MIT L.SHIFT-R.SHIFT ",
  "",
  "",
  SMALL, 0, TE_LEFT , 0x1180, 0, 0, 41, 1,
  "00",
  "Linker Rand: __ Zeichen",
  "99",
  IBM  , 0, TE_CNTR , 0x11A3, 0, -1, 3, 24,
  "SEITENVORSCHUB NACH DATEI:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 27, 1,
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
  "CALCLOCK",
  " ________ ",
  "xxxxxxxx",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 9, 11,
  "TEXT-FONTEINSTELLUNG:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 22, 1,
  "IMAGE-DITHERFARBEN:",
  "",
  "",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 20, 1,
  0,
  "__________________________________________________________",
  "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 59, 59
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 TREE1 ****************************************************/
	 0,	   1,	    81, G_BOX	  |0x8900,   /* Object 0  */
  NONE, OUTLINED, 0x00011152L,
  0x0000, 0x0000, 0x0042, 0x0014,
  OUTPAR  ,	   2,	     2, G_BOX	  ,   /* Object 1  */
  NONE, OUTLINED, 0x00001100L,
  0x0002, 0x0001, 0x003E, 0x0001,
	 1,	  -1,	    -1, G_BUTTON  ,   /* Object 2  */
  NONE, SHADOWED, "1STGUIDE: Ausgabe-Konfiguration \275 Digital Design, " __DATE__,
  0x0000, 0x0000, 0x003E, 0x0001,
  ALLPAR  , NUMBER  ,	    39, G_BOX	  ,   /* Object 3 OUTPAR */
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
	19,	   8,	    15, G_BOX	  ,   /* Object 7  */
  NONE, NORMAL, 0x00FF1100L,
  0x0000, 0x0002, 0x001E, 0x0007,
  RAND	  ,	  -1,	    -1, G_STRING  ,   /* Object 8  */
  NONE, NORMAL, "Text",
  0x000D, 0x0000, 0x0004, 0x0001,
	10,	  -1,	    -1, G_FBOXTEXT,   /* Object 9 RAND */
  EDITABLE, NORMAL, &rs_tedinfo[2],
  0x0002, 0x0001, 0x001A, 0x0001,
	11,	  -1,	    -1, G_TEXT	  ,   /* Object 10  */
  NONE, NORMAL, &rs_tedinfo[3],
  0x0002, 0x0002, 0x001A, 0x0001,
	14, JA	    ,	    13, G_BOX	  ,   /* Object 11  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0003, 0x001A, 0x0001,
	13,	  -1,	    -1, G_BUTTON  ,   /* Object 12 JA */
  SELECTABLE|RBUTTON, SELECTED, "Ja",
  0x0004, 0x0000, 0x0007, 0x0001,
	11,	  -1,	    -1, G_BUTTON  ,   /* Object 13  */
  SELECTABLE|RBUTTON, NORMAL, "Nein",
  0x000F, 0x0000, 0x0007, 0x0001,
	15,	  -1,	    -1, G_TEXT	  ,   /* Object 14  */
  NONE, NORMAL, &rs_tedinfo[4],
  0x0002, 0x0004, 0x001A, 0x0001,
	 7, QNORMAL ,	    18, G_BOX	  ,   /* Object 15  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0005, 0x001A, 0x0001,
	17,	  -1,	    -1, G_BUTTON  ,   /* Object 16 QNORMAL */
  SELECTABLE|RBUTTON, SELECTED, "normal",
  0x0000, 0x0000, 0x0008, 0x0001,
	18,	  -1,	    -1, G_BUTTON  |0x0400,   /* Object 17  */
  SELECTABLE|RBUTTON, NORMAL, "Draft",
  0x0009, 0x0000, 0x0008, 0x0001,
	15,	  -1,	    -1, G_BUTTON  ,   /* Object 18  */
  SELECTABLE|RBUTTON, NORMAL, "Letter",
  0x0012, 0x0000, 0x0008, 0x0001,
	25,	  20,	    21, G_BOX	  ,   /* Object 19  */
  NONE, NORMAL, 0x00FF1100L,
  0x0000, 0x000A, 0x001E, 0x0003,
	21,	  -1,	    -1, G_STRING  ,   /* Object 20  */
  NONE, NORMAL, "Meta",
  0x000D, 0x0000, 0x0004, 0x0001,
	19,	  22,	    24, G_BOX	  ,   /* Object 21  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0001, 0x001A, 0x0001,
  PASSEND ,	  -1,	    -1, G_STRING  ,   /* Object 22  */
  NONE, NORMAL, "Gr\224\341e",
  0x0001, 0x0000, 0x0005, 0x0001,
	24,	  -1,	    -1, G_BUTTON  |0x0700,   /* Object 23 PASSEND */
  SELECTABLE|RBUTTON, NORMAL, "passend",
  0x0007, 0x0000, 0x0009, 0x0001,
	21,	  -1,	    -1, G_BUTTON  |0x0300,   /* Object 24  */
  SELECTABLE|RBUTTON, SELECTED, "Original",
  0x0011, 0x0000, 0x0009, 0x0001,
	39,	  26,	    36, G_BOX	  ,   /* Object 25  */
  NONE, NORMAL, 0x00FF1100L,
  0x0020, 0x0000, 0x001E, 0x0005,
	27,	  -1,	    -1, G_STRING  ,   /* Object 26  */
  NONE, NORMAL, "Image",
  0x000C, 0x0000, 0x0005, 0x0001,
	31, PIXEL   ,	    30, G_BOX	  ,   /* Object 27  */
  NONE, NORMAL, 0x00FF1123L,
  0x0001, 0x0001, 0x001C, 0x0001,
	29,	  -1,	    -1, G_BUTTON  |0x0300,   /* Object 28 PIXEL */
  SELECTABLE|RBUTTON, NORMAL, "Pixel 1:1",
  0x0000, 0x0000, 0x0009, 0x0001,
	30,	  -1,	    -1, G_BUTTON  ,   /* Object 29  */
  SELECTABLE|RBUTTON, NORMAL, "passend",
  0x000A, 0x0000, 0x0008, 0x0001,
	27,	  -1,	    -1, G_BUTTON  ,   /* Object 30  */
  SELECTABLE|RBUTTON, SELECTED, "Original",
  0x0013, 0x0000, 0x0009, 0x0001,
	32,	  -1,	    -1, G_TEXT	  ,   /* Object 31  */
  NONE, NORMAL, &rs_tedinfo[5],
  0x0001, 0x0002, 0x000D, 0x0001,
	33,	  -1,	    -1, G_TEXT	  ,   /* Object 32  */
  NONE, NORMAL, &rs_tedinfo[6],
  0x0010, 0x0002, 0x000D, 0x0001,
	36, XBRUCH  ,	    35, G_BOX	  ,   /* Object 33  */
  NONE, NORMAL, 0x00FF1123L,
  0x0001, 0x0003, 0x000D, 0x0001,
	35,	  -1,	    -1, G_BUTTON  ,   /* Object 34 XBRUCH */
  SELECTABLE|RBUTTON, NORMAL, "Bruch",
  0x0000, 0x0000, 0x0006, 0x0001,
	33,	  -1,	    -1, G_BUTTON  ,   /* Object 35  */
  SELECTABLE|RBUTTON, SELECTED, "Ganz",
  0x0007, 0x0000, 0x0006, 0x0001,
	25, YBRUCH  ,	    38, G_BOX	  ,   /* Object 36  */
  NONE, NORMAL, 0x00FF1123L,
  0x0010, 0x0003, 0x000D, 0x0001,
	38,	  -1,	    -1, G_BUTTON  |0x0400,   /* Object 37 YBRUCH */
  SELECTABLE|RBUTTON, NORMAL, "Bruch",
  0x0000, 0x0000, 0x0006, 0x0001,
	36,	  -1,	    -1, G_BUTTON  ,   /* Object 38  */
  SELECTABLE|RBUTTON, SELECTED, "Ganz",
  0x0007, 0x0000, 0x0006, 0x0001,
  OUTPAR  ,	  40,	    46, G_BOX	  ,   /* Object 39  */
  NONE, NORMAL, 0x00FF1100L,
  0x0020, 0x0006, 0x001E, 0x0005,
	41,	  -1,	    -1, G_TEXT	  ,   /* Object 40  */
  NONE, NORMAL, &rs_tedinfo[7],
  0x0002, 0x0000, 0x001A, 0x0001,
	45, LINKS   ,	    44, G_BOX	  ,   /* Object 41  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0001, 0x001A, 0x0001,
	43,	  -1,	    -1, G_BUTTON  |0x0400,   /* Object 42 LINKS */
  SELECTABLE|RBUTTON, NORMAL, "Links",
  0x0000, 0x0000, 0x0008, 0x0001,
	44,	  -1,	    -1, G_BUTTON  ,   /* Object 43  */
  SELECTABLE|RBUTTON, SELECTED, "Mitte",
  0x0009, 0x0000, 0x0008, 0x0001,
	41,	  -1,	    -1, G_BUTTON  |0x0400,   /* Object 44  */
  SELECTABLE|RBUTTON, NORMAL, "Rechts",
  0x0012, 0x0000, 0x0008, 0x0001,
	46,	  -1,	    -1, G_TEXT	  ,   /* Object 45  */
  NONE, NORMAL, &rs_tedinfo[8],
  0x0002, 0x0002, 0x001A, 0x0001,
	39, OBEN    ,	    49, G_BOX	  ,   /* Object 46  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0003, 0x001A, 0x0001,
	48,	  -1,	    -1, G_BUTTON  ,   /* Object 47 OBEN */
  SELECTABLE|RBUTTON, SELECTED, "Oben",
  0x0000, 0x0000, 0x0008, 0x0001,
	49,	  -1,	    -1, G_BUTTON  ,   /* Object 48  */
  SELECTABLE|RBUTTON, NORMAL, "Mitte",
  0x0009, 0x0000, 0x0008, 0x0001,
	46,	  -1,	    -1, G_BUTTON  ,   /* Object 49  */
  SELECTABLE|RBUTTON, NORMAL, "Unten",
  0x0012, 0x0000, 0x0008, 0x0001,
  LOAD	  ,	  51, IFILE   , G_BOX	  ,   /* Object 50 ALLPAR */
  NONE, NORMAL, 0x00FE1100L,
  0x0002, 0x0003, 0x003E, 0x000D,
	55,	  52,	    54, G_BOX	  ,   /* Object 51  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0001, 0x003A, 0x0001,
  OUTWIN  ,	  -1,	    -1, G_STRING  ,   /* Object 52  */
  NONE, NORMAL, "Standard-Ausgabe",
  0x0001, 0x0000, 0x0010, 0x0001,
	54,	  -1,	    -1, G_BUTTON  ,   /* Object 53 OUTWIN */
  SELECTABLE|RBUTTON, SELECTED, "Fenster",
  0x0016, 0x0000, 0x0010, 0x0001,
	51,	  -1,	    -1, G_BUTTON  ,   /* Object 54  */
  SELECTABLE|RBUTTON, NORMAL, "Ger\204t",
  0x0028, 0x0000, 0x0010, 0x0001,
	60,	  56, EXTSPOOL, G_BOX	  ,   /* Object 55  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0003, 0x003A, 0x0001,
  OUTINT  ,	  -1,	    -1, G_STRING  ,   /* Object 56  */
  NONE, NORMAL, "Control-P-Ausgabe",
  0x0001, 0x0000, 0x0011, 0x0001,
	58,	  -1,	    -1, G_BUTTON  ,   /* Object 57 OUTINT */
  SELECTABLE|RBUTTON, SELECTED, "Intern",
  0x0016, 0x0000, 0x000A, 0x0001,
  EXTSPOOL,	  -1,	    -1, G_BUTTON  ,   /* Object 58  */
  SELECTABLE|RBUTTON, NORMAL, "Extern:",
  0x0022, 0x0000, 0x000A, 0x0001,
	55,	  -1,	    -1, G_FTEXT   ,   /* Object 59 EXTSPOOL */
  EDITABLE, NORMAL, &rs_tedinfo[9],
  0x002E, 0x0000, 0x000A, 0x0001,
	64,	  61,	    63, G_BOX	  ,   /* Object 60  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0005, 0x003A, 0x0001,
  DOVER   ,	  -1,	    -1, G_STRING  ,   /* Object 61  */
  NONE, NORMAL, "Doppelklick-Standard",
  0x0001, 0x0000, 0x0014, 0x0001,
	63,	  -1,	    -1, G_BUTTON  ,   /* Object 62 DOVER */
  SELECTABLE|RBUTTON, SELECTED, "Neues Fenster",
  0x0016, 0x0000, 0x0010, 0x0001,
	60,	  -1,	    -1, G_BUTTON  ,   /* Object 63  */
  SELECTABLE|RBUTTON, NORMAL, "\232berschreiben",
  0x0028, 0x0000, 0x0010, 0x0001,
	65,	  -1,	    -1, G_TEXT	  ,   /* Object 64  */
  NONE, NORMAL, &rs_tedinfo[10],
  0x0002, 0x0006, 0x001C, 0x0001,
	66,	  -1,	    -1, G_TEXT	  ,   /* Object 65  */
  NONE, NORMAL, &rs_tedinfo[11],
  0x0020, 0x0006, 0x001C, 0x0001,
	69, FONTDEF ,	    68, G_BOX	  ,   /* Object 66  */
  NONE, NORMAL, 0x00FF1123L,
  0x0002, 0x0007, 0x001C, 0x0001,
	68,	  -1,	    -1, G_BUTTON  ,   /* Object 67 FONTDEF */
  SELECTABLE|RBUTTON, SELECTED, "Normal",
  0x0000, 0x0000, 0x000D, 0x0001,
	66,	  -1,	    -1, G_BUTTON  ,   /* Object 68  */
  SELECTABLE|RBUTTON, NORMAL, "\201ber VA_FILE",
  0x000F, 0x0000, 0x000D, 0x0001,
  IFSEL   , DITHCOL ,	    72, G_BOX	  ,   /* Object 69  */
  NONE, NORMAL, 0x00FF1123L,
  0x0020, 0x0007, 0x001C, 0x0001,
	71,	  -1,	    -1, G_BUTTON  ,   /* Object 70 DITHCOL */
  SELECTABLE|RBUTTON, NORMAL, "keine",
  0x0000, 0x0000, 0x0008, 0x0001,
	72,	  -1,	    -1, G_BUTTON  ,   /* Object 71  */
  SELECTABLE|RBUTTON, NORMAL, "2 (S/W)",
  0x000A, 0x0000, 0x0008, 0x0001,
	69,	  -1,	    -1, G_BUTTON  ,   /* Object 72  */
  SELECTABLE|RBUTTON, SELECTED, "8",
  0x0014, 0x0000, 0x0008, 0x0001,
  TOSNDPAR,	  -1,	    -1, G_BUTTON  ,   /* Object 73 IFSEL */
  SELECTABLE|EXIT, OUTLINED|SHADOWED, "Indexdatei:",
  0x0002, 0x0009, 0x000C, 0x0001,
  TOOUTPAR,	  -1,	    -1, G_BUTTON  |0x0800,   /* Object 74 TOSNDPAR */
  SELECTABLE|EXIT, DISABLED, "Sound-Parameter...",
  0x0013, 0x0009, 0x0013, 0x0001,
  IFILE   ,	  -1,	    -1, G_BUTTON  |0x0800,   /* Object 75 TOOUTPAR */
  SELECTABLE|EXIT, NORMAL, "Ger\204te-Parameter...",
  0x0028, 0x0009, 0x0014, 0x0001,
  ALLPAR  ,	  -1,	    -1, G_FBOXTEXT,   /* Object 76 IFILE */
  EDITABLE, OUTLINED, &rs_tedinfo[12],
  0x0002, 0x000B, 0x003A, 0x0001,
  SAVE	  ,	  -1,	    -1, G_BUTTON  |0x0100,   /* Object 77 LOAD */
  SELECTABLE|EXIT, NORMAL, "Laden",
  0x0002, 0x0011, 0x0009, 0x0002,
  OK	  ,	  -1,	    -1, G_BUTTON  ,   /* Object 78 SAVE */
  SELECTABLE|EXIT, NORMAL, "Sichern",
  0x000D, 0x0011, 0x0009, 0x0002,
  ABBRUCH ,	  -1,	    -1, G_BUTTON  ,   /* Object 79 OK */
  SELECTABLE|DEFAULT|EXIT, NORMAL, "OK",
  0x0019, 0x0011, 0x0009, 0x0002,
	81,	  -1,	    -1, G_BUTTON  ,   /* Object 80 ABBRUCH */
  SELECTABLE|EXIT, NORMAL, "Abbruch",
  0x0024, 0x0011, 0x0009, 0x0002,
	 0,	  82,	    82, G_BOX	  ,   /* Object 81  */
  NONE, OUTLINED, 0x00004100L,
  0x0031, 0x0011, 0x000F, 0x0002,
	81, MEM     , TOT     , G_BOX	  ,   /* Object 82  */
  NONE, SHADOWED, 0x00FF4100L,
  0x0000, 0x0000, 0x000F, 0x0002,
  TOT	  ,	  -1,	    -1, G_STRING  ,   /* Object 83 MEM */
  NONE, NORMAL, "MEM: XXXXXX K",
  0x0001, 0x0000, 0x000D, 0x0001,
	82,	  -1,	    -1, G_STRING  ,   /* Object 84 TOT */
  LASTOB, NORMAL, "TOT: XXXXXX K",
  0x0001, 0x0001, 0x000D, 0x0001,
	82,	  -1,	    -1, G_STRING  ,   /* Object 84 TOT */
  LASTOB, NORMAL, "TOT: XXXXXX K",
  0x0001, 0x0001, 0x000D, 0x0001
};

#define NUM_OBS   85
