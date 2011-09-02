list( APPEND coolfluid_mesh_vtkxml_files
  CWriter.hpp
  CWriter.cpp
  LibVTKXML.cpp
  LibVTKXML.hpp
)

list( APPEND coolfluid_mesh_vtkxml_cflibs coolfluid_mesh )

set( coolfluid_mesh_vtkxml_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_vtkxml )
