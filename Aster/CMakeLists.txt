ADD_LIBRARY( aster STATIC
       AsterWriter.cpp AsterRunner.cpp MedWriter.cpp AsterModel.cpp
)

# Default value for MEDFile_DIR
IF(NOT DEFINED MEDFile_DIR)
    set(MEDFile_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cmakemodules/" CACHE PATH "MEDFile cmake config path")
ENDIF(NOT DEFINED MEDFile_DIR)
if(IS_DIRECTORY ${MEDFile_DIR})
    mark_as_advanced(MEDFile_DIR)
endif(IS_DIRECTORY ${MEDFile_DIR})

#works only in Linux, in windows always choose the static version
set(MEDFILE_USE_STATIC_LIBS ${STATIC_LINKING})
SET(HDF5_USE_STATIC_LIBRARIES ${STATIC_LINKING})
find_package(MEDFile CONFIG REQUIRED NO_DEFAULT_PATH)
target_include_directories(aster SYSTEM PRIVATE ${MEDFILE_INCLUDE_DIRS})
target_include_directories(aster SYSTEM PRIVATE ${HDF5_INCLUDE_DIRS})
target_include_directories(aster SYSTEM PRIVATE ${MPI_CXX_INCLUDE_PATH})
#link_directories ( ${MEDFILE_LIBRARY_DIR} )
list(APPEND EXTERNAL_LIBRARIES ${MEDFILE_C_LIBRARIES})
list(APPEND EXTERNAL_LIBRARIES ${MPI_C_LIBRARIES})
IF(NOT CYGWIN)
    # Needed in static linking
    list(APPEND EXTERNAL_LIBRARIES ${HDF5_LIBRARIES})
ENDIF(NOT CYGWIN)
mark_as_advanced(HDF5_DIR)

target_link_libraries( aster abstract ${EXTERNAL_LIBRARIES})
