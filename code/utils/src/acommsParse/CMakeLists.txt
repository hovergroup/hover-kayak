# ---------------------------------------------
# acommsParse
#        File: CMakeLists.txt
#  Created on: May 23, 2013
#      Author: Josh Leighton
# ---------------------------------------------

SET(SRC acommsParse.cpp)

ADD_EXECUTABLE(acommsParse ${SRC})
   
TARGET_LINK_LIBRARIES(acommsParse
    goby_common
    goby_acomms
    protobuf
    )
