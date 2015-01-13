#!/bin/bash
set -e

declare -A MOOS_INFO

MOOS_INFO["0:NAME"]="core"
MOOS_INFO["0:URL"]="https://github.com/themoos/core-moos.git"
MOOS_INFO["0:TAG"]="10.0.2-release"

MOOS_INFO["1:NAME"]="essential"
MOOS_INFO["1:URL"]="https://github.com/themoos/essential-moos.git"
MOOS_INFO["1:TAG"]="10.0.1-release"

MOOS_INFO["2:NAME"]="ui"
MOOS_INFO["2:URL"]="https://github.com/themoos/ui-moos.git"
MOOS_INFO["2:TAG"]=""

MOOS_INFO["3:NAME"]="geodesy"
MOOS_INFO["3:URL"]="https://github.com/themoos/geodesy-moos.git"
MOOS_INFO["3:TAG"]=""

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
    printf "Usage: install_moos.sh [location] [-j4] [options]                  \n"
    printf "Options:                                                           \n"
    printf "  --distcc_host=[ip]     set distcc host (default 192.168.1.100)   \n"
    printf "  --vehicle, -v          install for vehicle (no gui or shore apps)\n"
    exit 1;
fi
    
# check start directory exists
if [ ! -d $START_DIRECTORY ] ; then
    printf "$START_DIRECTORY does not exist.\n"
    exit 1;
fi

cd $START_DIRECTORY

for INDEX in {0..4} ; do
    # check for subdirectory
    SUBDIR="${MOOS_INFO["$INDEX:NAME"]}-moos"
    echo -e "\e[93mUpdating $SUBDIR\e[0m"
    if [ ! -d $SUBDIR ] ; then
        git clone ${MOOS_INFO["$INDEX:URL"]} $SUBDIR
    fi
    cd $SUBDIR
    
    # check git url
    GIT_URL=$(git config --get remote.origin.url)
    if [ ! "${GIT_URL}" = "${MOOS_INFO["$INDEX:URL"]}" ] ; then
        # if incorrect, wipe directory and recreate
        if $INTERACTIVE ; then
            if ! ynprompt "Wipe $START_DIRECTORY/moos/$SUBDIR? [y/N]"; then
                exit 1;
            fi
        fi
        cd ..
        rm -rf $SUBDIR
        git clone ${MOOS_INFO["$INDEX:URL"]} $SUBDIR
        cd $SUBDIR
    fi
    
    # check local modifications
    git update-index -q --refresh  
    CHANGED=$(git diff-index --name-only HEAD --)
    if [ -n "$CHANGED" ] ; then
        REVERT=true
        if $INTERACTIVE ; then
            if ! ynprompt "Overwrite local modifications in $START_DIRECTORY/moos/$SUBDIR? [y/N]"; then
                REVERT=false
            fi
        fi
        # revert local modifications
        if $REVERT ; then
            git checkout .
        fi
    fi
    
    # check tag
    if [ -z "${MOOS_INFO["$INDEX:TAG"]}" ] ; then
        # if no tag specified, just use master
        git checkout master
    else
        TAG=$(git describe --tags HEAD)
        if [ ! "${TAG}" = "${MOOS_INFO["$INDEX:TAG"]}" ] ; then
            # tag mismatch
            REVERT=true
            if $INTERACTIVE ; then
                if ! ynprompt "Reset to tag ${MOOS_INFO["$INDEX:TAG"]}? [y/N]"; then
                    REVERT=false
                fi
            fi
            if $REVERT ; then
                # check that tag exists
                TAG=$(git tag -l | grep ${MOOS_INFO["$INDEX:TAG"]})
                if [ -z "${TAG}" ] ; then
                    # if tag doesn't exist, try pulling the latest master
                    git checkout master
                    git pull
                    TAG=$(git tag -l | grep ${MOOS_INFO["$INDEX:TAG"]})
                    if [ -z "${TAG}" ] ; then
                        echo "Error: tag ${MOOS_INFO["$INDEX:TAG"]} does not exist."
                        exit 1;
                    fi
                fi
                git checkout tags/${MOOS_INFO["$INDEX:TAG"]}
            fi
        fi
    fi
    
    case "$INDEX" in
    # core
    0)  cmake -D DISABLE_NAMES_LOOKUP=ON -D USE_ASYNC_COMMS=ON .
        make -j$NUM_THREADS
        ;;
        
    # essential
    1)  cmake -D MOOS_DIR="$START_DIRECTORY/moos/core-moos" .
        make -j$NUM_THREADS
        ;;
        
    # ui
    2)  if $VEHICLE ; then
            cmake -D MOOS_DIR="$START_DIRECTORY/moos/core-moos" -D BUILD_GRAPHICAL_TOOLS=OFF .
        else
            cmake -D MOOS_DIR="$START_DIRECTORY/moos/core-moos" -D BUILD_GRAPHICAL_TOOLS=ON .
        fi
        make -j$NUM_THREADS
        ;;
    
    # geodesy
    3)  cmake .
        make -j$NUM_THREADS
        ;;
        
    esac
    
    cd ..
done # for INDEX in {0..4}