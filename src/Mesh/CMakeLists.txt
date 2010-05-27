CF_FIND_ORPHAN_FILES()

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
  CMeshReader.hpp
  CMeshReader.cpp
  CMeshWriter.hpp
  CMeshWriter.cpp
  CRegion.hpp
  CRegion.cpp
  CTable.hpp
  CTable.cpp
  BufferT.hpp
  BufferT.cpp
  ConnectivityTableCF2.hpp
  ElementType.hpp
  ElementType.cpp
  GeoShape.hpp
  GeoShape.cpp
  RCM.hpp
  RCM.cpp

# directories with headers only can have their contents appended to base dir
  LagrangeSF/TriagP1.hpp
  LagrangeSF/QuadP1.hpp
  LagrangeSF/LagrangeSFTypes.hpp
  Integrators/Gauss.hpp
  Integrators/GaussImplementation.hpp
  Integrators/IntegrationFunctorBase.hpp
)

LIST ( APPEND Mesh_cflibs Common )

SET  ( Mesh_kernellib TRUE )

CF_ADD_LIBRARY ( Mesh )

ADD_SUBDIRECTORY ( P1 )            # implementation of P1 elements

ADD_SUBDIRECTORY ( Neu )           # Neutral file IO
ADD_SUBDIRECTORY ( OpenFOAM )      # OpenFOAM IO
ADD_SUBDIRECTORY ( Gmsh )          # Gmsh file IO
ADD_SUBDIRECTORY ( CGNS )          # CGNS file IO

ADD_SUBDIRECTORY ( uTests )        # unit tests for Mesh

