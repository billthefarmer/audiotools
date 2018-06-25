################################################################################
#
#  Makefile for LMS
#
################################################################################

GCC	= gcc
GXX     = g++
WINDRES = windres

all:	LMS.exe

LMS.exe:	LMS.cpp LMS.o

LMS.o:	LMS.rc LMS.ico LMS.manifest LMS.h Makefile

clean:
	rm *.exe *.o

%.exe:	%.cpp
	$(GXX) -o $@ $^ -Os -std=gnu++11 -static \
		-mwindows -lgdiplus -lcomctl32 -lwinmm

%.o:	%.rc
	$(WINDRES) -o $@ $<
