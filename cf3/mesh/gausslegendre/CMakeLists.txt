list( APPEND coolfluid_mesh_gausslegendre_files
  API.hpp
  LibGaussLegendre.hpp
  LibGaussLegendre.cpp
  Line.hpp
  Line.cpp
  Legendre.hpp
  Legendre.cpp
)

coolfluid3_add_library( TARGET  coolfluid_mesh_gausslegendre 
                        KERNEL
                        SOURCES ${coolfluid_mesh_gausslegendre_files}
                        LIBS    coolfluid_mesh )
