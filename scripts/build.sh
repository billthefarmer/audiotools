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

