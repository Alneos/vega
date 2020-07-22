add_executable(
 OptistructParser_test
 OptistructParser_test.cpp
)

SET_TARGET_PROPERTIES(OptistructParser_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(OptistructParser_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 OptistructParser_test
 optistruct
 nastran
 boost_unit_test_framework
)

add_test(NAME OptistructParser COMMAND OptistructParser_test)
