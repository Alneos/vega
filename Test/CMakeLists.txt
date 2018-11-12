
if(NOT STATIC_LINKING)
    ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)
endif(NOT STATIC_LINKING)

add_subdirectory(Abstract)
add_subdirectory(Aster)
add_subdirectory(Commandline)
add_subdirectory(Nastran)
add_subdirectory(Optistruct)
add_subdirectory(ResultReaders)
add_subdirectory(Systus)
add_subdirectory(NightlyTests)

 
