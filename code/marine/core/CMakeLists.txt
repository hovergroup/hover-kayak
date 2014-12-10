#===========================================================================
# FILE:  hovergroup/code/marine/core/CMakeLists.txt
# DATE:  2014/12/09
# INFO:  Top-level CMakeLists.txt file for the marine/core project
# NAME:  Maintained by Josh Leighton.  Original setup from 
#        moos-ivp-extend and written my Mike Benjamin
#===========================================================================

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT( IVP_EXTEND )

include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

SET(BUILD_SHORE_APPS TRUE CACHE BOOL "Build shore apps")

#=============================================================================
# Find the hovergroup trunk
#=============================================================================

find_path( HOVERGROUP_LIBRARIES
             NAMES src/lib_HoverAcomms
             PATHS "../libraries"
             NO_DEFAULT_PATH )

FILE(GLOB HOVERGROUP_LIB_DIRECTORIES 
       ${HOVERGROUP_LIBRARIES}/src/lib_*
       ${HOVERGROUP_LIBRARIES}/src/protobuf)

set ( HOVERGROUP_INCLUDE_DIRECTORIES
        ${HOVERGROUP_LIB_DIRECTORIES}
        ${HOVERGROUP_LIBRARIES}/src/ )
        
set ( HOVERGROUP_LIBRARY_PATH
        ${HOVERGROUP_LIBRARIES}/lib/ )

#=============================================================================
# Find the "goby" base directory
#=============================================================================

find_path( GOBY_BASE
             NAMES DEPENDENCIES build.sh AUTHORS
             PATHS "~/goby"
             NO_DEFAULT_PATH )

set( GOBY_INCLUDE_DIRS
       ${GOBY_BASE}/include )
    
set( GOBY_LIBRARY_PATH
       ${GOBY_BASE}/lib )

#=============================================================================
# Find the "moos-ivp" base directory
#=============================================================================

# Search for the moos-ivp folder
find_path( MOOSIVP_SOURCE_TREE_BASE
             NAMES build-ivp.sh build-moos.sh configure-ivp.sh
             PATHS "../moos-ivp" "../../moos-ivp" "../../moos-ivp/trunk/" "../moos-ivp/trunk/" "~/moos-ivp"
             DOC "Base directory of the MOOS-IvP source tree"
             NO_DEFAULT_PATH
)

if (NOT MOOSIVP_SOURCE_TREE_BASE)
    message("Please set MOOSIVP_SOURCE_TREE_BASE to  ")
    message("the location of the \"moos-ivp\" folder ")
    return()
endif()

GET_FILENAME_COMPONENT(MOOS_BASE_DIR "${MOOSIVP_SOURCE_TREE_BASE}/MOOS" ABSOLUTE)


#=============================================================================
# Specify where to find IvP's headers and libraries...
#=============================================================================

FILE(GLOB IVP_INCLUDE_DIRS ${MOOSIVP_SOURCE_TREE_BASE}/ivp/src/lib_* )

FILE(GLOB IVP_LIBRARY_DIRS ${MOOSIVP_SOURCE_TREE_BASE}/lib )

#=============================================================================
# Specify where to find MOOS's headers and libraries...
#=============================================================================

find_package(MOOS 10)
find_package(MOOSGeodesy)

LINK_DIRECTORIES(       ${IVP_LIBRARY_DIRS}
                        ${GOBY_LIBRARY_PATH}
                        ${HOVERGROUP_LIBRARY_PATH}    )

INCLUDE_DIRECTORIES(    ${MOOS_INCLUDE_DIRS}
                        ${MOOS_DEPEND_INCLUDE_DIRS}
                        ${MOOSGeodesy_INCLUDE_DIRS}
                        ${GOBY_INCLUDE_DIRS}
                        ${IVP_INCLUDE_DIRS}
                        ${HOVERGROUP_INCLUDE_DIRECTORIES}    )


#=============================================================================
# Set the output directories for the binary and library files
#=============================================================================

GET_FILENAME_COMPONENT(IVP_EXTEND_BIN_DIR "${CMAKE_SOURCE_DIR}/bin"  ABSOLUTE )
GET_FILENAME_COMPONENT(IVP_EXTEND_LIB_DIR "${CMAKE_SOURCE_DIR}/lib"  ABSOLUTE )

SET( LIBRARY_OUTPUT_PATH      "${IVP_EXTEND_LIB_DIR}" CACHE PATH "" )
SET( ARCHIVE_OUTPUT_DIRECTORY "${IVP_EXTEND_LIB_DIR}" CACHE PATH "" )
SET( LIBRARY_OUTPUT_DIRECTORY "${IVP_EXTEND_LIB_DIR}" CACHE PATH "" )

SET( EXECUTABLE_OUTPUT_PATH    "${IVP_EXTEND_BIN_DIR}" CACHE PATH "" )
SET( RUNTIME_OUTPUT_DIRECTORY "${IVP_EXTEND_BIN_DIR}"  CACHE PATH "" )


#=============================================================================
# Specify Compiler Flags
#=============================================================================
IF( ${WIN32} )
   #---------------------------------------------
   # Windows Compiler Flags
   #---------------------------------------------
   IF(MSVC)
      # Flags for Microsoft Visual Studio
      SET( WALL_ON OFF CACHE BOOL 
         "tell me about all compiler warnings (-Wall) ")
      IF(WALL_ON)
         SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
      ENDIF(WALL_ON)
   ELSE(MSVC)
      # Other Windows compilers go here
   ENDIF(MSVC)

ELSE( ${WIN32} )
   #---------------------------------------------
   # Linux and Apple Compiler Flags
   #---------------------------------------------
   # Force -fPIC because gcc complains when we don't use it with x86_64 code.
   # Note sure why: -fPIC should only be needed for shared objects, and
   # AFAIK, CMake gets that right when building shared objects. -CJC
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g")
   IF(CMAKE_COMPILER_IS_GNUCXX)
      # Flags for the GNU C++ Compiler
      SET( WALL_ON OFF CACHE BOOL 
         "tell me about all compiler warnings (-Wall) ")
      IF(WALL_ON)
         SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
      ENDIF( WALL_ON)
   ELSE(CMAKE_COMPILER_IS_GNUCXX)
    
   ENDIF(CMAKE_COMPILER_IS_GNUCXX)

ENDIF( ${WIN32} )


#=============================================================================
# Add Subdirectories
#=============================================================================
ADD_SUBDIRECTORY( src )

##############################################################################
#                           END of CMakeLists.txt
##############################################################################
