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

ADD_TEST(NastranParser ${EXECUTABLE_OUTPUT_PATH}/NastranParser_test)

ADD_TEST(NastranTokenizer ${EXECUTABLE_OUTPUT_PATH}/NastranTokenizer_test)
