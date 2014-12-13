#!/bin/bash 

# modify path
# PATH=$PATH:/home/student/moos-ivp-jleight/ivp-extend/trunk/bin

# source parameters
MISSIONS_HOME="../.."
source ${MISSIONS_HOME}/trunk/config/hard_config
source ${MISSIONS_HOME}/trunk/config/soft_config

# set defaults
HELP="no"
JUST_BUILD="no"
BAD_ARGS=""
VNAME=""
ALTIMETER=""
SIMULATION=false
TRANSMIT_PERIOD="27"

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
    if [ "${ARGI}" = "--icarus" ] ; then
        VNAME="ICARUS"
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
#  Part 2: Handle Ill-formed command-line arguments
#-------------------------------------------------------

if [ "${ALTIMETER}" != "" ] ; then
    if [ "${ALTIMETER}" != "tritech" ] && [ "${ALTIMETER}" != "cruzpro" ] ; then
        printf "Invalid altimeter option\n"
    else
    printf "Using %s altimeter\n" $ALTIMETER
    fi
fi

if [ "${HELP}" = "yes" ]; then
    printf "%s [SWITCHES]            \n" $0
    printf "Switches:                \n"
    printf "  --nostromo                     nostromo vehicle only  \n"
    printf "  --silvana                      silvana vehicle only   \n"
    printf "  --icarus                       icarus vehicle only    \n"
    printf "  --kestrel                      kestrel vehicle only   \n"
    printf "  --altimeter=tritech/cruzpro    enable specfied depths sounder\n"
    printf "  --just_build, -j       \n" 
    printf "  --help, -h             \n" 
    printf "  --sim             \n" 
    exit 0;
fi

if [ "${VNAME}" = "" ] ; then
    printf "Must specify a vehicle name. \n"
    exit 0
fi

if [ "${BAD_ARGS}" != "" ] ; then
    printf "Bad Argument: %s \n" $BAD_ARGS
    exit 0
fi

#-------------------------------------------------------
#  Part 3: Create the .moos and .bhv files. 
#-------------------------------------------------------

# if in simulation, change some configuration
if $SIMULATION ; then
    HARD_CONFIG["NOSTROMO:VHOST"]="localhost"
    HARD_CONFIG["SILVANA:VHOST"]="localhost"
    HARD_CONFIG["KESTREL:VHOST"]="localhost"
    VHOST_ICARUS="localhost"
    SHOREHOST="localhost"
fi

# Conditionally Prepare icarus files
if $SIMULATION ; then
    if [ "${VNAME}" = "ICARUS" ] ; then
        nsplug meta_icarus_sim.moos targ_$VNAME.moos -f           \
            VHOST=$VHOST_ICARUS                                   \
            VNAME=$VNAME_ICARUS                                   \
            VPORT=$VPORT_ICARUS                                   \
            LPORT=$LPORT_ICARUS                                   \
            WARP="1"                                              \
            ACOMMSID=$ACOMMSID_ICARUS                             \
            SHOREHOST=$SHOREHOST                                  \
            SLPORT=$SLPORT                        
    else
        nsplug meta_vehicle_sim.moos targ_$VNAME.moos -f           \
            VNAME=${HARD_CONFIG["${VNAME}:VNAME"]}                 \
            VHOST=${HARD_CONFIG["${VNAME}:VHOST"]}                 \
            VPORT=${HARD_CONFIG["${VNAME}:VPORT"]}                 \
            LPORT=${HARD_CONFIG["${VNAME}:LPORT"]}                 \
            ACOMMSID=${SOFT_CONFIG["${VNAME}:ACOMMSID"]}           \
            WARP="1"                                               \
            SHOREHOST=$SHOREHOST                                   \
            SLPORT=$SLPORT                                         \
            RETURN_PT=${SOFT_CONFIG["${VNAME}:RETURN_PT"]}
            
        nsplug meta_vehicle.bhv targ_$VNAME.bhv -f                 \
            VNAME=${HARD_CONFIG["${VNAME}:VNAME"]}                 \
            CRUISESPEED=${SOFT_CONFIG["${VNAME}:SPEED"]}           \
            RETURN_PT=${SOFT_CONFIG["${VNAME}:RETURN_PT"]}
    fi
else
    if [ "${VNAME}" = "ICARUS" ] ; then
        nsplug meta_icarus.moos targ_$VNAME.moos -f               \
            VHOST=$VHOST_ICARUS                                   \
            VNAME=$VNAME_ICARUS                                   \
            VPORT=$VPORT_ICARUS                                   \
            LPORT=$LPORT_ICARUS                                   \
            WARP="1"                                              \
            ACOMMSID=$ACOMMSID_ICARUS                             \
            MODEMPORT=$MODEMPORT_ICARUS                           \
            ARDUINO_PORT=$ARDUINOPORT_ICARUS                      \
            SHOREHOST=$SHOREHOST                                  \
            TransmitOffset=$TRANSMIT_OFFSET_ICARUS                \
            TransmitPeriod=$TRANSMIT_PERIOD                       \
            SLPORT=$SLPORT                                        
    else
        nsplug meta_vehicle_fld_rtk.moos targ_$VNAME.moos -f       \
            VNAME=${HARD_CONFIG["${VNAME}:VNAME"]}                 \
            VHOST=${HARD_CONFIG["${VNAME}:VHOST"]}                 \
            VPORT=${HARD_CONFIG["${VNAME}:VPORT"]}                 \
            LPORT=${HARD_CONFIG["${VNAME}:LPORT"]}                 \
            MODEMPORT=${HARD_CONFIG["${VNAME}:MODEMPORT"]}         \
            TRITECHPORT=${HARD_CONFIG["${VNAME}:TRITECHPORT"]}     \
            OS5000PORT=${HARD_CONFIG["${VNAME}:OS5000PORT"]}       \
            ARDUINO_PORT=${HARD_CONFIG["${VNAME}:ARDUINOPORT"]}    \
            ACOMMSID=${SOFT_CONFIG["${VNAME}:ACOMMSID"]}           \
            RUDDER_OFFSET=${SOFT_CONFIG["${VNAME}:RUDDER_OFFSET"]} \
            TransmitOffset=${SOFT_CONFIG["${VNAME}:TRANSMIT_OFFSET"]} \
            TransmitPeriod=$TRANSMIT_PERIOD                           \
            ALTIMETER=$ALTIMETER                                   \
            WARP="1"                                               \
            SHOREHOST=$SHOREHOST                                   \
            SLPORT=$SLPORT
            
        nsplug meta_vehicle.bhv targ_$VNAME.bhv -f                 \
            VNAME=${HARD_CONFIG["${VNAME}:VNAME"]}                 \
            CRUISESPEED=${SOFT_CONFIG["${VNAME}:SPEED"]}           \
            RETURN_PT=${SOFT_CONFIG["${VNAME}:RETURN_PT"]}
    fi
fi
    
if [ "${JUST_BUILD}" = "yes" ] ; then
    exit 0
fi

#-------------------------------------------------------
#  Part 4: Launch the processes
#-------------------------------------------------------

MOOSDATE=$(date +%-d_%-m_%Y)
FILEDATE=$(date +%-d_%-m_%Y_____%H_%M_%S)

DIRECTORY_NAME="CONSOLE_${VNAME}_${MOOSDATE}"
mkdir $HOME/logs/$DIRECTORY_NAME

# Launch
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

