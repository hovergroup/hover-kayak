# ---------------------------------------------
# uPokeRandom
#        File: CMakeLists.txt
#  Created on: May 12, 2013
#      Author: Mike Benjamin, Josh Leighton
# ---------------------------------------------

FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(uPokeRandom ${SRC})

TARGET_LINK_LIBRARIES(uPokeRandom
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
    mbutil
    m
    pthread)

