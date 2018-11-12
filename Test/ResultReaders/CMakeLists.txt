add_executable( CSVResultReader_test CSVResultReaderTest.cpp )

SET_TARGET_PROPERTIES(CSVResultReader_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(CSVResultReader_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 CSVResultReader_test
 resultReaders
)

add_test(NAME CSVResultReader_test COMMAND CSVResultReader_test)

#----

add_executable(
 F06Parser_test
 F06Parser_test.cpp
)

SET_TARGET_PROPERTIES(F06Parser_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(F06Parser_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 F06Parser_test
 resultReaders
)

add_test(NAME F06Parser_test COMMAND F06Parser_test)
