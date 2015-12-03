ADD_LIBRARY( aster STATIC
       AsterWriter.cpp AsterRunner.cpp AsterModel.cpp AsterFacade.cpp
)

target_link_libraries( aster abstract ${EXTERNAL_LIBRARIES})
