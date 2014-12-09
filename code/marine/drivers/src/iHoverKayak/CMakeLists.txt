# ---------------------------------------------
# iHoverKayak
#        File: CMakeLists.txt
#  Created on: Jun 20, 2012
#      Author: Josh Leighton
# ---------------------------------------------

add_executable(iHoverKayak main.cpp kayak_driver.cpp)

target_link_libraries(iHoverKayak
    boost_system
    boost_thread
    boost_date_time
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
    mbutil
    )