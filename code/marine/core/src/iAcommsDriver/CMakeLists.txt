# ---------------------------------------------
# iAcommsDriver
#        File: CMakeLists.txt
#  Created on: May 24, 2012
#      Author: Josh Leighton
# ---------------------------------------------

FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(iAcommsDriver ${SRC})

TARGET_LINK_LIBRARIES(iAcommsDriver
    HoverAcomms
    goby_common
    goby_acomms
    boost_system
    boost_signals
    protobuf
    geometry
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
    mbutil
    goby_util
    JoshUtils
    hovergroup_proto)

