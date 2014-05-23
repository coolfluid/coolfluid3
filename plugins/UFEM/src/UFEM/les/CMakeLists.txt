coolfluid3_add_library(TARGET coolfluid_ufem_les
  SOURCES
    WALE.hpp
    WALE.cpp
    LibUFEMLES.hpp
    LibUFEMLES.cpp
  LIBS     
    coolfluid_mesh_lagrangep1 
    coolfluid_mesh_lagrangep2 
    coolfluid_solver_actions 
    coolfluid_physics 
    coolfluid_mesh
    coolfluid_ufem
  CONDITION  CF3_ENABLE_PROTO)