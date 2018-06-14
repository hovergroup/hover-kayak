#!/bin/bash
set -e

HELP=false
VEHICLE=false

for ARGI; do
    UNDEFINED_ARG=$ARGI
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
        HELP=true
        UNDEFINED_ARG=""
    elif [ "${ARGI}" = "--vehicle" ] ; then
        VEHICLE="true"
        UNDEFINED_ARG=""
    fi
done

if $HELP ; then
    printf "Usage: install-dependencies.sh [options]       \n"
    printf "Options:                                       \n"
    printf "  --vehicle     don't install gui dependencies \n"
    exit 1;
fi

sudo apt-get update

if $VEHICLE ; then
    sudo apt-get install build-essential libboost-all-dev libgsl0-dev cmake-curses-gui git-core libprotobuf-dev libprotoc-dev protobuf-compiler
else
    sudo apt-get install build-essential libboost-all-dev libgsl0-dev cmake-curses-gui git-core xterm libtiff5-dev freeglut3-dev libpng12-dev libxft-dev libxinerama-dev fluid default-jre libgmp3-dev witty witty-dev witty-doc witty-dbg
fi
