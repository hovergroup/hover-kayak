#!/bin/bash
set -e

GOBY_URL="http://bazaar.launchpad.net/~goby-dev/goby/2.0/"

HELP=false
DISTCC_HOST="192.168.1.100"
NUM_THREADS=1
INTERACTIVE=true
VEHICLE=false

ynprompt() {
    printf "$1 "
    read -n 1 -r
    if [[ $REPLY =~ ^[Yy]$ ]] ; then
        printf "\n"
        return 0
    else
        printf "\n"
        return 1
    fi
}

for ARGI; do
    UNDEFINED_ARG=$ARGI
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
        HELP=true
        UNDEFINED_ARG=""
    elif [ "${ARGI:0:2}" = "-j" ] ; then
        NUM_THREADS="${ARGI#-j*}"
        UNDEFINED_ARG=""
    elif [ "${ARGI:0:14}" = "--distcc_host=" ] ; then
        DISTCC_HOST="${ARGI#--distcc_host=*}"
        UNDEFINED_ARG=""
    elif [ "${ARGI}" = "--vehicle" -o "${ARGI}" = "-v" ] ; then
        VEHICLE="true"
        UNDEFINED_ARG=""
    else
        START_DIRECTORY=${ARGI}
    fi
done

if $HELP ; then
    printf "Usage: install_goby.sh [location] [-j4] [options]                  \n"
    printf "Options:                                                           \n"
    printf "  --distcc_host=[ip]     set distcc host (default 192.168.1.100)   \n"
    printf "  --vehicle, -v          install for vehicle (no gui or shore apps)\n"
    exit 1;
fi

# if directory does not exist, do fresh checkout
if [ ! -d $START_DIRECTORY/goby ] ; then
    bzr checkout $GOBY_URL $START_DIRECTORY/goby
fi
cd $START_DIRECTORY/goby

# pull out the bzr url
BZR_URL=$(bzr info | grep "checkout of branch" | sed 's/  checkout of branch: //g')

# check that we got a url - is this a bzr directory?
if [ -z "${BZR_URL}" ] ; then
    if $INTERACTIVE ; then
    if ! ynprompt "$START_DIRECTORY/goby is not a bzr directory, wipe and fresh checkout? [y/N]"; then
        exit 1
    fi
    fi
    
    # fresh checkout
    cd ..; rm -rf goby
    bzr checkout $GOBY_URL goby
    cd goby
fi

# check that url is correct
if [ ! "${BZR_URL}" = "${GOBY_URL}" ] ; then
    SWITCH=true
    if $INTERACTIVE ; then
    if ! ynprompt "BZR URL is $BZR_URL. \nWipe directory and switch to ${GOBY_URL}? [y/N]"; then
        SWITCH=false
    fi
    fi
    
    # do fresh checkout
    if $SWITCH ; then
    cd ..; rm -rf goby
    bzr checkout $GOBY_URL goby
    cd goby
    fi
fi

bzr up

if $COMPLETE_INSTALL ; then
    sudo ./DEPENDENCIES ubuntu
    sudo apt-get install libgmp3-dev
fi
cmake -D build_apps=OFF -D build_moos=OFF $VEHICLE_CMAKE_ARGS .
make -j$NUM_THREADS