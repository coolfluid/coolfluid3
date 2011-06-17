########################################################################
# coolfluid_rdm_navierstokes

list( APPEND coolfluid_rdm_navierstokes_files
  # library
  LibNavierStokes.cpp
  LibNavierStokes.hpp
  # domain terms (schemes)
  NavierStokesLDA.cpp
  NavierStokesSUPG.cpp
  NavierStokesLF.cpp
  NavierStokesN.cpp
  NavierStokesB.cpp
  # boundary terms (BCs)
  WallWeakBc.hpp
  WallWeakBc.cpp
  WallEdwinBc.hpp
  WallEdwinBc.cpp
  SubsonicInFlowWeakBc.hpp
  SubsonicInFlowWeakBc.cpp
  SubsonicOutFlowWeakBc.hpp
  SubsonicOutFlowWeakBc.cpp
)

list( APPEND coolfluid_rdm_navierstokes_cflibs coolfluid_rdm_schemes )

coolfluid_add_library( coolfluid_rdm_navierstokes )
