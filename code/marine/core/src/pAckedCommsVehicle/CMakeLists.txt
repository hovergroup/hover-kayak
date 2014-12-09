# ---------------------------------------------
# pAckedCommsVehicle
#        File: CMakeLists.txt
#  Created on: May 26, 2014
#      Author: Josh Leighton
# ---------------------------------------------

SET(SRC
    AckedCommsVehicle.cpp
    AckedCommsVehicle_Info.cpp
    main.cpp
)

ADD_EXECUTABLE(pAckedCommsVehicle ${SRC})

TARGET_LINK_LIBRARIES(pAckedCommsVehicle
    hovergroup_proto
    ${MOOS_LIBRARIES}
    mbutil
    m
    pthread)

