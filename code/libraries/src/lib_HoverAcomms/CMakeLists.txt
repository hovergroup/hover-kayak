# ---------------------------------------------
# lib_HoverAcomms
#        File: CMakeLists.txt
#  Created on: Jan 3, 2013
#      Author: Josh Leighton
# ---------------------------------------------

ADD_LIBRARY(HoverAcomms SHARED
    HoverAcomms.cpp)
   
TARGET_LINK_LIBRARIES(HoverAcomms
    goby_common
    goby_acomms
    protobuf
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
    mbutil
    )