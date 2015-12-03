
configure_file (
  "build_properties.h.in"
  "${CMAKE_BINARY_DIR}/build_properties.h"
)

ADD_LIBRARY( abstract STATIC
       Analysis.cpp BoundaryCondition.cpp ConfigurationParameters.cpp CoordinateSystem.cpp
       Element.cpp Loading.cpp Material.cpp Model.cpp Mesh.cpp MeshComponents.cpp Objective.cpp
       SolverInterfaces.cpp Utility.cpp Value.cpp Constraint.cpp Dof.cpp
)
       
target_link_libraries(abstract ${EXTERNAL_LIBRARIES})

IF(WIN32)
	TARGET_COMPILE_OPTIONS(abstract PRIVATE /bigobj)
ENDIF(WIN32)