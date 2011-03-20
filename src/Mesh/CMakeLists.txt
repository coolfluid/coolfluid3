coolfluid_find_orphan_files()

list( APPEND coolfluid_mesh_files
  ArrayBase.hpp
  ArrayBufferT.hpp
  CCells.hpp
  CCells.cpp
  CDomain.hpp
  CDomain.cpp
  CDynTable.hpp
  CDynTable.cpp
  CEntities.hpp
  CEntities.cpp
  CElements.hpp
  CElements.cpp
  CFaceCellConnectivity.hpp
  CFaceCellConnectivity.cpp
  CFaces.hpp
  CFaces.cpp
  CField.hpp
  CField.cpp
  CFieldView.hpp
  CFieldView.cpp
  CHash.hpp
  CHash.cpp
  CInterpolator.hpp
  CInterpolator.cpp
  CLinearInterpolator.hpp
  CLinearInterpolator.cpp
  CList.hpp
  CList.cpp
  CMesh.hpp
  CMesh.cpp
  CMeshReader.hpp
  CMeshReader.cpp
  CMeshTransformer.hpp
  CMeshTransformer.cpp
  CMeshWriter.hpp
  CMeshWriter.cpp
  CMixedHash.hpp
  CMixedHash.cpp
  CNodes.hpp
  CNodes.cpp
  CNodeElementConnectivity.hpp
  CNodeElementConnectivity.cpp
  CNodeFaceCellConnectivity.hpp
  CNodeFaceCellConnectivity.cpp
  CCellFaces.hpp
  CCellFaces.cpp
  ConnectivityData.cpp
  ConnectivityData.hpp
  CMeshPartitioner.hpp
  CMeshPartitioner.cpp
  CRegion.hpp
  CRegion.cpp
  CSpace.hpp
  CSpace.cpp
  CTable.hpp
  CTable.cpp
  CUnifiedData.hpp
  ElementData.hpp
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
  ListBufferT.hpp
  ListBufferIterator.hpp
  LoadMesh.hpp
  LoadMesh.cpp
  Point1D.hpp
  Point1D.cpp
  Point2D.hpp
  Point2D.cpp
  Point3D.hpp
  Point3D.cpp
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
  WriteMesh.hpp
  WriteMesh.cpp

# directories with headers only can have their contents appended to base dir
  Integrators/Gauss.hpp
  Integrators/GaussImplementation.hpp
)

list( APPEND simplecomm_files
  SimpleCommunicationPattern.hpp
  SimpleCommunicationPattern.cpp
)

if(CF_TMP_HAVE_SIMPLECOMM)
  list( APPEND coolfluid_mesh_files ${simplecomm_files} )
else()
  coolfluid_mark_not_orphan( ${simplecomm_files} )
endif()

list( APPEND coolfluid_mesh_cflibs coolfluid_math )  # Math includes coolfluid_common

set( coolfluid_mesh_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh )

############################################################################################
# sub modules of CF.Mesh

add_subdirectory( SF )            # Shape functions

add_subdirectory( Neu )           # Neutral file IO

add_subdirectory( Gmsh )          # Gmsh file IO

add_subdirectory( BlockMesh )     # Structured mesh generation

add_subdirectory( CGNS )          # CGNS file IO

add_subdirectory( Tecplot )       # Tecplot file IO

add_subdirectory( Zoltan )        # Zoltan mesh partitioning

add_subdirectory( PTScotch )      # PTScotch mesh partitioning

add_subdirectory( Actions )       # Actions that can be performed on the mesh

