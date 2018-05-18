################################################################################
#
#  Makefile for LMS
#
################################################################################

GCC	  = gcc
GPLUSPLUS = g++
WINDRES   = windres

all:	LMS.exe

LMS.exe:	LMS.cpp LMS.o

LMS.o:	LMS.rc LMS.ico LMS.manifest LMS.h Makefile

clean:
	rm *.exe *.o

%.exe:	%.cpp
	$(GPLUSPLUS) -o $@ $^ -Os -std=gnu++11 \
		-mwindows -lgdiplus -lcomctl32 -lwinmm

%.o:	%.rc
	$(WINDRES) -o $@ $<
