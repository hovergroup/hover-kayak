# ---------------------------------------------
# iAltimeter
#        File: CMakeLists.txt
#  Created on: Aug 3, 2012
#      Author: Josh Leighton
# ---------------------------------------------

add_executable(iAltimeter main.cpp iAltimeter.cpp)

target_link_libraries(iAltimeter
    boost_system
    boost_thread
    boost_date_time
    mbutil
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES})