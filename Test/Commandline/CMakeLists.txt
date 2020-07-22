
ADD_LIBRARY( command_line_test_utils STATIC
       CommandLineUtils.cpp
)

if(ENABLE_ASTER)
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
     boost_unit_test_framework
    )

    add_test(NAME Nastran2Aster_test COMMAND Nastran2Aster_test)

    add_executable(
     Nastran2AsterLong_test
     Nastran2AsterLong_test.cpp
    )

    SET_TARGET_PROPERTIES(Nastran2AsterLong_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
    SET_TARGET_PROPERTIES(Nastran2AsterLong_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

    target_link_libraries(
     Nastran2AsterLong_test
     command_line_test_utils
     commandline
     boost_unit_test_framework
    )

    add_test(NAME Nastran2AsterLong_test COMMAND Nastran2AsterLong_test)
endif(ENABLE_ASTER)

if(ENABLE_SYSTUS)
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
 boost_unit_test_framework
)

add_test(NAME Nastran2Systus_test COMMAND Nastran2Systus_test)
endif(ENABLE_SYSTUS)

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
 boost_unit_test_framework
)

add_test(NAME Nastran2Nastran_test COMMAND Nastran2Nastran_test)

add_executable(
 Optistruct2Nastran_test
 Optistruct2Nastran_test.cpp
)

SET_TARGET_PROPERTIES(Optistruct2Nastran_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(Optistruct2Nastran_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 Optistruct2Nastran_test
 command_line_test_utils
 commandline
 boost_unit_test_framework
)

add_test(NAME Optistruct2Nastran_test COMMAND Optistruct2Nastran_test)

if(ENABLE_ASTER)
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
     boost_unit_test_framework
    )

    add_test(NAME Optistruct2Aster_test COMMAND Optistruct2Aster_test)
endif(ENABLE_ASTER)

if(ENABLE_SYSTUS)
    add_executable(
     Optistruct2Systus_test
     Optistruct2Systus_test.cpp
    )

    SET_TARGET_PROPERTIES(Optistruct2Systus_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
    SET_TARGET_PROPERTIES(Optistruct2Systus_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

    target_link_libraries(
     Optistruct2Systus_test
     command_line_test_utils
     commandline
     boost_unit_test_framework
    )

    add_test(NAME Optistruct2Systus_test COMMAND Optistruct2Systus_test)
endif(ENABLE_SYSTUS)

add_executable(
 NastranCoverage_test
 NastranCoverage_test.cpp
)

SET_TARGET_PROPERTIES(NastranCoverage_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(NastranCoverage_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 NastranCoverage_test
 nastran
 command_line_test_utils
 commandline
 boost_unit_test_framework
)

add_test(NAME NastranCoverage COMMAND NastranCoverage_test)

add_executable(
 IRTMiniRBS_test
 IRTMiniRBS_test.cpp
)

SET_TARGET_PROPERTIES(IRTMiniRBS_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(IRTMiniRBS_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 IRTMiniRBS_test
 nastran
 command_line_test_utils
 commandline
 boost_unit_test_framework
)

#add_test(NAME IRTMiniRBS COMMAND IRTMiniRBS_test)


#uncomment to see details of each test method (update tests.cmake with
#the batch file ../update_tests.sh
#include(tests.cmake)
