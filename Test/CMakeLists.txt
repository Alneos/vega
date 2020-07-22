
#if(NOT STATIC_LINKING)
    ADD_DEFINITIONS(-DBOOST_TEST_NO_LIB)
    ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)
    ADD_DEFINITIONS(-DBOOST_LIB_DIAGNOSTIC)
#endif(NOT STATIC_LINKING)

add_subdirectory(Abstract)
if(ENABLE_ASTER)
    add_subdirectory(Aster)
endif(ENABLE_ASTER)
add_subdirectory(Commandline)
add_subdirectory(Nastran)
add_subdirectory(Optistruct)
add_subdirectory(ResultReaders)
add_subdirectory(Systus)
add_subdirectory(NightlyTests)


