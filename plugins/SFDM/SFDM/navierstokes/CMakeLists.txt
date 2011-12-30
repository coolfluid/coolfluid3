# coolfluid_sfdm_navierstokes

list( APPEND coolfluid_sfdm_navierstokes_files
  LibNavierStokes.hpp
  LibNavierStokes.cpp
  Convection1D.hpp
  Convection1D.cpp
  Convection2D.hpp
  Convection2D.cpp
)

list( APPEND coolfluid_sfdm_navierstokes_cflibs coolfluid_sfdm coolfluid_physics_navierstokes )

coolfluid_add_library( coolfluid_sfdm_navierstokes )

