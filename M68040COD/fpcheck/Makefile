#
#	Makefile for 68040 floating point test driver
#

#		Copyright (C) Motorola, Inc. 1990
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README.lic, in this same directory.

#
# Select PLATFORM:
#	SUN = SunOs and BSD4.x systems
#	SYSV= AT&T System V
#
PLATFORM=SUN
#PLATFORM=SYSV

#
# Select LIST:
#	LIST   = 881/882 instructions only
#	LIST40 = 881/882 + new '040 instructions (FSADD, FDADD, et.al)
#
LIST=LIST
#LIST=LIST40

LOCAL = 

CFLAGS	= -O -D$(PLATFORM) $(LOCAL)
LDFLAGS = -Bstatic
TARGETS	= fpcheck fprand fplegal 

OBJS	= fpcheck.o allops.o drivers.o convert.o

INSDIR	= /usr/bin

#
#	For standalone operation
#
ENTRY	=	main
TEXTSTRT=	10000
SOBJS	= fpchecks.o allops.o drivers.o convert.o standalone.o

TEMPLATES= $(PLATFORM).$(LIST) $(PLATFORM).MONADIC $(PLATFORM).DYADIC \
	$(PLATFORM).SINCOS $(PLATFORM).MOVECR $(PLATFORM).MOVEO \
	$(PLATFORM).MOVEPS $(PLATFORM).MOVEPD $(PLATFORM).TST \
	$(PLATFORM).SGLDIV $(PLATFORM).SGLMUL $(PLATFORM).BCC


all:	$(TARGETS)

fpcheck: $(OBJS)
	cc -s -o fpcheck $(OBJS) $(LDFLAGS)

fprand:	fprand.c
	cc -s -O -o fprand fprand.c

fplegal:	fprand.c
	cc -DLEGAL_ONLY -s -O -o fplegal fprand.c

fpcheck.mx: $(SOBJS)
	make $(PLATFORM)link

fprun: fprun.c
	cc -o fprun $(CFLAGS) fprun.c

SUNlink: tohex
	ld -e _$(ENTRY) -T $(TEXTSTRT) $(SOBJS) -lc
	nm a.out >memmap
	./tohex -s a.out > fpcheck.mx
	rm -f a.out

SYSVlink:
	echo "MEMORY {					" >ifile
	echo " valid : org = 0x$(TEXTSTRT), len = 0x100000">>ifile
	echo "}						">>ifile
	echo "						">>ifile
	echo "SECTIONS {				">>ifile
	echo "        .text : {}			">>ifile
	echo "        .data : {}			">>ifile
	echo "        .bss  : {}			">>ifile
	echo "}						">>ifile
	ld ifile $(SOBJS) -lc -m > memmap
	./ubuilds a.out; mv a.out.mx fpcheck.mx
	rm -f ifile a.out

fpcheck.o:	fpcheck.c template.h

fpchecks.o:	fpcheck.c template.h
	cp fpcheck.c fpchecks.c
	cc -c $(CFLAGS) -DSTANDALONE fpchecks.c
	-rm -f fpchecks.c

allops.o:	template.h extdefs.h body.c

drivers.s extdefs.h body.c:	makedrive $(TEMPLATES)
	./makedrive $(PLATFORM) <$(PLATFORM).$(LIST)

convert.s:	$(PLATFORM).CONVERT
	cp $(PLATFORM).CONVERT convert.s

makedrive:	makedrive.c
	cc -o makedrive makedrive.c

tohex:	tohex.c
	cc -o tohex tohex.c

ubuilds:	ubuilds.c
	cc -o ubuilds ubuilds.c -lld

install:	all
	cp $(TARGETS) $(INSDIR)

clean:
	rm -f $(TARGETS) $(OBJS) $(SOBJS)
	rm -f makedrive drivers.s extdefs.h body.c convert.s
	rm -f ubuilds tohex ifile memmap a.out

clobber:	clean

