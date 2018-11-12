ADD_LIBRARY(optistruct STATIC
    OptistructParser.cpp
)

target_link_libraries(
    optistruct
    nastran 
    abstract
)
