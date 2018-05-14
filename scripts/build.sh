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
    sudo apt update
    sudo apt install mingw-w64
    cd LMS/windows
    make GCC=i586-mingw32msvc-gcc WINDRES=i586-mingw32msvc-windres
    cd ../../Scope/windows
    make GPLUSPLUS=x86_64-w64-mingw32-g++ WINDRES=x86_64-w64-mingw32-windres
    cd ../../SigGen/windows
    make GPLUSPLUS=x86_64-w64-mingw32-g++ WINDRES=x86_64-w64-mingw32-windres
    cd ../../SLMS/windows
    make GCC=i586-mingw32msvc-gcc WINDRES=i586-mingw32msvc-windres
fi
