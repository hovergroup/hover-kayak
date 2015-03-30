#!/bin/bash
set -e

GOBY_URL="http://bazaar.launchpad.net/~goby-dev/goby/2.0/"

HELP=false
NUM_THREADS=1
INTERACTIVE=true
INSTALL_DEPENDENCIES=false
DEPENDENCY_TYPE=""

ynprompt() {
    echo -e "\e[93m$1 \e[0m"
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
    elif [ "${ARGI}" = "--debian-dep" ] ; then
        INSTALL_DEPENDENCIES=true
        DEPENDENCY_TYPE=debian
    elif [ "${ARGI}" = "--ubuntu-dep" ] ; then
        INSTALL_DEPENDENCIES=true
        DEPENDENCY_TYPE=ubuntu
    else
        START_DIRECTORY=${ARGI%/}
    fi
done

if [ $HELP = "true" ] || [ "$#" = 0 ] ; then
    printf "Usage: install_goby.sh [location] [-j4] [options]   \n"
    printf "Options:                                            \n"
    printf "  --ubuntu-dep    install dependencies for ubuntu   \n"
    printf "  --debian-dep    install dependencies for debian   \n"
    exit 1;
fi

# if directory does not exist, do fresh checkout
if [ ! -d $START_DIRECTORY/goby ] ; then
    bzr checkout --lightweight $GOBY_URL $START_DIRECTORY/goby
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
    bzr checkout --lightweight $GOBY_URL goby
    cd goby
fi

# check that url is correct
if [ ! "${BZR_URL}" = " ${GOBY_URL}" ] ; then
    SWITCH=true
    if $INTERACTIVE ; then
    if ! ynprompt "BZR URL is $BZR_URL \nWipe directory and switch to ${GOBY_URL}? [y/N]"; then
        SWITCH=false
    fi
    fi
    
    # do fresh checkout
    if $SWITCH ; then
    cd ..; rm -rf goby
    bzr checkout --lightweight $GOBY_URL goby
    cd goby
    fi
fi

bzr up

if $INSTALL_DEPENDENCIES ; then
    sudo ./DEPENDENCIES $DEPENDENCY_TYPE
    sudo apt-get install libgmp3-dev
fi

cmake -D build_apps=OFF -D build_moos=OFF .
make -j$NUM_THREADS

echo -e "\e[93mDone.\e[0m"
