################################################################################
#
#  Makefile for Scope
#
################################################################################

GCC	= gcc
WINDRES = windres

all:	Scope.exe

Scope.exe:	Scope.c Scope.o

Scope.o:	Scope.rc Toolbar.bmp Scope.ico Scope.manifest

clean:
	rm *.exe *.o

%.exe:	%.c
	$(GCC) -o $@ $^ -g -march=opteron -ffast-math -std=gnu99 \
		-mwindows -lcomctl32 -lwinmm

%.o:	%.rc
	$(WINDRES) -o $@ $<
