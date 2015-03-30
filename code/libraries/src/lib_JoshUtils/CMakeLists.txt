# ---------------------------------------------
# lib_JoshUtils
#        File: CMakeLists.txt
#  Created on: Nov 12, 2012
#      Author: Josh Leighton
# ---------------------------------------------

ADD_LIBRARY(JoshUtils SHARED
    JoshUtils.cpp)
   
TARGET_LINK_LIBRARIES(JoshUtils
    boost_system
    boost_filesystem
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
    mbutil
    logutils
    )