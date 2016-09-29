coolfluid3_add_library(TARGET coolfluid_ufem_compressible
  SOURCES
    LibUFEMCompressible.hpp
    LibUFEMCompressible.cpp
    NavierStokes.hpp
    NavierStokes.cpp
    NavierStokesAssembly.hpp
    NavierStokesHexas.cpp
    NavierStokesPrisms.cpp
    NavierStokesQuads.cpp
    NavierStokesTetras.cpp
    NavierStokesTriags.cpp
    SUPG.hpp
  LIBS
    coolfluid_mesh_lagrangep1
    coolfluid_mesh_lagrangep2
    coolfluid_solver_actions
    coolfluid_physics
    coolfluid_mesh
    coolfluid_ufem
  CONDITION  CF3_ENABLE_PROTO)
