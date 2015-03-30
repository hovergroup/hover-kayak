# ---------------------------------------------
# pAckedCommsShoreside
#        File: CMakeLists.txt
#  Created on: May 24, 2014
#      Author: Josh Leighton
# ---------------------------------------------

SET(SRC
    AckedCommsShoreside.cpp
    AckedCommsShoreside_Info.cpp
    main.cpp
)

ADD_EXECUTABLE(pAckedCommsShoreside ${SRC})

TARGET_LINK_LIBRARIES(pAckedCommsShoreside
    hovergroup_proto
    ${MOOS_LIBRARIES}
    mbutil
    m
    pthread)

