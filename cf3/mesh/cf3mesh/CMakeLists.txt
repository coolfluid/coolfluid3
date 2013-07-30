list( APPEND coolfluid_mesh_CF3Mesh_files
  Reader.hpp
  Reader.cpp
  Writer.hpp
  Writer.cpp
  LibCF3Mesh.cpp
  LibCF3Mesh.hpp
)

coolfluid3_add_library( TARGET  coolfluid_mesh_cf3mesh
                        KERNEL
                        SOURCES ${coolfluid_mesh_CF3Mesh_files}
                        LIBS    coolfluid_mesh )
