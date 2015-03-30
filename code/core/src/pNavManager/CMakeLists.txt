# ---------------------------------------------
# pNavManager
#        File: CMakeLists.txt
#  Created on: Oct 18, 2013
#      Author: Josh Leighton
# ---------------------------------------------

SET(SRC
    NavManager.cpp
    NavManager_Info.cpp
    main.cpp
)

ADD_EXECUTABLE(pNavManager ${SRC})

TARGET_LINK_LIBRARIES(pNavManager
    geometry
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
    mbutil
    m
    pthread
    geometry)

