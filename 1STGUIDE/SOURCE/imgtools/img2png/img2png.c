#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <png.h>

#include "imgcodec.h"


typedef struct tput
{
  TPUTPUB pub;
  png_structp png_ptr;
}
TPUT;

static TPUT trueput;

static jmp_buf myjmpbuf;

#define BUF_LENGTH 80L*1024

typedef struct
{
  FBUFPUB pub;
  char fbuf[BUF_LENGTH];
}
FBUF;

static void Fbufread(FBUF *fp)
{
  long count;

  if ((count = fread(fp->fbuf, sizeof(char), BUF_LENGTH, stdin)) > 0)
  {
    fp->pub.bytes_left += count;
    fp->pub.pbuf = fp->fbuf;
  }
  else
  {
    /* Return to top level routine on short read. */
    longjmp(myjmpbuf, 1);
} }

static FBUF input;

static TIMG_HEADER header;

static png_infop info_ptr;

static png_color_8 sig_bit;

static png_color palette[256];

static unsigned short defcols[] =
	   {
		1000, 1000, 1000,
		1000,    0,    0,
		   0, 1000,    0,
		1000, 1000,    0,
		   0,    0, 1000,
		1000,    0, 1000,
		   0, 1000, 1000,
		 666,  666,  666,
		 333,  333,  333,
		 666,    0,    0,
		   0,  666,    0,
		 666,  666,    0,
		   0,    0,  666,
		 666,    0,  666,
		   0,  666,  666,
		   0,    0,    0
	   };

static png_text text_ptr;

static void puttruerow(TPUT *tp, unsigned char *data,
		       short num_cols, short num_comps)
{
  png_write_row(tp->png_ptr, data);
}

static void my_exit(void)
{
  /* Return to top level routine if image end is reached. */
  longjmp(myjmpbuf, 2);
}

#define MKVAL(a,b,c,d) (((unsigned long)a << 24) | ((unsigned long)b << 16) |\
  ((unsigned long)c << 8) | (unsigned long)d)

int main(int argc, char **argv)
{
  unsigned short temp, *p;
  unsigned long count;
  IBUFPUB *image;
  int idx;

  if (isatty(fileno(stdin)))
  {
    fprintf(stderr, "usage: img2png <input.img >output.png\n");
    return 0;
  }

  input.pub.bytes_left = 0;
  /* Note: pub.pbuf entry does not need to be set here. */
  input.pub.data_func = (void *)Fbufread;

  if (setjmp(myjmpbuf)) return 1;

  p = (unsigned short *)&header.ih; count = sizeof(header.ih);
  do { FGETW(&input.pub, temp); *p++ = temp; } while (count -= 2);

  header.comps = 0;

  if (header.ih.headlen >= 11)
  {
    FGETL(&input.pub, count); *((unsigned long *)p)++ = count;
    FGETW(&input.pub, temp); *p++ = temp;

    if (count == MKVAL('T','I','M','G'))
    {
      if (temp != 3) return 1;

      FGETW(&input.pub, temp); *p++ = temp; sig_bit.red = temp;
      FGETW(&input.pub, temp); *p++ = temp; sig_bit.green = temp;
      FGETW(&input.pub, temp); *p++ = temp; sig_bit.blue = temp;

      count = ((long)header.ih.headlen << 1) - sizeof(header);
    }
    else if (count == MKVAL('X','I','M','G'))
    {
      if (temp != 0) return 1;
      if (header.ih.planes != 1 &&
	  header.ih.planes != 2 &&
	  header.ih.planes != 4 &&
	  header.ih.planes != 8) return 1;

      header.comps = 1;
      header.red = header.ih.planes;
      header.green = 0;
      header.blue = 0;

      count = ((long)header.ih.headlen << 1) - (sizeof(header) - 6);

      for (idx = 0; idx < (1 << header.ih.planes); idx++)
      {
	FGETW(&input.pub, temp);
	palette[idx].red =
	(temp * (unsigned short)51 + (unsigned short)100) / (unsigned short)200;
	FGETW(&input.pub, temp);
	palette[idx].green =
	(temp * (unsigned short)51 + (unsigned short)100) / (unsigned short)200;
	FGETW(&input.pub, temp);
	palette[idx].blue =
	(temp * (unsigned short)51 + (unsigned short)100) / (unsigned short)200;
	count -= 6;
      }
    }
  }

  if (header.comps == 0)
  {
    if (header.ih.planes != 1 &&
	header.ih.planes != 2 &&
	header.ih.planes != 4) return 1;

    header.comps = 1;
    header.red = header.ih.planes;
    header.green = 0;
    header.blue = 0;

    p = defcols;
    for (idx = 0; idx < (1 << header.ih.planes) - 1; idx++)
    {
      palette[idx].red =
      (*p++ * (unsigned short)51 + (unsigned short)100) / (unsigned short)200;
      palette[idx].green =
      (*p++ * (unsigned short)51 + (unsigned short)100) / (unsigned short)200;
      palette[idx].blue =
      (*p++ * (unsigned short)51 + (unsigned short)100) / (unsigned short)200;
    }
    palette[idx].red = 0;
    palette[idx].green = 0;
    palette[idx].blue = 0;

    count = ((long)header.ih.headlen << 1) - sizeof(header.ih);
  }

  if (count) do FSKIPC(&input.pub); while (--count);

  image = true_out_init(&header, &trueput.pub,
			(void *)malloc, my_exit,
			header.comps);

  if (image == 0) return 3;

  trueput.pub.put_true_pix_row = (void *)puttruerow;

  trueput.png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if (trueput.png_ptr == 0) return 4;

  info_ptr = png_create_info_struct(trueput.png_ptr);
  if (info_ptr == 0)
  {
    png_destroy_write_struct(&trueput.png_ptr, 0);
    return 5;
  }

  if (setjmp(trueput.png_ptr->jmpbuf))
  {
    png_destroy_write_struct(&trueput.png_ptr, 0);
    return 6;
  }

  png_init_io(trueput.png_ptr, stdout);

  if (header.comps == 3)
  {
    png_set_IHDR(trueput.png_ptr, info_ptr,
      header.ih.sl_width, header.ih.sl_height, 8, PNG_COLOR_TYPE_RGB,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_set_sBIT(trueput.png_ptr, info_ptr, &sig_bit);
  }
  else
  {
    png_set_IHDR(trueput.png_ptr, info_ptr,
      header.ih.sl_width, header.ih.sl_height, header.ih.planes,
      PNG_COLOR_TYPE_PALETTE,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_set_PLTE(trueput.png_ptr, info_ptr, palette, 1 << header.ih.planes);
  }

  text_ptr.key = "Software";
  text_ptr.text = "img2png " __DATE__;
  text_ptr.compression = PNG_TEXT_COMPRESSION_NONE;
  png_set_text(trueput.png_ptr, info_ptr, &text_ptr, 1);

  png_write_info(trueput.png_ptr, info_ptr);

  if (header.comps == 1 && header.ih.planes != 8)
    png_set_packing(trueput.png_ptr);

  if (setjmp(myjmpbuf) == 0)

    /* OK, here's the real work... */
    level_3_decode(&input.pub, image);

  png_write_end(trueput.png_ptr, info_ptr);
  png_destroy_write_struct(&trueput.png_ptr, 0);

  free(image);
  /* My, that was easy, wasn't it? */

  return 0;
}
