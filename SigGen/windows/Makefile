################################################################################
#
#  Makefile for Siggen
#
################################################################################

GCC	= gcc
GXX     = g++
WINDRES = windres

all:	SigGen.exe

SigGen.exe:	SigGen.cpp SigGen.o

SigGen.o:	SigGen.rc SigGen.h SigGen.manifest SigGen.ico Makefile

clean:
	rm *.exe rm *.o

%.exe:	%.cpp
	$(GXX) -o $@ $^ -Os -std=gnu++11 -static \
		-mwindows -lgdiplus -lcomctl32 -lwinmm

%.o:	%.rc
	$(WINDRES) -o $@ $<
