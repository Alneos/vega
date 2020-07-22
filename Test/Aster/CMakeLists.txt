add_executable(
 MedWriter_test
 MedWriter_test.cpp
)

SET_TARGET_PROPERTIES(MedWriter_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(MedWriter_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})


target_link_libraries(
 MedWriter_test
 aster
 nastran
 boost_unit_test_framework
)

add_test(NAME MedWriter COMMAND MedWriter_test)

