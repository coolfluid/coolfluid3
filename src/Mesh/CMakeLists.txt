CF_FIND_ORPHAN_FILES()

LIST( APPEND Mesh_files
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
  CMeshInfo.hpp
  CMeshInfo.cpp
  CMeshExtract.hpp
  CMeshExtract.cpp
  CMeshReader.hpp
  CMeshReader.cpp
  CMeshTransformer.hpp
  CMeshTransformer.cpp
  CMeshWriter.hpp
  CMeshWriter.cpp
  CRegion.hpp
  CRegion.cpp
  CTable.hpp
  CTable.cpp
  BufferT.hpp
  BufferT.cpp
  ElementNodes.hpp
  ElementNodes.cpp
  ElementType.hpp
  ElementType.cpp
  GeoShape.hpp
  GeoShape.cpp

  Elements/ElementType.hpp
  Elements/ElementType.cpp
  Elements/Hexa3D.hpp
  Elements/Hexa3D.cpp
  Elements/Line1D.hpp
  Elements/Line1D.cpp
  Elements/Line2D.hpp
  Elements/Line2D.cpp
  Elements/Quad2D.hpp
  Elements/Quad2D.cpp
  Elements/Tetra3D.hpp
  Elements/Tetra3D.cpp
  Elements/Triag2D.hpp
  Elements/Triag2D.cpp

# directories with headers only can have their contents appended to base dir
  Integrators/Gauss.hpp
  Integrators/GaussImplementation.hpp
  Integrators/IntegrationFunctorBase.hpp
)

LIST( APPEND Mesh_cflibs Common )

SET( Mesh_kernellib TRUE )

CF_ADD_LIBRARY( Mesh )

ADD_SUBDIRECTORY( P1 )            # implementation of P1 elements

ADD_SUBDIRECTORY( Neu )           # Neutral file IO

ADD_SUBDIRECTORY( OpenFOAM )      # OpenFOAM IO

ADD_SUBDIRECTORY( Gmsh )          # Gmsh file IO

ADD_SUBDIRECTORY( CGNS )          # CGNS file IO

ADD_SUBDIRECTORY( CGAL )

ADD_SUBDIRECTORY( uTests )        # unit tests for Mesh

ADD_SUBDIRECTORY( Elements/SF )