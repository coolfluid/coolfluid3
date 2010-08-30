CF_FIND_ORPHAN_FILES()

LIST( APPEND MeshDiff_files
  MeshDiff.cpp
  MeshDiff.hpp
)

LIST( APPEND MeshDiff_cflibs Mesh )
IF( CF_BUILD_GooglePerfTools )
  LIST( APPEND MeshDiff_cflibs GooglePerfTools )
ENDIF()

CF_ADD_LIBRARY( MeshDiff )


