# coolfluid_sdm_navierstokesmovingreference

list( APPEND coolfluid_sdm_navierstokesmovingreference_files
  LibNavierStokesMovingReference.hpp
  LibNavierStokesMovingReference.cpp
  BCSubsonicInlet2D.hpp
  BCSubsonicInlet2D.cpp
  BCSubsonicInlet3D.hpp
  BCSubsonicInlet3D.cpp
  BCSubsonicOutlet2D.hpp
  BCSubsonicOutlet2D.cpp
  BCSubsonicOutlet3D.hpp
  BCSubsonicOutlet3D.cpp
  Convection2D.hpp
  Convection2D.cpp
  Convection3D.hpp
  Convection3D.cpp
  Source2D.hpp
  Source2D.cpp
  Source3D.hpp
  Source3D.cpp
  Diffusion2D.hpp
  Diffusion2D.cpp
)

list( APPEND coolfluid_sdm_navierstokesmovingreference_cflibs coolfluid_sdm coolfluid_physics_navierstokes )

coolfluid_add_library( coolfluid_sdm_navierstokesmovingreference )

