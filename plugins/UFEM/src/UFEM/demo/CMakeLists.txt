coolfluid3_add_library(TARGET coolfluid_ufem_demo
  SOURCES
    NavierStokesChorin.hpp
    NavierStokesChorin.cpp
    NavierStokesManual.hpp
    NavierStokesManual.cpp
    NavierStokesSpecialized.hpp
    NavierStokesSpecialized.cpp
    PoissonManual.cpp
    PoissonManual.hpp
    PoissonProto.cpp
    PoissonProto.hpp
    PoissonSpecialized.cpp
    PoissonSpecialized.hpp
    PoissonVirtual.cpp
    PoissonVirtual.hpp
    LibUFEMDemo.cpp
    LibUFEMDemo.hpp
  LIBS     
    coolfluid_mesh_lagrangep1 
    coolfluid_mesh_lagrangep2 
    coolfluid_solver_actions 
    coolfluid_physics 
    coolfluid_mesh
    coolfluid_ufem
  CONDITION  CF3_ENABLE_PROTO)