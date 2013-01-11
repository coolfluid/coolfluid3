list( APPEND coolfluid_mesh_vtklegacy_files
  Writer.hpp
  Writer.cpp
  LibVTKLegacy.cpp
  LibVTKLegacy.hpp
)

coolfluid3_add_library( TARGET  coolfluid_mesh_vtklegacy
                        KERNEL
                        SOURCES ${coolfluid_mesh_vtklegacy_files}
                        LIBS    coolfluid_mesh )
