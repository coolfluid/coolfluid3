coolfluid_find_orphan_files()

list( APPEND coolfluid_mesh_actions_files
  CInfo.hpp
  CInfo.cpp
  CExtract.hpp
  CExtract.cpp
  CBuildFaces.hpp
  CBuildFaces.cpp
  CBuildCoordinatesField.hpp
  CBuildCoordinatesField.cpp
  LibActions.hpp
  LibActions.cpp
)

list( APPEND coolfluid_mesh_actions_cflibs coolfluid_mesh )

set( coolfluid_mesh_actions_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_actions )
