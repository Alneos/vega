add_executable(
 nastran_tests
 NastranParser_test.cpp
)

SET_TARGET_PROPERTIES(nastran_tests PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(nastran_tests PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})


add_executable(
 nastran_tokenizer_tests
 NastranTokenizer_test.cpp
)

SET_TARGET_PROPERTIES(nastran_tokenizer_tests PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(nastran_tokenizer_tests PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 nastran_tokenizer_tests
 nastran
)

target_link_libraries(
 nastran_tests
 nastran
)

ADD_TEST(NastranParser ${EXECUTABLE_OUTPUT_PATH}/nastran_tests)

ADD_TEST(NastranTokenizer ${EXECUTABLE_OUTPUT_PATH}/nastran_tokenizer_tests)
