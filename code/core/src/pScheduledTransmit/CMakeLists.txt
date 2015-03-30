# ---------------------------------------------
# pScheduledTransmit
#        File: CMakeLists.txt
#  Created on: Apr 9, 2013
#      Author: Josh Leighton
# ---------------------------------------------

FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(pScheduledTransmit ${SRC})

TARGET_LINK_LIBRARIES(pScheduledTransmit
    boost_date_time
    mbutil
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES})

