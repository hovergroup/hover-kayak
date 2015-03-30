# ---------------------------------------------
# pSmartCommsVehicle
#        File: CMakeLists.txt
#  Created on: 
#      Author: 
# ---------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
    SmartCommsVehicle.cpp
    SmartCommsVehicle_Info.cpp
    main.cpp
)



ADD_EXECUTABLE(pSmartCommsVehicle ${SRC})

TARGET_LINK_LIBRARIES(pSmartCommsVehicle
    mbutil
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
    )

