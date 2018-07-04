# Find the libunwind library
#
#  LIBUNWIND_FOUND       - True if libunwind was found.
#  LIBUNWIND_LIBRARIES   - The libraries needed to use libunwind
#  LIBUNWIND_INCLUDE_DIR - Location of unwind.h and libunwind.h
 
FIND_PATH(LIBUNWIND_INCLUDE_DIR libunwind.h)
if(NOT LIBUNWIND_INCLUDE_DIR)
  message(FATAL_ERROR "failed to find libunwind.h")
elif(NOT EXISTS "${LIBUNWIND_INCLUDE_DIR}/unwind.h")
  message(FATAL_ERROR "libunwind.h was found, but unwind.h was not found in that directory.")
  SET(LIBUNWIND_INCLUDE_DIR "")
endif()

IF(LIBUNWIND_USE_STATIC_LIBS)
  IF(WIN32)
    FIND_LIBRARY(LIBUNWIND_GENERIC_LIBRARY unwind.lib unwind)
  ELSE(WIN32)
    FIND_LIBRARY(LIBUNWIND_GENERIC_LIBRARY libunwind.a unwind.a unwind)
  ENDIF(WIN32)
ELSE(LIBUNWIND_USE_STATIC_LIBS)
  FIND_LIBRARY(LIBUNWIND_GENERIC_LIBRARY libunwind.so unwind.so unwind)
ENDIF(LIBUNWIND_USE_STATIC_LIBS)
 

if (NOT LIBUNWIND_GENERIC_LIBRARY)
    MESSAGE(FATAL_ERROR "failed to find unwind generic library")
endif ()
SET(LIBUNWIND_LIBRARIES ${LIBUNWIND_GENERIC_LIBRARY})

# For some reason, we have to link to two libunwind shared object files:
# one arch-specific and one not.
if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    SET(LIBUNWIND_ARCH "arm")
elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "amd64")
    SET(LIBUNWIND_ARCH "x86_64")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "^i.86$")
    SET(LIBUNWIND_ARCH "x86")
endif()

if (LIBUNWIND_ARCH)
    IF(LIBUNWIND_USE_STATIC_LIBS)
      IF(WIN32)
        FIND_LIBRARY(LIBUNWIND_SPECIFIC_LIBRARY "unwind-${LIBUNWIND_ARCH}.lib" "unwind-${LIBUNWIND_ARCH}")
      ELSE(WIN32)
        FIND_LIBRARY(LIBUNWIND_SPECIFIC_LIBRARY "libunwind-${LIBUNWIND_ARCH}.a" "unwind-${LIBUNWIND_ARCH}.a" "unwind-${LIBUNWIND_ARCH}")
      ENDIF(WIN32)
    ELSE(LIBUNWIND_USE_STATIC_LIBS)
      FIND_LIBRARY(LIBUNWIND_SPECIFIC_LIBRARY "libunwind-${LIBUNWIND_ARCH}.so" "unwind-${LIBUNWIND_ARCH}.so" "unwind-${LIBUNWIND_ARCH}")
    ENDIF(LIBUNWIND_USE_STATIC_LIBS)
    if (NOT LIBUNWIND_SPECIFIC_LIBRARY)
        MESSAGE(FATAL_ERROR "failed to find unwind-${LIBUNWIND_ARCH}")
    endif ()
    SET(LIBUNWIND_LIBRARIES ${LIBUNWIND_LIBRARIES} ${LIBUNWIND_SPECIFIC_LIBRARY})
endif(LIBUNWIND_ARCH)

# some versions of libunwind need liblzma, and we don't use pkg-config
# so we just look whether liblzma is installed, and add it if it is.
# It might not be actually needed, but doesn't hurt if it is not.
# We don't need any headers, just the lib, as it's privately needed.
message(STATUS "looking for liblzma")
IF(LIBUNWIND_USE_STATIC_LIBS)
  IF(WIN32)
    find_library(LIBLZMA_LIBRARIES lzma.lib lzma )
  ELSE(WIN32)
    find_library(LIBLZMA_LIBRARIES liblzma.a lzma.a lzma )
  ENDIF(WIN32)
ELSE(LIBUNWIND_USE_STATIC_LIBS)
  find_library(LIBLZMA_LIBRARIES liblzma.so lzma.so lzma )
ENDIF(LIBUNWIND_USE_STATIC_LIBS)

if(NOT LIBLZMA_LIBRARIES STREQUAL "LIBLZMA_LIBRARIES-NOTFOUND")
  message(STATUS "liblzma found")
  set(LIBUNWIND_LIBRARIES "${LIBUNWIND_LIBRARIES};${LIBLZMA_LIBRARIES}")
endif()

if(LIBUNWIND_INCLUDE_DIR AND LIBUNWIND_GENERIC_LIBRARY AND LIBUNWIND_SPECIFIC_LIBRARY)
    SET(LIBUNWIND_FOUND ON)
endif()

message(STATUS "Found libunwind libraries:" ${LIBUNWIND_LIBRARIES})

MARK_AS_ADVANCED(LIBUNWIND_LIBRARIES LIBUNWIND_INCLUDE_DIR)
