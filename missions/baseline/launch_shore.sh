#!/bin/bash

HELP=false
JUST_BUILD=false
BAD_ARGS=""
WARP=1
SIMULATION=false

#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
let COUNT=0
for ARGI; do
    UNDEFINED_ARG=$ARGI
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
    HELP=true
    UNDEFINED_ARG=""
    fi
    if [ "${ARGI}" = "--just_build" -o "${ARGI}" = "-j" ] ; then
    JUST_BUILD=true
    UNDEFINED_ARG=""
    fi
    if [ "${ARGI}" = "--sim" ] ; then
    SIMULATION=true
    UNDEFINED_ARG=""
    fi
    if [ "${UNDEFINED_ARG}" != "" ] ; then
    BAD_ARGS=$UNDEFINED_ARG
    fi
done

if [ "${BAD_ARGS}" != "" ] ; then
    printf "Bad Argument: %s \n" $BAD_ARGS
    exit 0
fi

if $HELP  ; then
    printf "%s [SWITCHES]            \n" $0
    printf "Switches:                \n"
    printf "  --just_build, -j       \n" 
    printf "  --help, -h             \n" 
    printf "  --sim                  \n"
    exit 0;
fi

#-------------------------------------------------------
#  Part 2: Create the shoreside.moos file
#-------------------------------------------------------

# if in simulation, change some configuration
if $SIMULATION ; then
    HARD_CONFIG["NOSTROMO:VHOST"]="localhost"
    HARD_CONFIG["SILVANA:VHOST"]="localhost"
    HARD_CONFIG["KESTREL:VHOST"]="localhost"
    VHOST_ICARUS="localhost"
    SHOREHOST="localhost"
fi

nsplug meta_shoreside.moos targ_shoreside.moos -f       \
    LPORT=$SLPORT      VPORT=$SPORT                     \
    VNAME=$SNAME       WARP=$WARP                       \
    VHOST=$SHOREHOST                                    

if $JUST_BUILD ; then
    exit 0
fi

#-------------------------------------------------------
#  Part 3: Launch the processes
#-------------------------------------------------------

MOOSDATE=$(date +%-d_%-m_%Y)
FILEDATE=$(date +%-d_%-m_%Y_____%H_%M_%S)

printf "Launching $SNAME MOOS Community \n"

DIRECTORY_NAME="CONSOLE_${SNAME}_${MOOSDATE}"
mkdir $HOME/logs/$DIRECTORY_NAME

pAntler targ_shoreside.moos >& $HOME/logs/$DIRECTORY_NAME/CONSOLE_$SNAME_$FILEDATE.clog &

#-------------------------------------------------------
#  Part 4: Exiting and/or killing the simulation
#-------------------------------------------------------

ANSWER="0"
while [ "${ANSWER}" != "1" -a "${ANSWER}" != "2" ]; do
    printf "Now what? (1) Exit script (2) Exit and Kill-All \n"
    printf "> "
    read ANSWER
done

# %1, matches the PID of the job in the active jobs list, 
# namely the pAntler job launched in Part 3.

if [ "${ANSWER}" = "2" ]; then
    printf "Killing all processes ... \n "
    kill %1 
    printf "Done killing processes.   \n "
fi
