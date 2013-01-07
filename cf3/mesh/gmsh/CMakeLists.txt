list( APPEND coolfluid_mesh_gmsh_files
  Writer.hpp
  Writer.cpp
  Reader.hpp
  Reader.cpp
  LibGmsh.cpp
  LibGmsh.hpp
  Shared.cpp
  Shared.hpp
)

coolfluid3_add_library( TARGET  coolfluid_mesh_gmsh 
                        KERNEL
                        SOURCES ${coolfluid_mesh_gmsh_files}
                        LIBS    coolfluid_mesh )
