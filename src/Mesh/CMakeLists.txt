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
  CFlexTable.hpp
  CFlexTable.cpp
  CInterpolator.hpp  
  CInterpolator.cpp
  CHoneycombInterpolator.hpp
  CHoneycombInterpolator.cpp
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

  ElementType.hpp
  ElementType.cpp
  Hexa3D.hpp
  Hexa3D.cpp
  Line1D.hpp
  Line1D.cpp
  Line2D.hpp
  Line2D.cpp
  Line3D.hpp
  Line3D.cpp
  Quad2D.hpp
  Quad2D.cpp
  Quad3D.hpp
  Quad3D.cpp
  Tetra3D.hpp
  Tetra3D.cpp
  Triag2D.hpp
  Triag2D.cpp
  Triag3D.hpp
  Triag3D.cpp

# directories with headers only can have their contents appended to base dir
  Integrators/Gauss.hpp
  Integrators/GaussImplementation.hpp
  Integrators/IntegrationFunctorBase.hpp
)

LIST( APPEND Mesh_cflibs Common )

SET( Mesh_kernellib TRUE )

CF_ADD_LIBRARY( Mesh )

ADD_SUBDIRECTORY( Neu )           # Neutral file IO

ADD_SUBDIRECTORY( OpenFOAM )      # OpenFOAM IO

ADD_SUBDIRECTORY( Gmsh )          # Gmsh file IO

ADD_SUBDIRECTORY( CGNS )          # CGNS file IO

ADD_SUBDIRECTORY( CGAL )

ADD_SUBDIRECTORY( uTests )        # unit tests for Mesh

ADD_SUBDIRECTORY( SF )
