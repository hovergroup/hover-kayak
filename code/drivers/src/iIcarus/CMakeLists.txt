# ---------------------------------------------
# iIcarus
#        File: CMakeLists.txt
#  Created on: Mar 11, 2014
#      Author: Josh Leighton
# ---------------------------------------------
 
add_executable(iIcarus main.cpp icarus.cpp)

target_link_libraries(iIcarus
    boost_system
    boost_thread
    boost_date_time
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
    mbutil
    )