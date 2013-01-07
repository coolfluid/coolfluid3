list( APPEND coolfluid_mesh_neu_files
  Reader.hpp
  Reader.cpp
  Writer.hpp
  Writer.cpp
  LibNeu.cpp
  LibNeu.hpp
  Shared.hpp
  Shared.cpp
)

coolfluid3_add_library( TARGET  coolfluid_mesh_neu 
                        KERNEL
                        SOURCES ${coolfluid_mesh_neu_files}
                        LIBS    coolfluid_mesh_lagrangep1 coolfluid_mesh_actions )
