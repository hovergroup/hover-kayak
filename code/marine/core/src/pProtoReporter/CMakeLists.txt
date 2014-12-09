# ---------------------------------------------
# pProtoReporter
#        File: CMakeLists.txt
#  Created on: Sep 11, 2013
#      Author: Josh Leighton
# ---------------------------------------------

SET(SRC
    ProtoReporter.cpp
    ProtoReporter_Info.cpp
    main.cpp
)

ADD_EXECUTABLE(pProtoReporter ${SRC})

TARGET_LINK_LIBRARIES(pProtoReporter
   hovergroup_proto
   HoverGeometry
   mbutil
   boost_system
   ${MOOS_LIBRARIES}
   ${MOOS_DEPEND_LIBRARIES})

