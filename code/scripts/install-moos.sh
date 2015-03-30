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
NUM_THREADS=1
INTERACTIVE=true
VEHICLE=false
ADD_PATH=false

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
    elif [ "${ARGI}" = "--vehicle" -o "${ARGI}" = "-v" ] ; then
        VEHICLE="true"
        UNDEFINED_ARG=""
    elif [ "${ARGI}" = "--addpath" -o "${ARGI}" = "-ap" ] ; then
        ADD_PATH=true
        UNDEFINED_ARG=""
    else
        START_DIRECTORY=${ARGI%/}
    fi
done

if [ $HELP = "true" ] || [ "$#" = 0 ] ; then
    printf "Usage: install_moos.sh [location] [-j4] [options]                  \n"
    printf "  A \"moos\" folder will be created in [location]                  \n"
    printf "Options:                                                           \n"
    printf "  --vehicle, -v          install for vehicle (no gui or shore apps)\n"
    printf "  --addpath, -ap         append binary locations to path in .bashrc\n"
    exit 1;
fi

# check start directory exists
if [ ! -d $START_DIRECTORY ] ; then
    printf "$START_DIRECTORY does not exist.\n"
    exit 1;
fi

cd $START_DIRECTORY
START_DIRECTORY=$(pwd)

if [ ! -d $START_DIRECTORY/moos ] ; then
    mkdir $START_DIRECTORY/moos
fi
cd $START_DIRECTORY/moos

for INDEX in {0..3} ; do
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
        ;;
        
    # essential
    1)  cmake -D MOOS_DIR="$START_DIRECTORY/moos/core-moos" .
        ;;
        
    # ui
    2)  if $VEHICLE ; then
            cmake -D MOOS_DIR="$START_DIRECTORY/moos/core-moos" -D BUILD_GRAPHICAL_TOOLS=OFF .
        else
            cmake -D MOOS_DIR="$START_DIRECTORY/moos/core-moos" -D BUILD_GRAPHICAL_TOOLS=ON .
        fi
        ;;
    
    # geodesy
    3)  cmake .
        ;;
        
    esac
    
    make -j$NUM_THREADS

    cd ..
done # for INDEX in {0..4}

if $ADD_PATH ; then
    if grep -Fxq "# Automatically generated by install_moos.sh script." ~/.bashrc ; then
        echo -e "\e[93mExisting path block detected in ~/.bashrc, no modifications will be made.\e[0m"
    else
        cp ~/.bashrc ~/.bashrc.bck
        printf "\n# Automatically generated by install_moos.sh script.\n" >> ~/.bashrc
        printf "PATH=\$PATH:$START_DIRECTORY/moos/core-moos/bin\n" >> ~/.bashrc
        printf "PATH=\$PATH:$START_DIRECTORY/moos/essential-moos/bin\n" >> ~/.bashrc
        printf "PATH=\$PATH:$START_DIRECTORY/moos/ui-moos/bin\n" >> ~/.bashrc
    fi
fi

echo -e "\e[93mDone.\e[0m"
