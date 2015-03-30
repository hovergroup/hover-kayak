# ---------------------------------------------
# pProtoReportParser
#        File: CMakeLists.txt
#  Created on: Sep 11, 2013
#      Author: Josh Leighton
# ---------------------------------------------

SET(SRC
    ProtoReportParser.cpp
    ProtoReportParser_Info.cpp
    main.cpp
)

ADD_EXECUTABLE(pProtoReportParser ${SRC})

TARGET_LINK_LIBRARIES(pProtoReportParser
    hovergroup_proto
    HoverGeometry
    mbutil
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
    contacts)

