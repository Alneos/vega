
configure_file (
  "build_properties.h.in"
  "${CMAKE_BINARY_DIR}/build_properties.h"
)

ADD_LIBRARY( abstract STATIC
       Analysis.cpp BoundaryCondition.cpp ConfigurationParameters.cpp CoordinateSystem.cpp
       Element.cpp Loading.cpp Material.cpp Model.cpp Mesh.cpp MeshComponents.cpp Objective.cpp
       SolverInterfaces.cpp Utility.cpp Value.cpp Constraint.cpp Dof.cpp Target.cpp
)

target_link_libraries(abstract ${EXTERNAL_LIBRARIES})

IF(MSVC)
	TARGET_COMPILE_OPTIONS(abstract PRIVATE /bigobj)
ENDIF(MSVC)

CHECK_CXX_COMPILER_FLAG(-Wa,-mbig-obj COMPILER_SUPPORTS_BIGOBJ)
if(COMPILER_SUPPORTS_BIGOBJ)
  TARGET_COMPILE_OPTIONS(abstract PRIVATE -Wa,-mbig-obj)
endif()

