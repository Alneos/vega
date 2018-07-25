
ADD_LIBRARY( command_line_test_utils STATIC
       CommandLineUtils.cpp
)

add_executable(
 Nastran2Aster_test
 Nastran2Aster_test.cpp
)

SET_TARGET_PROPERTIES(Nastran2Aster_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(Nastran2Aster_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 Nastran2Aster_test
 command_line_test_utils
 commandline
)

add_test(NAME Nastran2Aster_test COMMAND $<TARGET_FILE:Nastran2Aster_test>)

add_executable(
 Nastran2Systus_test
 Nastran2Systus_test.cpp
)

SET_TARGET_PROPERTIES(Nastran2Systus_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(Nastran2Systus_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 Nastran2Systus_test
 command_line_test_utils
 commandline
)

add_test(NAME Nastran2Systus_test COMMAND $<TARGET_FILE:Nastran2Systus_test>)

add_executable(
 Nastran2Nastran_test
 Nastran2Nastran_test.cpp
)

SET_TARGET_PROPERTIES(Nastran2Nastran_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(Nastran2Nastran_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 Nastran2Nastran_test
 command_line_test_utils
 commandline
)

add_test(NAME Nastran2Nastran_test COMMAND $<TARGET_FILE:Nastran2Nastran_test>)

add_executable(
 Optistruct2Aster_test
 Optistruct2Aster_test.cpp
)

SET_TARGET_PROPERTIES(Optistruct2Aster_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(Optistruct2Aster_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 Optistruct2Aster_test
 command_line_test_utils
 commandline
)

add_test(NAME Optistruct2Aster_test COMMAND $<TARGET_FILE:Optistruct2Aster_test>)

#uncomment to see details of each test method (update tests.cmake with
#the batch file ../update_tests.sh
#include(tests.cmake)
