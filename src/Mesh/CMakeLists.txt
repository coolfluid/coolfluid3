#CF_FIND_ORPHAN_FILES()

LIST ( APPEND Mesh_files
  MeshAPI.hpp
  MeshAPI.cpp
  CMesh.cpp
  CArray.hpp
  CArray.cpp
  CElements.hpp
  CElements.cpp
  CField.hpp
  CField.cpp
  CMesh.hpp
  CMesh.cpp
  CRegion.hpp
  CRegion.cpp
  CTable.hpp
  CTable.cpp
  ConnectivityTable.hpp
  ConnectivityTableCF2.hpp
  ElementType.hpp
  ElementType.cpp
  GeoShape.hpp
  GeoShape.cpp
  RCM.hpp
  RCM.cpp
)

LIST ( APPEND Mesh_cflibs Common )

SET  ( Mesh_kernellib TRUE )

CF_ADD_LIBRARY ( Mesh )

ADD_SUBDIRECTORY ( P1 )        # implementation of P1 elements
ADD_SUBDIRECTORY ( CFmesh )    # CFmesh file IO
ADD_SUBDIRECTORY ( LagrangeSF ) 
ADD_SUBDIRECTORY ( Neu )       # Neutral file IO
ADD_SUBDIRECTORY ( uTests )    # unit tests for Mesh

