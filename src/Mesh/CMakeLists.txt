#CF_FIND_ORPHAN_FILES()

LIST ( APPEND Mesh_files
  MeshAPI.hpp
  MeshAPI.cpp
  CMesh.cpp
  CArray.hpp
  CArray.cpp
  CField.hpp
  CField.cpp
  CMesh.hpp
  CMesh.cpp
  CRegion.hpp
  CRegion.cpp
  CTable.hpp
  CTable.cpp
  ConnectivityTable.hpp
  RCM.hpp
  RCM.cpp
)

LIST ( APPEND Mesh_cflibs Common )

SET  ( Mesh_kernellib TRUE )

CF_ADD_LIBRARY ( Mesh )

ADD_SUBDIRECTORY ( CFmesh )    # CFmesh file IO
ADD_SUBDIRECTORY ( Neu )       # Neutral file IO
ADD_SUBDIRECTORY ( uTests )    # unit tests for Mesh

