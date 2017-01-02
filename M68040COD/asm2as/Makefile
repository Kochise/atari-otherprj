#
#       Makefile for 680x0 cross assemblers
#

#		Copyright (C) Motorola, Inc. 1990
#			All Rights Reserved
#
#	For details on the license for this file, please see the
#	file, README.lic, in this same directory.

ASM2AS    = asm2as


OFILES = asm2as.o
CFLAGS = -O $(OPTIONS)

$(ASM2AS):  $(OFILES)
	cc $(CFLAGS) -s -o $(ASM2AS) $(OFILES)
	size $(ASM2AS)

asm2as.o:	asm2as.c mnemonics.h operands.h


clean:
	-rm -f $(ASM2AS) $(OFILES)

install:  $(ASM2AS)
	strip $(ASM2AS)
	cp $(ASM2AS) $(INSDIR)
