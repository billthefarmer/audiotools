################################################################################
#
#  Makefile for Scope
#
################################################################################

GCC	= gcc
GXX     = g++
WINDRES = windres

all:	Scope.exe

Scope.exe:	Scope.cpp Scope.o

Scope.o:	Scope.rc Scope.h Toolbar.bmp Scope.ico Scope.manifest Makefile

clean:
	rm *.exe *.o

%.exe:	%.cpp
	$(GXX) -o $@ $^ -Os -std=gnu++11 -static \
		-mwindows -lgdiplus -lcomctl32 -lwinmm

%.o:	%.rc
	$(WINDRES) -o $@ $<
