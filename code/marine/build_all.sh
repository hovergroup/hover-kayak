#!/bin/bash
set -e

HELP=false
NUM_THREADS=1
VEHICLE=false

for ARGI; do
    UNDEFINED_ARG=$ARGI
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
        HELP=true
        UNDEFINED_ARG=""
    elif [ "${ARGI:0:2}" = "-j" ] ; then
        NUM_THREADS="${ARGI#-j*}"
        UNDEFINED_ARG=""
        START_DIRECTORY=${ARGI}
    elif [ "${ARGI}" = "--vehicle" -o "${ARGI}" = "-v" ] ; then
        VEHICLE="true"
        UNDEFINED_ARG=""
    else
        printf "Bad argument: $UNDEFINED_ARG \n"
        exit 1;
    fi
done

if $HELP ; then
    printf "Usage: build_all.sh [location] [-j4] [options]                     \n"
    printf "Options:                                                           \n"
    printf "  --distcc_host=[ip]     set distcc host (default 192.168.1.100)   \n"
    printf "  --vehicle, -v          install for vehicle (no gui or shore apps)\n"
    exit 1;
fi

cd libraries
cmake .
make -j$NUM_THREADS

cd ../core
./build_proto.sh
if $VEHICLE ; then
    cmake -D BUILD_SHORE_APPS=OFF .
else
    cmake -D BUILD_SHORE_APPS=ON .
fi
make -j$NUM_THREADS

cd ../drivers
cmake .
make -j$NUM_THREADS

cd ../utils
if $VEHICLE ; then
    cmake -D BUILD_SHORE_APPS=OFF .
else
    cmake -D BUILD_SHORE_APPS=ON .
fi
make -j$NUM_THREADS
