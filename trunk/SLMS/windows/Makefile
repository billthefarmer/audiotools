################################################################################
#
#  Makefile for SLMS
#
################################################################################

GCC	= gcc
WINDRES = windres

all:	SLMS.exe

SLMS.exe:	SLMS.c SLMS.o

SLMS.o:	SLMS.rc SLMS.ico Knob.bmp SLMS.manifest

clean:
	rm *.exe *.o

%.exe:	%.c
	$(GCC) -o $@ $^ -g -march=opteron -ffast-math -std=gnu99 \
		-mwindows -lcomctl32 -lwinmm

%.o:	%.rc
	$(WINDRES) -o $@ $<
