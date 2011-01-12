list( APPEND coolfluid_ufem_files
  HeatConductionLinearSteady.cpp
  HeatConductionLinearSteady.hpp
  LibUFEM.cpp
  LibUFEM.hpp
  SetupHeatConductionLinearSteady.cpp
  SetupHeatConductionLinearSteady.hpp
)

list( APPEND coolfluid_ufem_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_solver_actions coolfluid_solver)

coolfluid_add_library( coolfluid_ufem )
