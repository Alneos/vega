ADD_LIBRARY(resultReaders STATIC
    ResultReadersFacade.cpp
    CSVResultReader.cpp
    F06Parser.cpp
)

target_link_libraries(
	resultReaders
    abstract
)

