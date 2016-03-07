#!/bin/bash
set -ev
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    sudo apt-get update -qq
    sudo apt-get install -y mingw32
fi
