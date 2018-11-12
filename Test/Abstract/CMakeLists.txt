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
add_test(NAME Utility_test COMMAND Utility_test)

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

add_test(NAME Dof_test COMMAND Dof_test)
add_test(NAME CoordinateSystem_test  COMMAND CoordinateSystem_test)
add_test(NAME Model_test COMMAND Model_test)
add_test(NAME Value_test COMMAND Value_test)
add_test(NAME Mesh_test COMMAND Mesh_test)
add_test(NAME Element_test COMMAND Element_test)

#uncomment to see details of each test method (update tests.cmake with
#the batch file ../update_tests.sh
#include(tests.cmake)
