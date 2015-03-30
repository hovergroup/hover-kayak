# ---------------------------------------------
# uPokeDBHex
#        File: CMakeLists.txt
#  Created on: Jul 19, 2012
#      Author: Mike Benjamin, Josh Leighton
# ---------------------------------------------

FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(uPokeDBHex ${SRC})

TARGET_LINK_LIBRARIES(uPokeDBHex
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
    mbutil
    m
    pthread)

