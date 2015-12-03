
ADD_LIBRARY( commandline STATIC
       VegaCommandLine.cpp
)

if(ENABLE_ASTER)
    list(APPEND MODULES aster)
endif(ENABLE_ASTER)

if(ENABLE_NASTRAN)
  list(APPEND MODULES nastran)
endif(ENABLE_NASTRAN)

if(ENABLE_SYSTUS)
    list(APPEND MODULES systus)
endif(ENABLE_SYSTUS)

list(APPEND MODULES resultReaders)

target_link_libraries( commandline ${MODULES})


add_executable(
    vegapp 
    vegamain.cpp 
)

SET_TARGET_PROPERTIES(vegapp PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKING})
SET_TARGET_PROPERTIES(vegapp PROPERTIES LINK_SEARCH_END_STATIC OFF)

target_link_libraries( 
    vegapp
    commandline
)
#install visual studio libraries
if(WIN32)
    include(InstallRequiredSystemLibraries)
    add_dependencies(vegapp ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
endif(WIN32)
        
install(TARGETS vegapp 
        EXPORT vegappTargets
        CONFIGURATIONS Release
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)
#install examples
set(NASTRAN_BASE ${PROJECT_SOURCE_DIR}/testdata/nastran2aster)
