CF_FIND_ORPHAN_FILES()

LIST ( APPEND MeshGeneration_files
  MeshGeneration.cpp
  MeshGeneration.hpp
)

LIST ( APPEND MeshGeneration_cflibs Common )
IF(CF_BUILD_GooglePerfTools)
  LIST ( APPEND MeshGeneration_cflibs GooglePerfTools )
ENDIF()

CF_ADD_LIBRARY ( MeshGeneration )

