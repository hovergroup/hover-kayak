# ---------------------------------------------
# pProtoReportParser
#        File: CMakeLists.txt
#  Created on: Mar 20, 2014
#      Author: Josh Leighton
# ---------------------------------------------

SET(SRC
    ResourceMon.cpp
    ResourceMon_Info.cpp
    main.cpp
)

ADD_EXECUTABLE(pResourceMon ${SRC})

TARGET_LINK_LIBRARIES(pResourceMon
    ${MOOS_LIBRARIES}
    mbutil
    m
    pthread
    JoshUtils
    boost_system)

