#!/bin/bash 

# modify path
# PATH=$PATH:/home/student/moos-ivp-jleight/ivp-extend/trunk/bin

# set defaults
HELP="no"
JUST_BUILD="no"
BAD_ARGS=""
SIMULATION=false

#-------------------------------------------------------
#  Part 1: Process command-line arguments
#-------------------------------------------------------

for ARGI; do
    UNDEFINED_ARG=$ARGI
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
        HELP="yes"
        UNDEFINED_ARG=""
    fi
    if [ "${ARGI}" = "--just_build" -o "${ARGI}" = "-j" ] ; then
        JUST_BUILD="yes"
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

#-------------------------------------------------------
#  Part 2: Verify input
#-------------------------------------------------------

# print help if requested or no arguments provided
if [ "${HELP}" = "yes" -o "${1}" = "" ]; then
    printf "%s [SWITCHES]      \n" $0
    printf "Switches:          \n"
    printf "  --just_build, -j \n"
    printf "  --help, -h       \n"
    printf "  --sim            \n"
    exit 0;
fi

# report any unhandled arguments
if [ "${BAD_ARGS}" != "" ] ; then
    printf "Bad Argument: %s \n" $BAD_ARGS
    exit 0
fi

#-------------------------------------------------------
#  Part 3: Create the .moos and .bhv files. 
#-------------------------------------------------------


# Conditionally prepare simulation or field files
if $SIMULATION ; then
    nsplug meta_icarus_sim.moos targ_ICARUS.moos -f \
        VHOST=localhost SHOREHOST=localhost
else
    nsplug meta_icarus.moos targ_ICARUS.moos -f
fi

# exit here of only building
if [ "${JUST_BUILD}" = "yes" ] ; then
    exit 0
fi

#-------------------------------------------------------
#  Part 4: Launch the processes
#-------------------------------------------------------

# create directory corresponding to current data
MOOSDATE=$(date +%-d_%-m_%Y)
FILEDATE=$(date +%-d_%-m_%Y_____%H_%M_%S)

DIRECTORY_NAME="CONSOLE_${VNAME}_${MOOSDATE}"
mkdir $HOME/logs/$DIRECTORY_NAME

# launch and log console to file
printf "Launching MOOS Community \n"
pAntler targ_$VNAME.moos >& $HOME/logs/$DIRECTORY_NAME/CONSOLE_$VNAME_$FILEDATE.clog &

#-------------------------------------------------------
#  Part 5: Exiting and/or killing the simulation
#-------------------------------------------------------

ANSWER="0"
while [ "${ANSWER}" != "1" -a "${ANSWER}" != "2" ]; do
    printf "Now what? (1) Exit script (2) Exit and Kill Simulation \n"
    printf "> "
    read ANSWER
done

# %1 matches the PID of the first job in the active jobs list, 
# namely the pAntler job launched in Part 4.
if [ "${ANSWER}" = "2" ]; then
    printf "Killing all processes ... \n "
    kill %1 
    printf "Done killing processes.   \n "
fi

