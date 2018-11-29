ADD_LIBRARY( aster STATIC
       AsterWriter.cpp AsterRunner.cpp MedWriter.cpp AsterModel.cpp
)

target_link_libraries( aster abstract ${EXTERNAL_LIBRARIES})
