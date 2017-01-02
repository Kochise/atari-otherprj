/*
 * jrdjfif.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains routines to decode standard JPEG file headers/markers.
 * This code will handle "raw JPEG" and JFIF-convention JPEG files.
 *
 * You can also use this module to decode a raw-JPEG or JFIF-standard data
 * stream that is embedded within a larger file.  To do that, you must
 * position the file to the JPEG SOI marker (0xFF/0xD8) that begins the
 * data sequence to be decoded.  If nothing better is possible, you can scan
 * the file until you see the SOI marker, then use JUNGETC to push it back.
 *
 * This module relies on the JGETC macro and the read_jpeg_data method (which
 * is provided by the user interface) to read from the JPEG data stream.
 * Therefore, this module is not dependent on any particular assumption about
 * the data source; it need not be a stdio stream at all.  (This fact does
 * NOT carry over to more complex JPEG file formats such as JPEG-in-TIFF;
 * those format control modules may well need to assume stdio input.)
 *
 * These routines are invoked via the methods read_file_header,
 * read_scan_header, read_jpeg_data, read_scan_trailer, and read_file_trailer.
 */

#include "jinclude.h"

#ifdef JFIF_SUPPORTED


typedef enum {			/* JPEG marker codes */
  M_SOF0  = 0xc0,
  M_SOF1  = 0xc1,
  M_SOF2  = 0xc2,
  M_SOF3  = 0xc3,

  M_SOF5  = 0xc5,
  M_SOF6  = 0xc6,
  M_SOF7  = 0xc7,

  M_JPG   = 0xc8,
  M_SOF9  = 0xc9,
  M_SOF10 = 0xca,
  M_SOF11 = 0xcb,

  M_SOF13 = 0xcd,
  M_SOF14 = 0xce,
  M_SOF15 = 0xcf,

  M_DHT   = 0xc4,

  M_DAC   = 0xcc,

  M_RST0  = 0xd0,
  M_RST1  = 0xd1,
  M_RST2  = 0xd2,
  M_RST3  = 0xd3,
  M_RST4  = 0xd4,
  M_RST5  = 0xd5,
  M_RST6  = 0xd6,
  M_RST7  = 0xd7,

  M_SOI   = 0xd8,
  M_EOI   = 0xd9,
  M_SOS   = 0xda,
  M_DQT   = 0xdb,
  M_DNL   = 0xdc,
  M_DRI   = 0xdd,
  M_DHP   = 0xde,
  M_EXP   = 0xdf,

  M_APP0  = 0xe0,
  M_APP15 = 0xef,

  M_JPG0  = 0xf0,
  M_JPG13 = 0xfd,
  M_COM   = 0xfe,

  M_TEM   = 0x01,

  M_ERROR = 0x100
} JPEG_MARKER;


/*
 * Routines to parse JPEG markers & save away the useful info.
 */


LOCAL INT32
get_2bytes (decompress_info_ptr cinfo)
/* Get a 2-byte unsigned integer (e.g., a marker parameter length field) */
{
  INT32 c1, c2;

  JGETC(cinfo, c1);
  c1 <<= 8;
  JGETC(cinfo, c2);
  c2 += c1;

  return c2;
}


LOCAL void
skip_variable (decompress_info_ptr cinfo, int code)
/* Skip over an unknown or uninteresting variable-length marker */
{
  INT32 length;

  length = get_2bytes(cinfo);

  TRACEMS2(cinfo->emethods, 1,
	   "Skipping marker 0x%02x, length %u", code, (int) length);

  length -= 2; while (--length >= 0) JSKIPC(cinfo);
}


LOCAL void
get_dht (decompress_info_ptr cinfo)
/* Process a DHT marker */
{
  INT32 length, i;
  JHUFF_TBL *htblbase, *htblptr;
  int s;
  UINT8 c, *htbl1, *htbl2;

  length = get_2bytes(cinfo) - 2;

  while ((length -= 1 + 16) >= 0) {
    JGETC(cinfo, i);

    TRACEMS1(cinfo->emethods, 1, "Define Huffman Table 0x%02x", (int) i);

    if (i & 0x10L) {		/* AC table definition */
      i -= i & 0x10L;
      htblbase = (JHUFF_TBL *) &cinfo->first_ac_huff_tbl;
    } else			/* DC table definition */
      htblbase = (JHUFF_TBL *) &cinfo->first_dc_huff_tbl;

    htblptr = htblbase->next_huff_tbl;
    for (;;) {
      if (htblptr == NULL) {
	htblptr = (JHUFF_TBL *)
	  (*cinfo->emethods->alloc_small) (SIZEOF(JHUFF_TBL));
	htblptr->huff_tbl_id = (short) i;
	htblptr->next_huff_tbl = htblbase->next_huff_tbl;
	htblbase->next_huff_tbl = htblptr;
      }
      if (htblptr->huff_tbl_id == (short) i)
	break;
      htblptr = htblptr->next_huff_tbl;
    }
    htbl2 = htbl1 = htblptr->huffval;

    s = 256; c = 16;
    do {
      JGETC(cinfo, i);
      if ((length -= i) < 0 || (s -= (int) i) < 0)
	ERREXIT(cinfo->emethods, "Bogus DHT counts");
      *htbl1++ = (UINT8) i;
      htbl1 += i;
    } while (--c);
/***
    TRACEMS8(cinfo->emethods, 2, "        %3d %3d %3d %3d %3d %3d %3d %3d",
	     htbl->bits[0], htbl->bits[1], htbl->bits[2], htbl->bits[3],
	     htbl->bits[4], htbl->bits[5], htbl->bits[6], htbl->bits[7]);
    TRACEMS8(cinfo->emethods, 2, "        %3d %3d %3d %3d %3d %3d %3d %3d",
	     htbl->bits[8], htbl->bits[9], htbl->bits[10], htbl->bits[11],
	     htbl->bits[12], htbl->bits[13], htbl->bits[14], htbl->bits[15]);
***/
    do if ((c = *htbl2++) != 0) do JGETC(cinfo, *htbl2++); while (--c);
    while (htbl2 != htbl1);

    c = 8; do *htbl2++ = 0; while (--c);
  }
}


LOCAL void
get_dac (decompress_info_ptr cinfo)
/* Process a DAC marker */
{
  INT32 length;
  int index, val;

  length = get_2bytes(cinfo) - 2;

  while ((length -= 2) >= 0) {
    JGETC(cinfo, index);
    JGETC(cinfo, val);

    TRACEMS2(cinfo->emethods, 1,
	     "Define Arithmetic Table 0x%02x: 0x%02x", index, val);

    if (index >= (2*NUM_ARITH_TBLS))
      ERREXIT1(cinfo->emethods, "Bogus DAC index %d", index);

    if (index >= NUM_ARITH_TBLS) { /* define AC table */
      cinfo->arith_ac_K[index-NUM_ARITH_TBLS] = (UINT8) val;
    } else {			/* define DC table */
      cinfo->arith_dc_L[index] = (UINT8) (val & 15);
      cinfo->arith_dc_U[index] = (UINT8) (val >> 4);
      if (cinfo->arith_dc_L[index] > cinfo->arith_dc_U[index])
	ERREXIT1(cinfo->emethods, "Bogus DAC value 0x%x", val);
    }
  }
}


extern const unsigned char ZAG[DCTSIZE2];


LOCAL void
get_dqt (decompress_info_ptr cinfo)
/* Process a DQT marker */
{
  INT32 length;
  int i, prec;
  JQUANT_TBL *qtblptr;
  UINT16 *quant_ptr;

  length = get_2bytes(cinfo) - 2;

  while (--length >= 0) {
    JGETC(cinfo, prec);
    i = prec & 15;
    prec >>= 4;

    TRACEMS2(cinfo->emethods, 1,
	     "Define Quantization Table %d  precision %d", i, prec);

    qtblptr = cinfo->first_quant_tbl;
    for (;;) {
      if (qtblptr == NULL) {
	qtblptr = (JQUANT_TBL *)
	  (*cinfo->emethods->alloc_small) (SIZEOF(JQUANT_TBL));
	qtblptr->quant_tbl_id = (short) i;
	qtblptr->next_quant_tbl = cinfo->first_quant_tbl;
	cinfo->first_quant_tbl = qtblptr;
      }
      if (qtblptr->quant_tbl_id == (short) i)
	break;
      qtblptr = qtblptr->next_quant_tbl;
    }
    quant_ptr = qtblptr->quantval;

    i = -DCTSIZE2; length -= DCTSIZE2;
    if (prec) {
      length -= DCTSIZE2;
      do quant_ptr[ZAG[DCTSIZE2 + i]] = (UINT16) get_2bytes(cinfo);
      while (++i < 0);
    } else {
      do JGETC(cinfo, quant_ptr[ZAG[DCTSIZE2 + i]]);
      while (++i < 0);
    }
/***
    i = 8 - DCTSIZE2;
    do {
      TRACEMS8(cinfo->emethods, 2, "        %4u %4u %4u %4u %4u %4u %4u %4u",
	       quant_ptr[(DCTSIZE2 - 8) + i], quant_ptr[(DCTSIZE2 - 7) + i],
	       quant_ptr[(DCTSIZE2 - 6) + i], quant_ptr[(DCTSIZE2 - 5) + i],
	       quant_ptr[(DCTSIZE2 - 4) + i], quant_ptr[(DCTSIZE2 - 3) + i],
	       quant_ptr[(DCTSIZE2 - 2) + i], quant_ptr[(DCTSIZE2 - 1) + i]);
    } while ((i += 8) <= 0);
***/
  }
}


LOCAL void
get_dri (decompress_info_ptr cinfo)
/* Process a DRI marker */
{
  if (get_2bytes(cinfo) - 4)
    ERREXIT(cinfo->emethods, "Bogus length in DRI");

  cinfo->restart_interval = (UINT16) get_2bytes(cinfo);

  TRACEMS1(cinfo->emethods, 1,
	   "Define Restart Interval %u", cinfo->restart_interval);
}


LOCAL void
get_app0 (decompress_info_ptr cinfo)
/* Process an APP0 marker */
{
#define JFIF_LEN 14
  INT32 length;
  UINT8 b[JFIF_LEN];
  int buffp;
  unsigned thumb;

  length = get_2bytes(cinfo) - 2;

  /* See if a JFIF APP0 marker is present */

  if (length >= JFIF_LEN) {
    length -= JFIF_LEN;
    buffp = -JFIF_LEN; do JGETC(cinfo, b[JFIF_LEN + buffp]); while (++buffp < 0);

    if (b[0]==0x4A && b[1]==0x46 && b[2]==0x49 && b[3]==0x46 && b[4]==0) {
      /* Found JFIF APP0 marker: check version */
      /* Major version must be 1, anything else signals an incompatible change.
       * We used to treat this as an error, but now it's a nonfatal warning,
       * because some bozo at Hijaak couldn't read the spec.
       * Minor version should be 0..2, but process anyway if newer.
       */
      if (b[5] != 1)
	WARNMS2(cinfo->emethods, "Warning: unknown JFIF revision number %d.%02d",
		b[5], b[6]);
      else if (b[6] > 2)
	TRACEMS2(cinfo->emethods, 1, "Unknown JFIF minor revision number %d.%02d",
		 b[5], b[6]);
      /* Save info */
      cinfo->density_unit = b[7];
      cinfo->X_density = (b[8] << 8) + b[9];
      cinfo->Y_density = (b[10] << 8) + b[11];
      /* Assume colorspace is YCbCr, unless UI has overridden me */
      if (cinfo->jpeg_color_space == CS_UNKNOWN)
	cinfo->jpeg_color_space = CS_YCbCr;
      TRACEMS3(cinfo->emethods, 1, "JFIF APP0 marker, density %dx%d  %d",
	       cinfo->X_density, cinfo->Y_density, cinfo->density_unit);
      if ((thumb = (unsigned) b[12] * (unsigned) b[13]) != 0)
	TRACEMS2(cinfo->emethods, 1, "    with %d x %d thumbnail image",
		 (unsigned) b[12], (unsigned) b[13]);
      if (length != (INT32) thumb * 3L)
	TRACEMS1(cinfo->emethods, 1,
		 "Warning: thumbnail image size does not match data length %u",
		 (int) length);
    } else {
      TRACEMS1(cinfo->emethods, 1, "Unknown APP0 marker (not JFIF), length %u",
	       (int) length + JFIF_LEN);
    }
  } else {
    TRACEMS1(cinfo->emethods, 1, "Short APP0 marker, length %u", (int) length);
  }

  while (--length >= 0)		/* skip any remaining data */
    JSKIPC(cinfo);
}


LOCAL void
get_sof (decompress_info_ptr cinfo, boolean is_prog, boolean is_arith)
/* Process a SOFn marker */
{
  short c, ci;
  jpeg_component_info *compptr;
  JSAMPIMAGE fullsize_data;

  cinfo->progressive_mode = is_prog;
  cinfo->arith_code = is_arith;

  c = (short) get_2bytes(cinfo);

  JGETC(cinfo, cinfo->data_precision);
  cinfo->image_height = get_2bytes(cinfo);
  cinfo->image_width  = get_2bytes(cinfo);
  JGETC(cinfo, ci);
  if (ci == 0) ci = 256;
  cinfo->num_components = ci;

  TRACEMS3(cinfo->emethods, 1,
	   "Start Of Frame: width=%u, height=%u, components=%d",
	   (int) cinfo->image_width, (int) cinfo->image_height, ci);

  /* We don't support files in which the image height is initially specified */
  /* as 0 and is later redefined by DNL.  As long as we have to check that,  */
  /* might as well have a general sanity check. */
  if (cinfo->image_height == 0 || cinfo->image_width == 0)
    ERREXIT(cinfo->emethods, "Empty JPEG image (DNL not supported)");

#ifdef EIGHT_BIT_SAMPLES
  if (cinfo->data_precision != 8)
    ERREXIT(cinfo->emethods, "Unsupported JPEG data precision");
#endif
#ifdef TWELVE_BIT_SAMPLES
  if (cinfo->data_precision != 12) /* this needs more thought?? */
    ERREXIT(cinfo->emethods, "Unsupported JPEG data precision");
#endif
#ifdef SIXTEEN_BIT_SAMPLES
  if (cinfo->data_precision != 16) /* this needs more thought?? */
    ERREXIT(cinfo->emethods, "Unsupported JPEG data precision");
#endif

  if ((ci * 3 + 8) != c)
    ERREXIT(cinfo->emethods, "Bogus SOF length");

  cinfo->fullsize_data = fullsize_data =
    (JSAMPIMAGE) (*cinfo->emethods->alloc_small)
      ((long) ci * (SIZEOF(JSAMPARRAY) + SIZEOF(jpeg_component_info)));

  cinfo->comp_info = compptr =
    (jpeg_component_info *) (fullsize_data + ci);

  do {
    JGETC(cinfo, compptr->component_id);
    JGETC(cinfo, c);
    if ((compptr->h_samp_factor = c >> 4) == 0)
      compptr->h_samp_factor = 16;
    if ((compptr->v_samp_factor = c & 15) == 0)
      compptr->v_samp_factor = 16;
    JGETC(cinfo, compptr->quant_tbl_no);

    TRACEMS4(cinfo->emethods, 1, "    Component %d: %dhx%dv q=%d",
	     compptr->component_id, compptr->h_samp_factor,
	     compptr->v_samp_factor, compptr->quant_tbl_no);

    compptr->fullsize_data_ptr = fullsize_data++; compptr++;
  } while (--ci);
}


LOCAL void
get_sos (decompress_info_ptr cinfo)
/* Process a SOS marker */
{
  short c, n, ci;
  jpeg_component_info *compptr;
  jpeg_component_info *curcomp;

  c = (short) get_2bytes(cinfo);

  JGETC(cinfo, n);  /* Number of components */

  if (n == 0) n = 256;
  cinfo->comps_in_scan = n;

  TRACEMS1(cinfo->emethods, 1, "Start Of Scan: %d components", n);

  if ((n * 2 + 6) != c)
    ERREXIT(cinfo->emethods, "Bogus SOS length");

  ci = cinfo->num_components;
  compptr = cinfo->comp_info;
  while (--ci >= 0) { compptr->next_comp_in_scan = 0; compptr++; }

  compptr = (jpeg_component_info *) &cinfo->first_comp_in_scan;
  do {
    JGETC(cinfo, c);
    ci = cinfo->num_components;
    curcomp = cinfo->comp_info;
    while (--ci >= 0 &&
	   (curcomp->component_id != c || curcomp->next_comp_in_scan))
      curcomp++;

    if (ci < 0)
      ERREXIT(cinfo->emethods, "Invalid component number in SOS");

    compptr->next_comp_in_scan = curcomp;
    compptr = curcomp;
    compptr->next_comp_in_scan = compptr;

    JGETC(cinfo, c);
    compptr->dc_tbl_no = c >> 4;
    compptr->ac_tbl_no = c & 15;

    TRACEMS3(cinfo->emethods, 1, "    c%d: [dc=%d ac=%d]",
	     compptr->component_id,
	     compptr->dc_tbl_no,
	     compptr->ac_tbl_no);
  } while (--n);
  compptr->next_comp_in_scan = 0;

  /* Collect the additional scan parameters Ss, Se, Ah/Al. */
  JGETC(cinfo, cinfo->Ss);
  JGETC(cinfo, cinfo->Se);
  JGETC(cinfo, c);
  cinfo->Ah = c >> 4;
  cinfo->Al = c & 15;

  /* Initialize next-restart state */
  cinfo->next_restart_num = 0;
}


LOCAL void
get_soi (decompress_info_ptr cinfo)
/* Process an SOI marker */
{
  int i;

  TRACEMS(cinfo->emethods, 1, "Start of Image");

  /* Reset all parameters that are defined to be reset by SOI */

  for (i = 0; i < NUM_ARITH_TBLS; i++) {
    cinfo->arith_dc_L[i] = 0;
    cinfo->arith_dc_U[i] = 1;
    cinfo->arith_ac_K[i] = 5;
  }
  cinfo->restart_interval = 0;

  cinfo->density_unit = 0;	/* set default JFIF APP0 values */
  cinfo->X_density = 1;
  cinfo->Y_density = 1;

  cinfo->CCIR601_sampling = FALSE; /* Assume non-CCIR sampling */
}


LOCAL int
next_marker (decompress_info_ptr cinfo)
/* Find the next JPEG marker */
/* Note that the output might not be a valid marker code, */
/* but it will never be 0 or FF */
{
  int c;
  long nbytes;

  if (cinfo->bits_left < 0) {
    if ((c = (int) cinfo->get_buffer & 0xFF) == 0) {
      WARNMS(cinfo->emethods, "Corrupt JPEG data: bad Huffman code");
      goto search;
    }
    WARNMS(cinfo->emethods,
	   "Corrupt JPEG data: premature end of data segment");
  } else {
    search:
    nbytes = 0;
    do {
      do {			/* skip any non-FF bytes */
	nbytes++;
	JGETC(cinfo, c);
      } while (c != 0xFF);
      do {			/* skip any duplicate FFs */
	/* we don't increment nbytes here since extra FFs are legal */
	JGETC(cinfo, c);
      } while (c == 0xFF);
    } while (c == 0);		/* repeat if it was a stuffed FF/00 */

    if (--nbytes)
      WARNMS2(cinfo->emethods,
	      "Corrupt JPEG data: %ld extraneous bytes before marker 0x%02x",
	      nbytes, c);
  }

  /* Reset bit counter */
  cinfo->bits_left = 0;

  return c;
}


LOCAL int
process_tables (decompress_info_ptr cinfo)
/* Scan and process JPEG markers that can appear in any order */
/* Return when an SOI, EOI, SOFn, or SOS is found */
{
  int c;

  for (;;) {
    c = next_marker(cinfo);

    switch (c) {
    case M_SOF0:
    case M_SOF1:
    case M_SOF2:
    case M_SOF3:
    case M_SOF5:
    case M_SOF6:
    case M_SOF7:
    case M_JPG:
    case M_SOF9:
    case M_SOF10:
    case M_SOF11:
    case M_SOF13:
    case M_SOF14:
    case M_SOF15:
    case M_SOI:
    case M_EOI:
    case M_SOS:
      return c;

    case M_DHT:
      get_dht(cinfo);
      break;

    case M_DAC:
      get_dac(cinfo);
      break;

    case M_DQT:
      get_dqt(cinfo);
      break;

    case M_DRI:
      get_dri(cinfo);
      break;

    case M_APP0:
      get_app0(cinfo);
      break;

    case M_RST0:		/* these are all parameterless */
    case M_RST1:
    case M_RST2:
    case M_RST3:
    case M_RST4:
    case M_RST5:
    case M_RST6:
    case M_RST7:
    case M_TEM:
      TRACEMS1(cinfo->emethods, 1, "Unexpected marker 0x%02x", c);
      break;

    default:	/* must be DNL, DHP, EXP, APPn, JPGn, COM, or RESn */
      skip_variable(cinfo, c);
    }
  }
}


/*
 * The entropy decoder calls this routine if it finds a marker other than
 * the restart marker it was expecting.  (This code is *not* used unless
 * a nonzero restart interval has been declared.)  The passed parameter is
 * the marker code actually found (might be anything, except 0 or FF).
 * The desired restart marker is that indicated by cinfo->next_restart_num.
 * This routine is supposed to apply whatever error recovery strategy seems
 * appropriate in order to position the input stream to the next data segment.
 * For some file formats (eg, TIFF) extra information such as tile boundary
 * pointers may be available to help in this decision.
 *
 * This implementation is substantially constrained by wanting to treat the
 * input as a data stream; this means we can't back up.  (For instance, we
 * generally can't fseek() if the input is a Unix pipe.)  Therefore, we have
 * only the following actions to work with:
 *   1. Do nothing, let the entropy decoder resume at next byte of file.
 *   2. Read forward until we find another marker, discarding intervening
 *      data.  (In theory we could look ahead within the current bufferload,
 *      without having to discard data if we don't find the desired marker.
 *      This idea is not implemented here, in part because it makes behavior
 *      dependent on buffer size and chance buffer-boundary positions.)
 *   3. Push back the passed marker.
 *      This will cause the entropy decoder to process an empty data segment,
 *      and then re-read the marker we pushed back.
 * #2 is appropriate if we think the desired marker lies ahead, while #3 is
 * appropriate if the found marker is a future restart marker (indicating
 * that we have missed the desired restart marker, probably because it got
 * corrupted).

 * We apply #2 or #3 if the found marker is a restart marker no more than
 * two counts behind or ahead of the expected one.  We also apply #2 if the
 * found marker is not a legal JPEG marker code (it's certainly bogus data).
 * If the found marker is a restart marker more than 2 counts away, we do #1
 * (too much risk that the marker is erroneous; with luck we will be able to
 * resync at some future point).
 * For any valid non-restart JPEG marker, we apply #3.  This keeps us from
 * overrunning the end of a scan.  An implementation limited to single-scan
 * files might find it better to apply #2 for markers other than EOI, since
 * any other marker would have to be bogus data in that case.
 */

LOCAL void
resync_to_restart (decompress_info_ptr cinfo, int marker)
{
  int desired = cinfo->next_restart_num;
  int action = 1;

  /* Always put up a warning. */
  WARNMS2(cinfo->emethods,
	  "Corrupt JPEG data: found 0x%02x marker instead of RST%d",
	  marker, desired);
  /* Outer loop handles repeated decision after scanning forward. */
  for (;;) {
    if (marker < M_SOF0)
      action = 2;		/* invalid marker */
    else if (marker < M_RST0 || marker > M_RST7)
      action = 3;		/* valid non-restart marker */
    else {
      if (marker == (M_RST0 + ((desired+1) & 7)) ||
	  marker == (M_RST0 + ((desired+2) & 7)))
	action = 3;		/* one of the next two expected restarts */
      else if (marker == (M_RST0 + ((desired-1) & 7)) ||
	       marker == (M_RST0 + ((desired-2) & 7)))
	action = 2;		/* a prior restart, so advance */
      else
	action = 1;		/* desired restart or too far away */
    }
    TRACEMS2(cinfo->emethods, 4,
	     "At marker 0x%02x, recovery action %d", marker, action);
    switch (action) {
    case 1:
      /* Let entropy decoder resume processing. */
      return;
    case 2:
      /* Scan to the next marker, and repeat the decision loop. */
      marker = next_marker(cinfo);
      break;
    case 3:
      /* Put back this marker & return. */
      /* Entropy decoder will be forced to process an empty segment. */
      cinfo->get_buffer = marker;
      cinfo->bits_left--;
      return;
    }
  }
}



/*
 * Initialize and read the file header (everything through the SOF marker).
 */

METHODDEF void
read_file_header (decompress_info_ptr cinfo)
{
  int c;

  /* Demand an SOI marker at the start of the file --- otherwise it's
   * probably not a JPEG file at all.  If the user interface wants to support
   * nonstandard headers in front of the SOI, it must skip over them itself
   * before calling jpeg_decompress().
   */
  if (next_marker(cinfo) != M_SOI)
    ERREXIT(cinfo->emethods, "Not a JPEG file");

  get_soi(cinfo);		/* OK, process SOI */

  /* Process markers until SOF */
  c = process_tables(cinfo);

  switch (c) {
  case M_SOF0:			/* Baseline */
  case M_SOF1:			/* Extended sequential, Huffman */
    get_sof(cinfo, FALSE, FALSE);
    break;
  case M_SOF2:			/* Progressive, Huffman */
    get_sof(cinfo, TRUE, FALSE);
    break;
  case M_SOF9:			/* Extended sequential, arithmetic */
    get_sof(cinfo, FALSE, TRUE);
    break;
  case M_SOF10:			/* Progressive, arithmetic */
    get_sof(cinfo, TRUE, TRUE);
    break;
  default:
    ERREXIT1(cinfo->emethods, "Unsupported SOF marker type 0x%02x", c);
  }

  /* Figure out what colorspace we have */
  /* (too bad the JPEG committee didn't provide a real way to specify this) */

  switch (cinfo->num_components) {
  case 1:
    cinfo->jpeg_color_space = CS_GRAYSCALE;
    break;

  case 3:
    /* if we saw a JFIF marker, leave it set to YCbCr; */
    /* also leave it alone if UI has provided a value */
    if (cinfo->jpeg_color_space == CS_UNKNOWN) {
      short cid0 = cinfo->comp_info[0].component_id;
      short cid1 = cinfo->comp_info[1].component_id;
      short cid2 = cinfo->comp_info[2].component_id;

      if (cid0 == 1 && cid1 == 2 && cid2 == 3)
	cinfo->jpeg_color_space = CS_YCbCr; /* assume JFIF w/out marker */
      else if (cid0 == 82 && cid1 == 71 && cid2 == 66)
	cinfo->jpeg_color_space = CS_RGB; /* ASCII 'R', 'G', 'B' */
      else {
	TRACEMS3(cinfo->emethods, 1,
		 "Unrecognized component IDs %d %d %d, assuming YCbCr",
		 cid0, cid1, cid2);
	cinfo->jpeg_color_space = CS_YCbCr;
      }
    }
    break;

  default:
    cinfo->jpeg_color_space = CS_UNKNOWN;
  }
}


/*
 * Read the start of a scan (everything through the SOS marker).
 * Return TRUE if find SOS, FALSE if find EOI.
 */

METHODDEF boolean
read_scan_header (decompress_info_ptr cinfo)
{
  int c;

  /* Process markers until SOS or EOI */
  c = process_tables(cinfo);

  switch (c) {
  case M_SOS:
    get_sos(cinfo);
    return TRUE;

  case M_EOI:
    TRACEMS(cinfo->emethods, 1, "End Of Image");
    return FALSE;

  default:
    ERREXIT1(cinfo->emethods, "Unexpected marker 0x%02x", c);
  }
  return FALSE;			/* keeps lint happy */
}


/*
 * Read a restart marker, which is expected to appear next in the datastream;
 * if the marker is not there, take appropriate recovery action.
 *
 * This is called by the entropy decoder after it has read an appropriate
 * number of MCUs.
 */

METHODDEF void
read_restart_marker (decompress_info_ptr cinfo)
{
  int c = next_marker(cinfo);

  if (c != (M_RST0 + cinfo->next_restart_num)) {
    /* Uh-oh, the restart markers have been messed up too. */
    /* Try to figure out how to resync. */
    resync_to_restart(cinfo, c);
  }
  else {
    TRACEMS1(cinfo->emethods, 2, "RST%d", cinfo->next_restart_num);
  }

  /* Update next-restart state */
  cinfo->next_restart_num = (cinfo->next_restart_num + 1) & 7;
}


/*
 * Finish up after a compressed scan (series of read_jpeg_data calls);
 * prepare for another read_scan_header call.
 */

METHODDEF void
read_trailer ()
{
  /* no work needed */
}


/*
 * The method selection routine for standard JPEG header reading.
 * Note that this must be called by the user interface before calling
 * jpeg_decompress.  When a non-JFIF file is to be decompressed (TIFF,
 * perhaps), the user interface must discover the file type and call
 * the appropriate method selection routine.
 */

GLOBAL void
jselrjfif (decompress_info_ptr cinfo)
{
  cinfo->read_file_header = read_file_header;
  cinfo->read_scan_header = read_scan_header;
  cinfo->read_restart_marker = read_restart_marker;
  cinfo->read_scan_trailer = read_trailer;
  cinfo->read_file_trailer = read_trailer;
}

#endif /* JFIF_SUPPORTED */
