add_executable( CSVResultReaderTest CSVResultReaderTest.cpp )

SET_TARGET_PROPERTIES(CSVResultReaderTest PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(CSVResultReaderTest PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 CSVResultReaderTest
 resultReaders
)

ADD_TEST(CSVResultReaderTest ${EXECUTABLE_OUTPUT_PATH}/CSVResultReaderTest)

#----

add_executable(
 nastran_f06_tests
 F06Parser_test.cpp
)

SET_TARGET_PROPERTIES(nastran_f06_tests PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(nastran_f06_tests PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 nastran_f06_tests
 resultReaders
)

ADD_TEST(NastranF06 ${EXECUTABLE_OUTPUT_PATH}/nastran_f06_tests)