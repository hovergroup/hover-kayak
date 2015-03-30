# ---------------------------------------------
# pMarinePID_Hover
#        File: CMakeLists.txt
#  Created on: Aug 28, 2012
#      Author: Mike Benjamin, Josh Leighton
# ---------------------------------------------

SET(SRC
    MarinePID.cpp
    MarinePID_Info.cpp
    PIDEngine.cpp
    ScalarPID.cpp
    main.cpp
)

ADD_EXECUTABLE(pMarinePID_Hover ${SRC})
   
TARGET_LINK_LIBRARIES(pMarinePID_Hover
    geometry
    mbutil
    m
    pthread
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES})

