# ---------------------------------------------
# iKST
#        File: CMakeLists.txt
#  Created on: Aug 12, 2013
#      Author: Josh Leighton
# ---------------------------------------------

SET(SRC
    KST.cpp
    KST_Info.cpp
    main.cpp
    )

ADD_EXECUTABLE(iKST ${SRC})

TARGET_LINK_LIBRARIES(iKST
    ${MOOS_LIBRARIES}
    mbutil
    m
    pthread
    )

