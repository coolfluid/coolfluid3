list( APPEND coolfluid_mesh_vtklegacy_files
  Writer.hpp
  Writer.cpp
  LibVTKLegacy.cpp
  LibVTKLegacy.hpp
)

list( APPEND coolfluid_mesh_vtklegacy_cflibs coolfluid_mesh )

set( coolfluid_mesh_vtklegacy_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_vtklegacy )
