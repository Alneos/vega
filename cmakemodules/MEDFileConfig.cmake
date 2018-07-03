# - Config file for the MEDFile package
# It defines the following variables. 
# Specific to the pacakge MEDFile itself:
#  MEDFILE_INCLUDE_DIRS - include directories 
#  MEDFILE_LIBRARIES    - libraries to link against (C and Fortran)
#  MEDFILE_C_LIBRARIES  - C libraries only
#  MEDFILE_EXTRA_LIBRARIES 
#  MEDFILE_ROOT_DIR_EXP - the root path of the installation providing this CMake file
#
# Other stuff specific to this package:
#  1. Dependencies of MEDFILE that might be re-used by dependent modules:
#   HDF5_ROOT_DIR_EXP  - path to the HDF5 installation used by MEDFile
#   MPI_ROOT_DIR_EXP   - path to the MPI installation used by MEDFile
#  2. Some flags
#   MEDFILE_USE_MPI   - boolean indicating if med fichier was compiled with MPI support

### Initialisation performed by CONFIGURE_PACKAGE_CONFIG_FILE:

####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was MEDFileConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

### First the generic stuff for a standard module:

message("Configuring MED dependencies")

SET(MEDFILE_INCLUDE_DIRS "${PACKAGE_PREFIX_DIR}/include")
MARK_AS_ADVANCED(MEDFILE_INCLUDE_DIRS)

# Options exported by the package:
SET(MEDFILE_USE_MPI OFF)
MARK_AS_ADVANCED(MEDFILE_USE_MPI)
SET(HDF5_ENABLE_PARALLEL OFF)
MARK_AS_ADVANCED(HDF5_ENABLE_PARALLEL)
SET(MEDFILE_VERSION 3.0.7)
SET(MEDFILE_USE_STATIC_LIBS ON)
MARK_AS_ADVANCED(MEDFILE_USE_STATIC_LIBS)

IF(MEDFILE_USE_STATIC_LIBS)
  IF(WIN32)
    FIND_LIBRARY(MEDFILE_C_LIBRARIES NAMES medC.lib medC)
  ELSE(WIN32)
    FIND_LIBRARY(MEDFILE_C_LIBRARIES NAMES libmedC.a medC)    
  ENDIF(WIN32)
ELSE(MEDFILE_USE_STATIC_LIBS)
  FIND_LIBRARY(MEDFILE_C_LIBRARIES NAMES libmedC.so medC)
ENDIF(MEDFILE_USE_STATIC_LIBS)

#IF(MEDFILE_USE_STATIC_LIBS)
#  IF(WIN32)
#    SET(MEDFILE_C_LIBRARIES libmedC.lib)
#    SET(MEDFILE_EXTRA_LIBRARIES libmedimportengine.lib)
#  ELSE(WIN32)
#    SET(MEDFILE_C_LIBRARIES libmedC.a)
#    SET(MEDFILE_EXTRA_LIBRARIES libmedimportengine.a)
#  ENDIF(WIN32)
#ELSE(MEDFILE_USE_STATIC_LIBS)
#  SET(MEDFILE_C_LIBRARIES medC)
#  SET(MEDFILE_EXTRA_LIBRARIES medimportengine)
#ENDIF()
#SET(MEDFILE_C_LIBRARIES medC)
#SET(MEDFILE_EXTRA_LIBRARIES medimportengine)

# For now we don't expose Fortran bindings:
SET(MEDFILE_LIBRARIES ${MEDFILE_C_LIBRARIES})
MARK_AS_ADVANCED(MEDFILE_LIBRARIES)
MESSAGE(STATUS "MEDFILE_LIBRARIES: " ${MEDFILE_LIBRARIES})

# If HDF5 was found in CONFIG mode, we need to include its targets so that
# dependent projects can compile
IF(HDF5_USE_STATIC_LIBRARIES)
find_library(HDF5_C_LIBRARY_z NAMES libz.a z)
ENDIF(HDF5_USE_STATIC_LIBRARIES)
FIND_PACKAGE(HDF5)

IF(NOT WIN32 AND MEDFILE_USE_MPI)
  FIND_PACKAGE(MPI)
ENDIF()

# Package root dir:
SET_AND_CHECK(MEDFILE_ROOT_DIR_EXP "${PACKAGE_PREFIX_DIR}") 

