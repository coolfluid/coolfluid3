# coolfluid_sdm_navierstokes

list( APPEND coolfluid_sdm_navierstokes_files
  LibNavierStokes.hpp
  LibNavierStokes.cpp
  BCSubsonicInlet2D.hpp
  BCSubsonicInlet2D.cpp
  BCSubsonicOutlet2D.hpp
  BCSubsonicOutlet2D.cpp
  BCWallEuler2D.hpp
  BCWallEuler2D.cpp
  BCWall2D.hpp
  BCWall2D.cpp
  Convection1D.hpp
  Convection1D.cpp
  Convection2D.hpp
  Convection2D.cpp
  Diffusion2D.hpp
  Diffusion2D.cpp
)

list( APPEND coolfluid_sdm_navierstokes_cflibs coolfluid_sdm coolfluid_physics_navierstokes )

coolfluid_add_library( coolfluid_sdm_navierstokes )

