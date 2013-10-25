coolfluid3_add_library(TARGET coolfluid_ufem_particles
  SOURCES
    EulerDNS.hpp
    EulerDNS.cpp
    LibUFEMParticles.hpp
    LibUFEMParticles.cpp
  LIBS     
    coolfluid_mesh_lagrangep1 
    coolfluid_mesh_lagrangep2 
    coolfluid_solver_actions 
    coolfluid_physics 
    coolfluid_mesh
    coolfluid_ufem
  CONDITION  CF3_ENABLE_PROTO)