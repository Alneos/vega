#----- Dof_test

add_executable(
 Dof_test
 Dof_test.cpp
)

SET_TARGET_PROPERTIES(Dof_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(Dof_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 Dof_test
 abstract
 ${EXTERNAL_LIBRARIES}
)

#----- Coordinate System test
add_executable(
 CoordinateSystem_test
 CoordinateSystem_test.cpp
)

SET_TARGET_PROPERTIES(CoordinateSystem_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(CoordinateSystem_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 CoordinateSystem_test
 abstract
 ${EXTERNAL_LIBRARIES}
)

add_executable(
 Mesh_test
 Mesh_test.cpp
)

SET_TARGET_PROPERTIES(Mesh_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(Mesh_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 Mesh_test
 abstract
 ${EXTERNAL_LIBRARIES}
)


add_executable(
 Model_test
 Model_test.cpp
)

SET_TARGET_PROPERTIES(Model_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(Model_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 Model_test
 abstract
 ${EXTERNAL_LIBRARIES}
)

add_executable(
 Utility_test
 Utility_test.cpp
)

SET_TARGET_PROPERTIES(Utility_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(Utility_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 Utility_test
 abstract
 ${EXTERNAL_LIBRARIES} 
)

add_executable(
 Value_test
 Value_test.cpp
)

SET_TARGET_PROPERTIES(Value_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(Value_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 Value_test
 abstract
 ${EXTERNAL_LIBRARIES} 
)

add_executable(
 Element_test
 Element_test.cpp
)

SET_TARGET_PROPERTIES(Element_test PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(Element_test PROPERTIES LINK_SEARCH_END_STATIC ${STATIC_LINKING})

target_link_libraries(
 Element_test
 abstract
 ${EXTERNAL_LIBRARIES} 
)

add_test(Dof_test ${EXECUTABLE_OUTPUT_PATH}/Dof_test)
add_test(CoordinateSystem_tests ${EXECUTABLE_OUTPUT_PATH}/CoordinateSystem_test)
add_test(Model_test ${EXECUTABLE_OUTPUT_PATH}/Model_test)
add_test(Utility_test ${EXECUTABLE_OUTPUT_PATH}/Utility_test)
add_test(Value_test ${EXECUTABLE_OUTPUT_PATH}/Value_test)
add_test(Mesh_test ${EXECUTABLE_OUTPUT_PATH}/Mesh_test)
add_test(Element_test ${EXECUTABLE_OUTPUT_PATH}/Element_test)

#uncomment to see details of each test method (update tests.cmake with
#the batch file ../update_tests.sh
#include(tests.cmake)
