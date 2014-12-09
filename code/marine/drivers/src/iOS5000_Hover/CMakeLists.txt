# ---------------------------------------------
# iOS5000_Hover
#        File: CMakeLists.txt
#  Created on: Aug 9, 2012
#      Author: Josh Leighton
# ---------------------------------------------

add_executable(iOS5000_Hover main.cpp iOS5000.cpp)

target_link_libraries(iOS5000_Hover
    boost_system
    boost_thread
    boost_date_time
    mbutil
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES})