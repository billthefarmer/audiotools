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
	$(GCC) -o $@ $^ -Os -std=gnu99 -mwin32 \
		-mwindows -lcomctl32 -lwinmm

%.o:	%.rc
	$(WINDRES) -o $@ $<
