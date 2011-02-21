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
  CBuildFaces.hpp
  CBuildFaces.cpp
  CBuildFaceNormals.hpp
  CBuildFaceNormals.cpp
  CBuildCoordinatesField.hpp
  CBuildCoordinatesField.cpp
  CBuildVolume.hpp
  CBuildVolume.cpp
  CInitSolution.hpp
  CInitSolution.cpp
#  CMatchNodes.hpp
#  CMatchNodes.cpp
  LibActions.hpp
  LibActions.cpp
)

list( APPEND coolfluid_mesh_actions_cflibs coolfluid_mesh )

set( coolfluid_mesh_actions_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_actions )
