# ---------------------------------------------
# lib_HoverGeometry
#        File: CMakeLists.txt
#  Created on: Apr 14, 2014
#      Author: Josh Leighton
# ---------------------------------------------

ADD_LIBRARY(HoverGeometry SHARED
    HoverGeometry.cpp)
   
TARGET_LINK_LIBRARIES(HoverGeometry
    protobuf
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
    mbutil
    geometry
    )