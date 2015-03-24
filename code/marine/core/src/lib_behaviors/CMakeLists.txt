# ---------------------------------------------
# lib_HoverAcomms
#        File: CMakeLists.txt
#  Created on: Aug 17, 2014
#      Author: Josh Leighton
# ---------------------------------------------

ADD_LIBRARY(BHV_Waypoint_Hover SHARED BHV_Waypoint_Hover.cpp)

TARGET_LINK_LIBRARIES(BHV_Waypoint_Hover
    mbutil
    behaviors
    geometry
    )

