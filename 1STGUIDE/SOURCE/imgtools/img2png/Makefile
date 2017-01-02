
CC=gcc

PNGLIBDIR=/usr/lib
ZLIBDIR=/usr/lib
PNGINCDIR=/usr/include
ZINCDIR=/usr/include

LIBS=$(PNGLIBDIR)/libpng.a $(ZLIBDIR)/libz.a
INCLUDES=-I$(PNGINCDIR) -I$(ZINCDIR)

CFLAGS=$(INCLUDES) -O2 -Wall
LDFLAGS=-s

OBJS=img2png.o l3decode.o true_out.o

all: img2png

img2png: $(OBJS) $(LIBS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

img2png.o: img2png.c imgcodec.h
l3decode.o: l3decode.c imgcodec.h
true_out.o: true_out.c imgcodec.h
