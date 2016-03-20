#!/bin/bash
set -ev
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    cd LMS/mac
    make
    cd ../../Scope/mac
    make
    cd ../../SigGen/mac
    make
    cd ../../SLMS/mac
    make
fi

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    cd LMS/windows
    make GCC=i586-mingw32msvc-gcc WINDRES=i586-mingw32msvc-windres
    cd ../../Scope/windows
    make GCC=i586-mingw32msvc-gcc WINDRES=i586-mingw32msvc-windres
    cd ../../SigGen/windows
    make GCC=i586-mingw32msvc-gcc WINDRES=i586-mingw32msvc-windres
    cd ../../SLMS/windows
    make GCC=i586-mingw32msvc-gcc WINDRES=i586-mingw32msvc-windres
fi
