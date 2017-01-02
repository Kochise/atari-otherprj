/*
 * jwrjfif.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains routines to write standard JPEG file headers/markers.
 * The file format created is a raw JPEG data stream with (optionally) an
 * APP0 marker per the JFIF spec.  This will handle baseline and
 * JFIF-convention JPEG files, although there is currently no provision
 * for inserting a thumbnail image in the JFIF header.
 *
 * These routines are invoked via the methods write_file_header,
 * write_scan_header, write_jpeg_data, write_scan_trailer, and
 * write_file_trailer.
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


LOCAL void
emit_marker (compress_info_ptr cinfo, JPEG_MARKER mark)
/* Emit a marker code */
{
  JPUTC(cinfo, 0xFF);
  JPUTC(cinfo, mark);
}


LOCAL void
emit_2bytes (compress_info_ptr cinfo, int value)
/* Emit a 2-byte integer; these are always MSB first in JPEG files */
{
  JPUTC(cinfo, value >> 8);
  JPUTC(cinfo, value);
}


LOCAL int
emit_dqt (compress_info_ptr cinfo, int index)
/* Emit a DQT marker */
/* Returns the precision used (0 = 8bits, 1 = 16bits) for baseline checking */
{
  QUANT_TBL_PTR data = cinfo->quant_tbl_ptrs[index];
  int prec = 0;
  int i;

  for (i = 0; i < DCTSIZE2; i++) {
    if (data[i] > 255)
      prec = 1;
  }

  emit_marker(cinfo, M_DQT);

  emit_2bytes(cinfo, prec ? DCTSIZE2*2 + 1 + 2 : DCTSIZE2 + 1 + 2);

  JPUTC(cinfo, index + (prec<<4));

  for (i = 0; i < DCTSIZE2; i++) {
    if (prec)
      JPUTC(cinfo, data[i] >> 8);
    JPUTC(cinfo, data[i]);
  }

  return prec;
}


LOCAL void
emit_dht (compress_info_ptr cinfo, int index, boolean is_ac)
/* Emit a DHT marker */
{
  HUFF_TBL * htbl;
  UINT8 b, * p1, * p2;
  INT32 c;

  if (is_ac) {
    htbl = cinfo->ac_huff_tbl_ptrs[index];
    index += 0x10;		/* output index has AC bit set */
  } else {
    htbl = cinfo->dc_huff_tbl_ptrs[index];
  }

  if (htbl == NULL)
    ERREXIT1(cinfo->emethods, "Huffman table 0x%02x was not defined", index);

  if (htbl->sent_table == FALSE) {
    emit_marker(cinfo, M_DHT);

    p1 = htbl->hufftbl; b = 16;
    do {
      c = *p1++;
      p1 += c;
    } while (--b);

    emit_2bytes(cinfo, 2 + 1 + (int) (p1 - htbl->hufftbl));
    JPUTC(cinfo, index);

    p2 = htbl->hufftbl;
    do {
      c = *p2++;
      p2 += c;
      JPUTC(cinfo, (int) c);
    } while (p2 < p1);

    p2 = htbl->hufftbl;
    do if ((b = *p2++) != 0) do JPUTC(cinfo, *p2++); while (--b);
    while (p2 < p1);

    htbl->sent_table = TRUE;
  }
}


LOCAL void
emit_dac (compress_info_ptr cinfo)
/* Emit a DAC marker */
/* Since the useful info is so small, we want to emit all the tables in */
/* one DAC marker.  Therefore this routine does its own scan of the table. */
{
  char dc_in_use[NUM_ARITH_TBLS];
  char ac_in_use[NUM_ARITH_TBLS];
  int length, i;

  for (i = 0; i < NUM_ARITH_TBLS; i++)
    dc_in_use[i] = ac_in_use[i] = 0;

  for (i = 0; i < cinfo->num_components; i++) {
    dc_in_use[cinfo->comp_info[i].dc_tbl_no] = 1;
    ac_in_use[cinfo->comp_info[i].ac_tbl_no] = 1;
  }

  length = 0;
  for (i = 0; i < NUM_ARITH_TBLS; i++)
    length += dc_in_use[i] + ac_in_use[i];

  emit_marker(cinfo, M_DAC);

  emit_2bytes(cinfo, length*2 + 2);

  for (i = 0; i < NUM_ARITH_TBLS; i++) {
    if (dc_in_use[i]) {
      JPUTC(cinfo, i);
      JPUTC(cinfo, cinfo->arith_dc_L[i] + (cinfo->arith_dc_U[i]<<4));
    }
    if (ac_in_use[i]) {
      JPUTC(cinfo, i + 0x10);
      JPUTC(cinfo, cinfo->arith_ac_K[i]);
    }
  }
}


LOCAL void
emit_dri (compress_info_ptr cinfo)
/* Emit a DRI marker */
{
  emit_marker(cinfo, M_DRI);

  emit_2bytes(cinfo, 4);	/* fixed length */

  emit_2bytes(cinfo, (int) cinfo->restart_interval);
}


LOCAL void
emit_sof (compress_info_ptr cinfo, JPEG_MARKER code)
/* Emit a SOF marker */
{
  long i;

  emit_marker(cinfo, code);

  emit_2bytes(cinfo, 3 * cinfo->num_components + 8); /* length */

  i = 65535L;
  if (cinfo->image_height > i || cinfo->image_width > i)
    ERREXIT(cinfo->emethods, "Maximum image dimension for JFIF is 65535 pixels");

  JPUTC(cinfo, cinfo->data_precision);
  emit_2bytes(cinfo, (int) cinfo->image_height);
  emit_2bytes(cinfo, (int) cinfo->image_width);
  JPUTC(cinfo, cinfo->num_components);

  for (i = 0; (short)i < cinfo->num_components; i++) {
    JPUTC(cinfo, cinfo->comp_info[i].component_id);
    JPUTC(cinfo, (cinfo->comp_info[i].h_samp_factor << 4)
		     + cinfo->comp_info[i].v_samp_factor);
    JPUTC(cinfo, cinfo->comp_info[i].quant_tbl_no);
  }
}


LOCAL void
emit_sos (compress_info_ptr cinfo)
/* Emit a SOS marker */
{
  long i;

  emit_marker(cinfo, M_SOS);

  emit_2bytes(cinfo, 2 * cinfo->comps_in_scan + 6); /* length */

  JPUTC(cinfo, cinfo->comps_in_scan);

  for (i = 0; (short)i < cinfo->comps_in_scan; i++) {
    JPUTC(cinfo, cinfo->cur_comp_info[i]->component_id);
    JPUTC(cinfo, (cinfo->cur_comp_info[i]->dc_tbl_no << 4)
		     + cinfo->cur_comp_info[i]->ac_tbl_no);
  }

  JPUTC(cinfo, 0);		/* Spectral selection start */
  JPUTC(cinfo, DCTSIZE2-1);	/* Spectral selection end */
  JPUTC(cinfo, 0);		/* Successive approximation */
}


LOCAL void
emit_jfif_app0 (compress_info_ptr cinfo)
/* Emit a JFIF-compliant APP0 marker */
{
  /*
   * Length of APP0 block	(2 bytes)
   * Block ID			(4 bytes - ASCII "JFIF")
   * Zero byte			(1 byte to terminate the ID string)
   * Version Major, Minor	(2 bytes - 0x01, 0x01)
   * Units			(1 byte - 0x00 = none, 0x01 = inch, 0x02 = cm)
   * Xdpu			(2 bytes - dots per unit horizontal)
   * Ydpu			(2 bytes - dots per unit vertical)
   * Thumbnail X size		(1 byte)
   * Thumbnail Y size		(1 byte)
   */

  emit_marker(cinfo, M_APP0);

  emit_2bytes(cinfo, 2 + 4 + 1 + 2 + 1 + 2 + 2 + 1 + 1); /* length */

  JPUTC(cinfo, 0x4A);	/* Identifier: ASCII "JFIF" */
  JPUTC(cinfo, 0x46);
  JPUTC(cinfo, 0x49);
  JPUTC(cinfo, 0x46);
  JPUTC(cinfo, 0);
  JPUTC(cinfo, 1);	/* Major version */
  JPUTC(cinfo, 1);	/* Minor version */
  JPUTC(cinfo, cinfo->density_unit); /* Pixel size information */
  emit_2bytes(cinfo, (int) cinfo->X_density);
  emit_2bytes(cinfo, (int) cinfo->Y_density);
  JPUTC(cinfo, 0);	/* No thumbnail image */
  JPUTC(cinfo, 0);
}


LOCAL void
emit_comment (compress_info_ptr cinfo)
/* Emit a comment */
{
  char *p, *q;

  p = q = cinfo->comment_to_write; while (*q++);

  emit_marker(cinfo, M_COM);

  emit_2bytes(cinfo, (int) (q - p + 1)); /* length */

  while (*p) JPUTC(cinfo, *p++);
}


/*
 * Write the file header.
 */


METHODDEF void
write_file_header (compress_info_ptr cinfo)
{
  char qt_in_use[NUM_QUANT_TBLS];
  int i, prec;
  boolean is_baseline;

  emit_marker(cinfo, M_SOI);	/* first the SOI */

  if (cinfo->write_JFIF_header)	/* next an optional JFIF APP0 */
    emit_jfif_app0(cinfo);

  if (cinfo->comment_to_write)	/* next an optional comment */
    emit_comment(cinfo);

  /* Emit DQT for each quantization table. */
  /* Note that doing it here means we can't adjust the QTs on-the-fly. */
  /* If we did want to do that, we'd have a problem with checking precision */
  /* for the is_baseline determination. */

  for (i = 0; i < NUM_QUANT_TBLS; i++)
    qt_in_use[i] = 0;

  for (i = 0; i < cinfo->num_components; i++)
    qt_in_use[cinfo->comp_info[i].quant_tbl_no] = 1;

  prec = 0;
  for (i = 0; i < NUM_QUANT_TBLS; i++) {
    if (qt_in_use[i])
      prec += emit_dqt(cinfo, i);
  }
  /* now prec is nonzero iff there are any 16-bit quant tables. */

  /* Check for a non-baseline specification. */
  /* Note we assume that Huffman table numbers won't be changed later. */
  is_baseline = TRUE;
  if (cinfo->arith_code || (cinfo->data_precision != 8))
    is_baseline = FALSE;
  for (i = 0; i < cinfo->num_components; i++) {
    if (cinfo->comp_info[i].dc_tbl_no > 1 || cinfo->comp_info[i].ac_tbl_no > 1)
      is_baseline = FALSE;
  }
  if (prec && is_baseline) {
    is_baseline = FALSE;
    /* If it's baseline except for quantizer size, warn the user */
    TRACEMS(cinfo->emethods, 0,
	    "Caution: quantization tables are too coarse for baseline JPEG");
  }


  /* Emit the proper SOF marker */
  if (cinfo->arith_code)
    emit_sof(cinfo, M_SOF9);	/* SOF code for arithmetic coding */
  else if (is_baseline)
    emit_sof(cinfo, M_SOF0);	/* SOF code for baseline implementation */
  else
    emit_sof(cinfo, M_SOF1);	/* SOF code for non-baseline Huffman file */
}


/*
 * Write the start of a scan (everything through the SOS marker).
 */

METHODDEF void
write_scan_header (compress_info_ptr cinfo)
{
  int i;

  if (cinfo->arith_code) {
    /* Emit arith conditioning info.  We will have some duplication
     * if the file has multiple scans, but it's so small it's hardly
     * worth worrying about.
     */
    emit_dac(cinfo);
  } else {
    /* Emit Huffman tables.  Note that emit_dht takes care of
     * suppressing duplicate tables.
     */
    for (i = 0; i < cinfo->comps_in_scan; i++) {
      emit_dht(cinfo, cinfo->cur_comp_info[i]->dc_tbl_no, FALSE);
      emit_dht(cinfo, cinfo->cur_comp_info[i]->ac_tbl_no, TRUE);
    }
  }

  /* Emit DRI if required --- note that DRI value could change for each scan.
   * If it doesn't, a tiny amount of space is wasted in multiple-scan files.
   * We assume DRI will never be nonzero for one scan and zero for a later one.
   */
  if (cinfo->restart_interval)
    emit_dri(cinfo);

  emit_sos(cinfo);

  /* Initialize next-restart state */
  cinfo->next_restart_num = 0;
}


/*
 * Write a restart marker.
 */

METHODDEF void
write_restart_marker (compress_info_ptr cinfo)
{
  emit_marker(cinfo, M_RST0 + cinfo->next_restart_num);

  /* Update next-restart state */
  cinfo->next_restart_num = (cinfo->next_restart_num + 1) & 7;
}


/*
 * Finish up after a compressed scan (series of write_jpeg_data calls).
 */

METHODDEF void
write_scan_trailer (compress_info_ptr cinfo)
{
  /* no work needed in this format */
}


/*
 * Finish up at the end of the file.
 */

METHODDEF void
write_file_trailer (compress_info_ptr cinfo)
{
  emit_marker(cinfo, M_EOI);
  (*cinfo->methods->write_jpeg_data) (cinfo);
}


/*
 * The method selection routine for standard JPEG header writing.
 * This should be called from c_ui_method_selection if appropriate.
 */

GLOBAL void
jselwjfif (compress_info_ptr cinfo)
{
  cinfo->methods->write_file_header = write_file_header;
  cinfo->methods->write_scan_header = write_scan_header;
  cinfo->methods->write_restart_marker = write_restart_marker;
  cinfo->methods->write_scan_trailer = write_scan_trailer;
  cinfo->methods->write_file_trailer = write_file_trailer;
}

#endif /* JFIF_SUPPORTED */
