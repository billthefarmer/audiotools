################################################################################
#
#  Makefile for SLMS
#
################################################################################

GCC	= gcc
GXX     = g++
WINDRES = windres

all:	SLMS.exe

SLMS.exe:	SLMS.cpp SLMS.o

SLMS.o:	SLMS.rc SLMS.ico SLMS.h SLMS.manifest Makefile

clean:
	rm *.exe *.o

%.exe:	%.cpp
	$(GXX) -o $@ $^ -Os -std=gnu++11 -static \
		-mwindows -lgdiplus -lcomctl32 -lwinmm

%.o:	%.rc
	$(WINDRES) -o $@ $<
