################################################################################
#
#  Makefile for LMS
#
################################################################################

GCC	= gcc
WINDRES = windres

all:	LMS.exe Test.exe

LMS.exe:	LMS.c LMS.o

LMS.o:	LMS.rc LMS.ico LMS.manifest

clean:
	rm *.exe *.o

%.exe:	%.c
	$(GCC) -o $@ $^ -g -march=opteron -ffast-math -std=gnu99 \
		-mwindows -lcomctl32 -lwinmm

%.o:	%.rc
	$(WINDRES) -o $@ $<
