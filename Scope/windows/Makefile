################################################################################
#
#  Makefile for Scope
#
################################################################################

GCC	  = gcc
GPLUSPLUS = g++
WINDRES   = windres

all:	Scope.exe

Scope.exe:	Scope.cpp Scope.o

Scope.o:	Scope.rc Scope.h Toolbar.bmp Scope.ico Scope.manifest Makefile

clean:
	rm *.exe *.o

%.exe:	%.cpp
	$(GPLUSPLUS) -o $@ $^ -Os -std=gnu++11 \
		-mwindows -lgdiplus -lcomctl32 -lwinmm

%.o:	%.rc
	$(WINDRES) -o $@ $<
