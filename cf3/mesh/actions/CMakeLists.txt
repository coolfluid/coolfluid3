coolfluid_find_orphan_files()

list( APPEND coolfluid_mesh_actions_files
  AddPointRegion.hpp
  AddPointRegion.cpp
  Info.hpp
  Info.cpp
  CreateField.hpp
  CreateField.cpp
  Extract.hpp
  Extract.cpp
  BuildArea.hpp
  BuildArea.cpp
  BuildFaces.hpp
  BuildFaces.cpp
  BuildFaceNormals.hpp
  BuildFaceNormals.cpp
  BuildVolume.hpp
  BuildVolume.cpp
  ComputeFieldGradient.hpp
  ComputeFieldGradient.cpp
  GlobalNumbering.hpp
  GlobalNumbering.cpp
  GlobalNumberingElements.hpp
  GlobalNumberingElements.cpp
  GlobalNumberingNodes.hpp
  GlobalNumberingNodes.cpp
  GlobalConnectivity.hpp
  GlobalConnectivity.cpp
  ImportVariables.hpp
  ImportVariables.cpp
  InitFieldConstant.hpp
  InitFieldConstant.cpp
  InitFieldFunction.hpp
  InitFieldFunction.cpp
  GrowOverlap.hpp
  GrowOverlap.cpp
  Interpolate.hpp
  Interpolate.cpp
  LibActions.hpp
  LibActions.cpp
  LinkPeriodicNodes.hpp
  LinkPeriodicNodes.cpp
  MakeBoundaryGlobal.hpp
  MakeBoundaryGlobal.cpp
  PeriodicMeshPartitioner.hpp
  PeriodicMeshPartitioner.cpp
  LoadBalance.hpp
  LoadBalance.cpp
  RemoveGhostElements.hpp
  RemoveGhostElements.cpp
  Rotate.hpp
  Rotate.cpp
  ShortestEdge.hpp
  ShortestEdge.cpp
  Translate.hpp
  Translate.cpp
  WallDistance.hpp
  WallDistance.cpp
)

coolfluid3_add_library( TARGET   coolfluid_mesh_actions
                        KERNEL
                        SOURCES  ${coolfluid_mesh_actions_files}
                        LIBS     coolfluid_mesh coolfluid_mesh_lagrangep1 )
