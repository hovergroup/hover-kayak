#!/bin/bash
set -e

MOOS_IVP_URL="https://oceanai.mit.edu/svn/moos-ivp-aro/releases/moos-ivp-14.7.1"

NUM_THREADS=1
INTERACTIVE=true
HELP=false
VEHICLE=false
ADD_PATH=false
MOOS_PATH=""

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
    elif [ "${ARGI:0:10}" = "--moosdir=" ] ; then
        MOOS_PATH="${ARGI#--moosdir=*}"
        UNDEFINED_ARG=""
    else
        START_DIRECTORY=${ARGI%/}
    fi
done

if [ $HELP = "true" ] || [ "$#" = 0 ] ; then
    printf "Usage: hover_update [location] [-j4] [options]\n"
    printf "Options:                                      \n"
    printf "  --vehicle, -v          install for vehicle (no gui or shore apps)\n"
    printf "  --addpath, -ap         append binary locations to path in .bashrc\n"
    printf "  --moosdir=[directory]  specify path to moos directory (default   \n"
    printf "                         [location]/moos) \n"
    exit 1;
fi

if [ -z $MOOS_PATH ] ; then
    MOOS_PATH=$START_DIRECTORY/moos
fi

# check start directory exists
if [ ! -d $START_DIRECTORY ] ; then
    printf "$START_DIRECTORY does not exist.\n"
    exit 1;
fi

cd $START_DIRECTORY
START_DIRECTORY=$(pwd)

# check that MOOS install exists
if [ ! -d $MOOS_PATH/core-moos ] ; then
    printf "$MOOS_PATH/core-moos does not exist - where is MOOS?\n"
    exit 1;
fi

# reduced checkout on vehicle
if $VEHICLE ; then
    MOOS_IVP_URL=$MOOS_IVP_URL/ivp
fi

echo -e "\e[1;93mUpdating IVP\e[0m"

# if directory does not exist, do fresh checkout
if [ ! -d $START_DIRECTORY/moos-ivp ] ; then
    if $VEHICLE ; then
        mkdir $START_DIRECTORY/moos-ivp
        svn checkout $MOOS_IVP_URL $START_DIRECTORY/moos-ivp/ivp
    else
        svn checkout $MOOS_IVP_URL $START_DIRECTORY/moos-ivp
    fi
fi
if $VEHICLE ; then
    cd $START_DIRECTORY/moos-ivp/ivp
else
    cd $START_DIRECTORY/moos-ivp
fi

# pull out the svn url
SVN_URL=$(svn info | grep URL | sed -n 1p | sed 's/URL: //g')

# check that we got a url - is this an svn directory?
if [ -z "${SVN_URL}" ] ; then
    if $INTERACTIVE ; then
        currentdir=$(pwd)
        if ! ynprompt "$currentdir is not an svn directory, wipe and fresh checkout? [y/N]"; then
            exit 1
        fi
    fi
    
    # fresh checkout
    if $VEHICLE ; then
        cd ..; rm -rf ivp
        svn checkout $MOOS_IVP_URL ivp
        cd ivp
    else
        cd ..; rm -rf moos-ivp
        svn checkout $MOOS_IVP_URL moos-ivp
        cd moos-ivp
    fi
fi

# check that url is correct
if [ ! "${SVN_URL}" = "${MOOS_IVP_URL}" ] ; then
    SWITCH=true
    if $INTERACTIVE ; then
        if ! ynprompt "SVN URL is $SVN_URL. \nWipe directory and switch to ${MOOS_IVP_URL}? [y/N]"; then
            SWITCH=false
        fi
    fi
    
    # do fresh checkout
    if $SWITCH ; then
        if $VEHICLE ; then
            cd ..; rm -rf ivp
            svn checkout $MOOS_IVP_URL ivp
            cd ivp
        else
            cd ..; rm -rf moos-ivp
            svn checkout $MOOS_IVP_URL moos-ivp
            cd moos-ivp
        fi
    fi
fi

# update
SVN_VAL=$(svn up)
if [ -z "$SVN_VAL" ] ; then
    printf "ivp svn update failed"
    exit 1;
fi

# build
if $VEHICLE ; then
    cd src
    # make modifications to CMakeLists to avoid building as much
    COMMENT_LINES=(lib_ufield
                   lib_ufld_hazards
                   app_gen_hazards
                   pXRelay
                   pMarinePIDpBasicContactMgr
                   pNodeReporter
                   uFldCollisionDetect
                   uFldPathCheck
                   uFldShoreBroker
                   uFldMessageHandler
                   uFldNodeBroker
                   uFldScope
                   uFldNodeComms
                   uFldBeaconRangeSensor
                   uFldContactRangeSensor
                   uFldHazardSensor
                   uFldHazardMgr
                   uFldHazardMetric
                   uFldGenericSensor
                   uFldWrapDetect
                   pSearchGrid
                   uLoadWatch
                   uSimMarine
                   pHostInfo
                   iSay)
    for LINE in ${COMMENT_LINES[@]} ; do
        sed -i "s/\ $LINE/\ #$LINE/" CMakeLists.txt
    done
    cmake -D IVP_BUILD_GUI_CODE=OFF $VEHICLE_CMAKE_ARGS .
    make -j$NUM_THREADS
else
    cd ivp/src
    cmake -D IVP_BUILD_GUI_CODE=ON .
    make -j$NUM_THREADS
fi


if $ADD_PATH ; then
    if grep -Fxq "# Automatically generated by install_ivp.sh script." ~/.bashrc ; then
        echo -e "\e[93mExisting path block detected in ~/.bashrc, no modifications will be made.\e[0m"
    else
        cp ~/.bashrc ~/.bashrc.bck
        printf "\n# Automatically generated by install_ivp.sh script.\n" >> ~/.bashrc
        printf "PATH=\$PATH:$START_DIRECTORY/moos-ivp/ivp/bin\n" >> ~/.bashrc
    fi
fi

echo -e "\e[93mDone.\e[0m"
