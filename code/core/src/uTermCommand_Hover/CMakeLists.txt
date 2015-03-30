# ---------------------------------------------
# uTermCommand_Hover
#        File: CMakeLists.txt
#  Created on: May 19, 2014
#      Author: Mike Benjamin, Josh Leighton
# ---------------------------------------------

SET(SRC 
    main.cpp
    TermCommand.cpp
    TermCommand_Info.cpp)

ADD_EXECUTABLE(uTermCommand_Hover ${SRC})

TARGET_LINK_LIBRARIES(uTermCommand_Hover
    ${MOOS_LIBRARIES}
    mbutil
    genutil
    m
    pthread)

