################################################################################
#
#  Makefile for Siggen
#
################################################################################

GCC	= gcc
WINDRES = windres

all:	SigGen.exe

SigGen.exe:	SigGen.c SigGen.o

SigGen.o:	SigGen.rc SigGen.manifest SigGen.ico Knob.bmp

clean:
	rm *.exe

%.exe:	%.c
	$(GCC) -o $@ $^ -O2 -march=opteron -ffast-math -std=gnu99 \
		-mwindows -lcomctl32 -lwinmm

%.o:	%.rc
	$(WINDRES) -o $@ $<
