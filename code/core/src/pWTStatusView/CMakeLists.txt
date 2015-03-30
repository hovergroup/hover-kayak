# ---------------------------------------------
# pWTStatusView
#        File: CMakeLists.txt
#  Created on: Sep 11, 2013
#      Author: Josh Leighton
# ---------------------------------------------

SET(SRC
    WTStatusView.cpp
    WTStatusView_Info.cpp
    main.cpp
)

ADD_EXECUTABLE(pWTStatusView ${SRC})

TARGET_LINK_LIBRARIES(pWTStatusView
    wt
    wthttp
    boost_signals
    boost_thread
    boost_system
    hovergroup_proto
    mbutil
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES})

