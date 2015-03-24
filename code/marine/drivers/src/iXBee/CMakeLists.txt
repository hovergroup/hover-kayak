# ---------------------------------------------
# iXBee
#        File: CMakeLists.txt
#  Created on: 
#      Author: 
# ---------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
    XBee.cpp
    XBee_Info.cpp
    main.cpp
)



ADD_EXECUTABLE(iXBee ${SRC})

TARGET_LINK_LIBRARIES(iXBee
    boost_system
    boost_thread
    boost_date_time
    mbutil
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
    )

