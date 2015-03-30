##############################################################################
# FILE:        moos-ivp-extend/src/CMakeLists.txt
# DATE:        2010/09/07
# DESCRIPTION: CMakeLists.txt file for the moos-ivp-extend source directory
##############################################################################


#============================================================================
# Add the libraries in the current directory to the include path
#============================================================================
FILE(GLOB LOCAL_LIBRARY_DIRS ./lib_*)
INCLUDE_DIRECTORIES(${LOCAL_LIBRARY_DIRS}
                    "./protobuf")

#============================================================================
# List the subdirectories to build...
#============================================================================

ADD_SUBDIRECTORY(protobuf)

ADD_SUBDIRECTORY(lib_HoverGeometry)
ADD_SUBDIRECTORY(lib_behaviors)

ADD_SUBDIRECTORY(pMarinePID_Hover)
ADD_SUBDIRECTORY(pScheduledTransmit)
ADD_SUBDIRECTORY(pProtoReporter)
ADD_SUBDIRECTORY(pProtoReportParser)
ADD_SUBDIRECTORY(pNavManager)
ADD_SUBDIRECTORY(pResourceMon)
ADD_SUBDIRECTORY(pAckedCommsVehicle)
ADD_SUBDIRECTORY(iAcommsDriver)

IF(BUILD_SHORE_APPS)
    ADD_SUBDIRECTORY(pWTStatusView)
    ADD_SUBDIRECTORY(pAcommsSimulator)
    ADD_SUBDIRECTORY(uTermCommand_Hover)
    ADD_SUBDIRECTORY(pAckedCommsShoreside)
ENDIF(BUILD_SHORE_APPS)

##############################################################################
#                           END of CMakeLists.txt
##############################################################################

