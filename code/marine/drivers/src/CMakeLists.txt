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

ADD_SUBDIRECTORY(iAltimeter)
ADD_SUBDIRECTORY(iAltimeter_cruzPro)
ADD_SUBDIRECTORY(iGPS_Hover)
ADD_SUBDIRECTORY(iHoverKayak)
ADD_SUBDIRECTORY(iIcarus)
ADD_SUBDIRECTORY(iNSFModem)
ADD_SUBDIRECTORY(iOS5000_Hover)
ADD_SUBDIRECTORY(iRTKNAVI)
ADD_SUBDIRECTORY(iXBee)

##############################################################################
#                           END of CMakeLists.txt
##############################################################################

