coolfluid3_add_library(TARGET coolfluid_ufem_particles
  SOURCES
    EquilibriumEuler.hpp
    EquilibriumEuler.cpp
    EquilibriumEulerConvergence.hpp
    EquilibriumEulerConvergence.cpp
    EquilibriumEulerFEM.hpp
    EquilibriumEulerFEM.cpp
    LibUFEMParticles.hpp
    LibUFEMParticles.cpp
    ParticleConcentration.hpp
    ParticleConcentration.cpp
    TaylorGreen.hpp
    TaylorGreen.cpp
  LIBS     
    coolfluid_mesh_lagrangep1 
    coolfluid_mesh_lagrangep2 
    coolfluid_solver_actions 
    coolfluid_physics 
    coolfluid_mesh
    coolfluid_ufem
  CONDITION  CF3_ENABLE_PROTO)
