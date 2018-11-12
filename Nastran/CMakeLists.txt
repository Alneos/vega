ADD_LIBRARY(nastran STATIC
    NastranParser.cpp
    NastranParser_geometry.cpp
    NastranParser_param.cpp
    NastranTokenizer.cpp
    NastranWriter.cpp
    NastranRunner.cpp
)

target_link_libraries(
    nastran 
    abstract
)
