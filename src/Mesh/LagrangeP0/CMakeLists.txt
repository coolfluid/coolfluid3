list( APPEND coolfluid_mesh_lagrangeP0_files
  LibLagrangeP0.hpp
  LibLagrangeP0.cpp
  Triag.hpp
  Triag.cpp
)

list( APPEND coolfluid_mesh_lagrangeP0_cflibs coolfluid_mesh )

set( coolfluid_mesh_lagrangeP0_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_lagrangeP0 )
