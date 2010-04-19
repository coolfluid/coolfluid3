CF_FIND_ORPHAN_FILES()

LIST ( APPEND Mesh_files
  Mesh.hpp
)

LIST ( APPEND Mesh_cflibs Common )

SET  ( Mesh_kernellib TRUE )

CF_ADD_LIBRARY ( Mesh )

ADD_SUBDIRECTORY ( uTests )    # unit tests for Mesh

