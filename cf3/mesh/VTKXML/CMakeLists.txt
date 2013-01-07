list( APPEND coolfluid_mesh_vtkxml_files
  Writer.hpp
  Writer.cpp
  LibVTKXML.cpp
  LibVTKXML.hpp
)

coolfluid3_add_library( TARGET  coolfluid_mesh_vtkxml 
                        KERNEL
                        SOURCES ${coolfluid_mesh_vtkxml_files} 
                        LIBS    coolfluid_mesh )
