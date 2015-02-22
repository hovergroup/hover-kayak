# ---------------------------------------------
# pSmartCommsShoreside
#        File: CMakeLists.txt
#  Created on: 
#      Author: 
# ---------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
    SmartCommsShoreside.cpp
    SmartCommsShoreside_Info.cpp
    main.cpp
)



ADD_EXECUTABLE(pSmartCommsShoreside ${SRC})

TARGET_LINK_LIBRARIES(pSmartCommsShoreside
    mbutil
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
    )

