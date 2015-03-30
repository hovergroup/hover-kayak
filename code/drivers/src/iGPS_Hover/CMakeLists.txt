# ---------------------------------------------
# iGPS_Hover
#        File: CMakeLists.txt
#  Created on: Jul 24, 2012
#      Author: Josh Leighton
# ---------------------------------------------

add_executable(iGPS_Hover main.cpp simple_gps.cpp iGPS_Hover_nmea.cpp)

target_link_libraries(iGPS_Hover
    boost_system
    boost_thread
    boost_date_time
    ${MOOSGeodesy_LIBRARIES}
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
    mbutil
    )