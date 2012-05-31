coolfluid_find_orphan_files()

list( APPEND coolfluid_mesh_files
  AInterpolator.hpp
  AInterpolator.cpp
  BoundingBox.hpp
  BoundingBox.cpp
  CellFaces.hpp
  CellFaces.cpp
  Cells.hpp
  Cells.cpp
  Connectivity.hpp
  Connectivity.cpp
  Dictionary.hpp
  Dictionary.cpp
  ContinuousDictionary.hpp
  ContinuousDictionary.cpp
  DiscontinuousDictionary.hpp
  DiscontinuousDictionary.cpp
  Domain.hpp
  Domain.cpp
  Entities.hpp
  Entities.cpp
  Elements.hpp
  Elements.cpp
  ElementConnectivity.hpp
  ElementConnectivity.cpp
  FaceCellConnectivity.hpp
  FaceCellConnectivity.cpp
  Faces.hpp
  Faces.cpp
  ElementTypes.hpp
  Field.hpp
  Field.cpp
  FieldManager.cpp
  FieldManager.hpp
  ParallelDistribution.hpp
  ParallelDistribution.cpp
  InterpolationFunction.hpp
  InterpolationFunction.cpp
  Interpolator.hpp
  Interpolator.cpp
  InterpolatorTypes.cpp
  MatchedMeshInterpolator.hpp
  MatchedMeshInterpolator.cpp
  Mesh.hpp
  Mesh.cpp
  MeshElements.hpp
  MeshElements.cpp
  MeshGenerator.hpp
  MeshGenerator.cpp
  MeshPartitioner.hpp
  MeshPartitioner.cpp
  MeshReader.hpp
  MeshReader.cpp
  MeshTransformer.hpp
  MeshTransformer.cpp
  MeshTriangulator.hpp
  MeshTriangulator.cpp
  MeshWriter.hpp
  MeshWriter.cpp
  MergedParallelDistribution.hpp
  MergedParallelDistribution.cpp
  NodeElementConnectivity.hpp
  NodeElementConnectivity.cpp
  Node2FaceCellConnectivity.hpp
  Node2FaceCellConnectivity.cpp
  Octtree.hpp
  Octtree.cpp
  ConnectivityData.cpp
  ConnectivityData.hpp
  Reconstructions.hpp
  Region.hpp
  Region.cpp
  SimpleMeshGenerator.hpp
  SimpleMeshGenerator.cpp
  Space.hpp
  Space.cpp
  SpaceInterpolator.hpp
  SpaceInterpolator.cpp
  StencilComputer.hpp
  StencilComputer.cpp
  StencilComputerRings.hpp
  StencilComputerRings.cpp
  StencilComputerOcttree.hpp
  StencilComputerOcttree.cpp
  UnifiedData.hpp
  UnifiedData.cpp
  ElementData.hpp
  ElementFinder.hpp
  ElementFinder.cpp
  ElementFinderOcttree.hpp
  ElementFinderOcttree.cpp
  ElementType.hpp
  ElementTypePredicates.hpp
  ElementTypeT.hpp
  ElementTypeBase.hpp
  GeoShape.hpp
  GeoShape.cpp
  InterpolationFunction.hpp
  InterpolationFunction.cpp
  LibMesh.hpp
  LibMesh.cpp
  LoadMesh.hpp
  LoadMesh.cpp
  MeshAdaptor.hpp
  MeshAdaptor.cpp
  MeshMetadata.hpp
  MeshMetadata.cpp
  PointInterpolator.hpp
  PointInterpolator.cpp
  PointInterpolatorT.hpp
  PseudoLaplacianLinearInterpolation.hpp
  PseudoLaplacianLinearInterpolation.cpp
  ShapeFunction.hpp
  ShapeFunctionT.hpp
  ShapeFunctionBase.hpp
  ShapeFunctionInterpolation.hpp
  ShapeFunctionInterpolation.cpp
  Tags.hpp
  Tags.cpp
  WriteMesh.hpp
  WriteMesh.cpp
  Functions.hpp
  Functions.cpp

# directories with headers only can have their contents appended to base dir
  Integrators/Gauss.hpp
  Integrators/GaussImplementation.hpp
)

list( APPEND coolfluid_mesh_cflibs coolfluid_common coolfluid_math )

set( coolfluid_mesh_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh )

############################################################################################
# sub modules of cf3Mesh

add_subdirectory(LagrangeP0)      # LagrangeP0 shape functions

add_subdirectory(LagrangeP1)      # LagrangeP1 elements / shape functions

add_subdirectory(LagrangeP2)      # LagrangeP2 elements / shape functions

add_subdirectory(LagrangeP2B)     # LagrangeP2 bubble elements / shape functions

add_subdirectory(LagrangeP3)      # LagrangeP3 elements / shape functions

add_subdirectory( neu )           # neutral file IO

add_subdirectory( gmsh )          # gmsh file IO

add_subdirectory( BlockMesh )     # Structured mesh generation

add_subdirectory( CGNS )          # CGNS file IO

add_subdirectory( tecplot )       # tecplot file IO

add_subdirectory( zoltan )        # zoltan mesh partitioning

add_subdirectory( ptscotch )      # PTScotch mesh partitioning

add_subdirectory( actions )       # namespace actions that can be performed on the mesh

add_subdirectory(VTKLegacy)       # Writer for VTK legacy files

add_subdirectory(VTKXML)       # Writer for VTK XML files
