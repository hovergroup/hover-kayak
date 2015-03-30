# ---------------------------------------------
# pAcommsSimulator
#        File: CMakeLists.txt
#  Created on: Jan 13, 2014
#      Author: Josh Leighton
# ---------------------------------------------

SET(SRC
    AcommsSimulator.cpp
    AcommsSimulator_Info.cpp
    main.cpp
)

ADD_EXECUTABLE(pAcommsSimulator ${SRC})

TARGET_LINK_LIBRARIES(pAcommsSimulator
    hovergroup_proto
    goby_acomms
    ${MOOS_LIBRARIES}
    mbutil
    m
    pthread)

