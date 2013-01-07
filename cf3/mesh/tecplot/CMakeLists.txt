list( APPEND coolfluid_mesh_tecplot_files
  Writer.hpp
  Writer.cpp
  LibTecplot.cpp
  LibTecplot.hpp
  Shared.cpp
  Shared.hpp
)

coolfluid3_add_library( TARGET  coolfluid_mesh_tecplot 
                        KERNEL
                        SOURCES ${coolfluid_mesh_tecplot_files} 
                        LIBS    coolfluid_mesh )
