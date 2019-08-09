add_executable(
 NastranParser_test
 NastranParser_test.cpp
)

SET_TARGET_PROPERTIES(NastranParser_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(NastranParser_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 NastranParser_test
 nastran
)

add_test(NAME NastranParser COMMAND NastranParser_test)

add_executable(
 NastranTokenizer_test
 NastranTokenizer_test.cpp
)

SET_TARGET_PROPERTIES(NastranTokenizer_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(NastranTokenizer_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 NastranTokenizer_test
 nastran
)

add_test(NAME NastranTokenizer COMMAND NastranTokenizer_test)

add_executable(
 NastranWriter_test
 NastranWriter_test.cpp
)

SET_TARGET_PROPERTIES(NastranWriter_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(NastranWriter_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 NastranWriter_test
 nastran
)

add_test(NAME NastranWriter COMMAND NastranWriter_test)

add_executable(
 NastranCoverage_test
 NastranCoverage_test.cpp
)

SET_TARGET_PROPERTIES(NastranCoverage_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(NastranCoverage_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 NastranCoverage_test
 nastran
)

add_test(NAME NastranCoverage COMMAND NastranCoverage_test)
