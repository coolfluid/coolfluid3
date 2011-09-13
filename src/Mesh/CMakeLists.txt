coolfluid_find_orphan_files()

list( APPEND coolfluid_mesh_files
  ArrayBase.hpp
  ArrayBufferT.hpp
  CCellFaces.hpp
  CCellFaces.cpp
  CCells.hpp
  CCells.cpp
  CConnectivity.hpp
  CConnectivity.cpp
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
  ElementTypes.hpp
  Field.hpp
  Field.cpp
  FieldGroup.hpp
  FieldGroup.cpp
  FieldManager.cpp
  FieldManager.hpp
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
  CMeshElements.hpp
  CMeshElements.cpp
  CMeshGenerator.hpp
  CMeshGenerator.cpp
  CMeshPartitioner.hpp
  CMeshPartitioner.cpp
  CMeshReader.hpp
  CMeshReader.cpp
  CMeshTransformer.hpp
  CMeshTransformer.cpp
  CMeshWriter.hpp
  CMeshWriter.cpp
  CMixedHash.hpp
  CMixedHash.cpp
  Geometry.hpp
  Geometry.cpp
  CNodeElementConnectivity.hpp
  CNodeElementConnectivity.cpp
  CNodeFaceCellConnectivity.hpp
  CNodeFaceCellConnectivity.cpp
  COcttree.hpp
  COcttree.cpp
  ConnectivityData.cpp
  ConnectivityData.hpp
  CRegion.hpp
  CRegion.cpp
  CSimpleMeshGenerator.hpp
  CSimpleMeshGenerator.cpp
  CSpace.hpp
  CSpace.cpp
  CStencilComputer.hpp
  CStencilComputer.cpp
  CStencilComputerRings.hpp
  CStencilComputerRings.cpp
  CStencilComputerOcttree.hpp
  CStencilComputerOcttree.cpp
  CTable.hpp
  CTable.cpp
  CUnifiedData.hpp
  CUnifiedData.cpp
  ElementData.hpp
  ElementType.hpp
  ElementTypePredicates.hpp
  ElementTypeT.hpp
  ElementTypeBase.hpp
  GeoShape.hpp
  GeoShape.cpp
  LibMesh.hpp
  LibMesh.cpp
  ListBufferT.hpp
  ListBufferIterator.hpp
  LoadMesh.hpp
  LoadMesh.cpp
  Manipulations.hpp
  Manipulations.cpp
  MeshMetadata.hpp
  MeshMetadata.cpp
  ShapeFunction.hpp
  ShapeFunctionT.hpp
  ShapeFunctionBase.hpp
  Tags.hpp
  Tags.cpp
  WriteMesh.hpp
  WriteMesh.cpp

# directories with headers only can have their contents appended to base dir
  Integrators/Gauss.hpp
  Integrators/GaussImplementation.hpp
)

list( APPEND coolfluid_mesh_cflibs coolfluid_common coolfluid_math )

set( coolfluid_mesh_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh )

############################################################################################
# sub modules of CF.Mesh

add_subdirectory(LagrangeP0)      # LagrangeP0 shape functions

add_subdirectory(LagrangeP1)      # LagrangeP1 elements / shape functions

add_subdirectory(LagrangeP2)      # LagrangeP2 elements / shape functions

add_subdirectory(LagrangeP2B)     # LagrangeP2 bubble elements / shape functions

add_subdirectory(LagrangeP3)      # LagrangeP3 elements / shape functions

add_subdirectory( Neu )           # Neutral file IO

add_subdirectory( Gmsh )          # Gmsh file IO

add_subdirectory( BlockMesh )     # Structured mesh generation

add_subdirectory( CGNS )          # CGNS file IO

add_subdirectory( Tecplot )       # Tecplot file IO

add_subdirectory( Zoltan )        # Zoltan mesh partitioning

add_subdirectory( PTScotch )      # PTScotch mesh partitioning

add_subdirectory( Actions )       # Actions that can be performed on the mesh

add_subdirectory(VTKLegacy)       # Writer for VTK legacy files

add_subdirectory(VTKXML)       # Writer for VTK XML files
