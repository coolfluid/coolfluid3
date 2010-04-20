# MUST UNCOMMENT FIRST AND LAST LINE WHEN Mesh lib can be created!

#CF_FIND_ORPHAN_FILES()

LIST ( APPEND Mesh_files
  Mesh.hpp
)

LIST ( APPEND Mesh_cflibs Common )

SET  ( Mesh_kernellib TRUE )

CF_ADD_LIBRARY ( Mesh )

ADD_SUBDIRECTORY ( CFmesh )    # CFmesh file IO
ADD_SUBDIRECTORY ( Neu )       # Neutral file IO
#ADD_SUBDIRECTORY ( uTests )    # unit tests for Mesh

