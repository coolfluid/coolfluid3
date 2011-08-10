coolfluid_find_orphan_files()

list( APPEND coolfluid_mesh_actions_files
  CBubbleEnrich.hpp
  CBubbleEnrich.cpp
  CBubbleRemove.hpp
  CBubbleRemove.cpp
  CInfo.hpp
  CInfo.cpp
  CExtract.hpp
  CExtract.cpp
  CBuildArea.hpp
  CBuildArea.cpp
  CBuildFaces.hpp
  CBuildFaces.cpp
  CBuildFaceNormals.hpp
  CBuildFaceNormals.cpp
  CBuildVolume.hpp
  CBuildVolume.cpp
  CGlobalNumbering.hpp
  CGlobalNumbering.cpp
  CGlobalNumberingElements.hpp
  CGlobalNumberingElements.cpp
  CGlobalNumberingNodes.hpp
  CGlobalNumberingNodes.cpp
  CGlobalConnectivity.hpp
  CGlobalConnectivity.cpp
  CInitFieldConstant.hpp
  CInitFieldConstant.cpp
  CInitFieldFunction.hpp
  CInitFieldFunction.cpp
  CMatchNodes.hpp
  CMatchNodes.cpp
  CreateSpaceP0.hpp
  CreateSpaceP0.cpp
  GrowOverlap.hpp
  GrowOverlap.cpp
  LibActions.hpp
  LibActions.cpp
  LoadBalance.hpp
  LoadBalance.cpp
)

list( APPEND coolfluid_mesh_actions_cflibs coolfluid_mesh )

set( coolfluid_mesh_actions_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_actions )
