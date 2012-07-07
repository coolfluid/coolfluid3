# coolfluid_sdm_navierstokesmovingreference

list( APPEND coolfluid_sdm_navierstokesmovingreference_files
  LibNavierStokesMovingReference.hpp
  LibNavierStokesMovingReference.cpp
  BCSubsonicInlet2D.hpp
  BCSubsonicInlet2D.cpp
  BCSubsonicOutlet2D.hpp
  BCSubsonicOutlet2D.cpp
  Convection2D.hpp
  Convection2D.cpp
  Source2D.hpp
  Source2D.cpp
  Diffusion2D.hpp
  Diffusion2D.cpp
  SourceCoriolis2D.hpp
  SourceCoriolis2D.cpp
  SourceCentrifugal2D.cpp
  SourceCentrifugal2D.hpp

)

list( APPEND coolfluid_sdm_navierstokesmovingreference_cflibs coolfluid_sdm coolfluid_physics_navierstokes )

coolfluid_add_library( coolfluid_sdm_navierstokesmovingreference )

