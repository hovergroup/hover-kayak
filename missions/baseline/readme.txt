Baseline Mission Configuration

Additional capabilities not setup in uTermCommand

1)  Constant Heading Behavior
    set MISSION_MODE = CONSTANT
    updates CONST_SPEED_UPDATES and CONST_HEADING_UPDATES
    
2)  Experimental nav source
    post "exp" to SET_NAV_SOURCE to get nav from EXP_*
    post anything else to reset