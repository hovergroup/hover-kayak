#!/bin/bash 

# modify path
# PATH=$PATH:/home/student/moos-ivp-jleight/ivp-extend/trunk/bin

# set defaults
HELP="no"
JUST_BUILD="no"
BAD_ARGS=""
VNAME=""
ALTIMETER="no_altimeter"
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
    if [ "${ARGI}" = "--nostromo" ] ; then
        VNAME="NOSTROMO"
        UNDEFINED_ARG=""
    fi
    if [ "${ARGI}" = "--silvana" ] ; then
        VNAME="SILVANA"
        UNDEFINED_ARG=""
    fi
    if [ "${ARGI}" = "--kestrel" ] ; then
        VNAME="KESTREL"
        UNDEFINED_ARG=""
    fi
    if [ "${ARGI:0:11}" = "--altimeter" ] ; then
        ALTIMETER="${ARGI#--altimeter=*}"
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

# check that altimeter option is valid
if [ "${ALTIMETER}" != "no_altimeter" ] ; then
    if [ "${ALTIMETER}" != "tritech" ] && [ "${ALTIMETER}" != "cruzpro" ] ; then
        printf "Invalid altimeter option\n"
        exit 0
    else
        printf "Using %s altimeter\n" $ALTIMETER
    fi
fi

# print help if requested or no arguments provided
if [ "${HELP}" = "yes" -o "${1}" = "" ]; then
    printf "%s [SWITCHES]      \n" $0
    printf "Switches:          \n"
    printf "  --nostromo                     nostromo vehicle only  \n"
    printf "  --silvana                      silvana vehicle only   \n"
    printf "  --kestrel                      kestrel vehicle only   \n"
    printf "  --altimeter=tritech/cruzpro    enable specfied depths sounder \n"
    printf "  --just_build, -j \n"
    printf "  --help, -h       \n"
    printf "  --sim            \n"
    exit 0;
fi

# check that a vehicle name has been provided
if [ "${VNAME}" = "" ] ; then
    printf "Must specify a vehicle name. \n"
    exit 0
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
    nsplug meta_vehicle_sim.moos targ_$VNAME.moos -f \
        $VNAME=1 $ALTIMETER=1 \
        VHOST=localhost SHOREHOST=localhost \
        SIMULATION=1
        
    nsplug meta_vehicle.bhv targ_$VNAME.bhv -f \
        $VNAME=1
else
    nsplug meta_vehicle_fld_rtk.moos targ_$VNAME.moos -f \
        $VNAME=1 $ALTIMETER=1
        
    nsplug meta_vehicle.bhv targ_$VNAME.bhv -f \
        $VNAME=1
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

