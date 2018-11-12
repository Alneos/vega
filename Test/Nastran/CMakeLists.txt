add_executable(
 NastranParser_test
 NastranParser_test.cpp
)

SET_TARGET_PROPERTIES(NastranParser_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(NastranParser_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})


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

target_link_libraries(
 NastranParser_test
 nastran
)

add_test(NAME NastranParser COMMAND NastranParser_test)

add_test(NAME NastranTokenizer COMMAND NastranTokenizer_test)
