coolfluid_find_orphan_files()

list( APPEND coolfluid_mesh_files
  ArrayBase.hpp
  ArrayBufferT.hpp
  CMesh.cpp
  CArray.hpp
  CArray.cpp
  CElements.hpp
  CElements.cpp
  CField.hpp
  CField.cpp
  CFieldElements.hpp
  CFieldElements.cpp
  CHoneycombInterpolator.hpp
  CHoneycombInterpolator.cpp
  CInterpolator.hpp
  CInterpolator.cpp
  CList.hpp
  CList.cpp
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
  COperation.hpp
  COperation.cpp
  ConnectivityData.cpp
  ConnectivityData.hpp
  CRegion.hpp
  CRegion.cpp
  CTable.hpp
  CTable.cpp
  ElementData.hpp
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
  LibMesh.hpp
  LibMesh.cpp
  Line1D.hpp
  Line1D.cpp
  Line2D.hpp
  Line2D.cpp
  Line3D.hpp
  Line3D.cpp
  ListBase.hpp
  ListBufferT.hpp
  ListBufferIterator.hpp
  Quad2D.hpp
  Quad2D.cpp
  Quad3D.hpp
  Quad3D.cpp
  SimpleCommunicationPattern.hpp
  SimpleCommunicationPattern.cpp
  Tetra3D.hpp
  Tetra3D.cpp
  Triag2D.hpp
  Triag2D.cpp
  Triag3D.hpp
  Triag3D.cpp

# directories with headers only can have their contents appended to base dir
  Integrators/Gauss.hpp
  Integrators/GaussImplementation.hpp
)

list( APPEND coolfluid_mesh_cflibs coolfluid_math )  # Math includes coolfluid_common

set( coolfluid_mesh_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh )

add_subdirectory( SF )            # Shape functions

add_subdirectory( Neu )           # Neutral file IO

add_subdirectory( Gmsh )          # Gmsh file IO
add_subdirectory( BlockMesh )     # Structured mesh generation
add_subdirectory( CGNS )          # CGNS file IO

add_subdirectory( CGAL )          # CGAL mesh generator


# Must be after all the other subdirectories
add_subdirectory( uTests )        # unit tests for Mesh


