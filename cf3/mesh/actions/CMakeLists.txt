coolfluid_find_orphan_files()

list( APPEND coolfluid_mesh_actions_files
  Info.hpp
  Info.cpp
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
  GlobalNumbering.hpp
  GlobalNumbering.cpp
  GlobalNumberingElements.hpp
  GlobalNumberingElements.cpp
  GlobalNumberingNodes.hpp
  GlobalNumberingNodes.cpp
  GlobalConnectivity.hpp
  GlobalConnectivity.cpp
  InitFieldConstant.hpp
  InitFieldConstant.cpp
  InitFieldFunction.hpp
  InitFieldFunction.cpp
  MatchNodes.hpp
  MatchNodes.cpp
  MergeMeshes.hpp
  MergeMeshes.cpp
  GrowOverlap.hpp
  GrowOverlap.cpp
  Interpolate.hpp
  Interpolate.cpp
  LibActions.hpp
  LibActions.cpp
  LoadBalance.hpp
  LoadBalance.cpp
)

list( APPEND coolfluid_mesh_actions_cflibs coolfluid_mesh )

set( coolfluid_mesh_actions_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_actions )
