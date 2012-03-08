# coolfluid_sdm_navierstokesmovingreference

list( APPEND coolfluid_sdm_navierstokesmovingreference_files
  LibNavierStokesMovingReference.hpp
  LibNavierStokesMovingReference.cpp
  Convection2D.hpp
  Convection2D.cpp
)

list( APPEND coolfluid_sdm_navierstokesmovingreference_cflibs coolfluid_sdm coolfluid_physics_navierstokes )

coolfluid_add_library( coolfluid_sdm_navierstokesmovingreference )

