########################################################################
# coolfluid_rdm_navierstokes

list( APPEND coolfluid_rdm_navierstokes_files
  LibNavierStokes.cpp
  LibNavierStokes.hpp
NavierStokesLDA.cpp
NavierStokesSUPG.cpp
NavierStokesLF.cpp
NavierStokesN.cpp
NavierStokesB.cpp
)

list( APPEND coolfluid_rdm_navierstokes_cflibs coolfluid_rdm_schemes )

coolfluid_add_library( coolfluid_rdm_navierstokes )
