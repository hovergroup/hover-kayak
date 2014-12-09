# ---------------------------------------------
# iNSFModem
#        File: CMakeLists.txt
#  Created on: May 7, 2013
#      Author: Josh Leighton, Pedro Vaz Teixeira
# ---------------------------------------------

SET(SRC
  NSFModem.cpp
  NSFModem_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(iNSFModem ${SRC})

TARGET_LINK_LIBRARIES(
  iNSFModem
  ${MOOS_LIBRARIES}   
  mbutil
  boost_thread
  boost_date_time
  boost_system
  )

