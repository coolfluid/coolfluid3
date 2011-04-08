list( APPEND coolfluid_ufem_files
  HeatConductionLinearSteady.cpp
  HeatConductionLinearSteady.hpp
  HeatConductionLinearUnsteady.cpp
  HeatConductionLinearUnsteady.hpp
  LibUFEM.cpp
  LibUFEM.hpp
  LinearSystem.cpp
  LinearSystem.hpp
  LinearSystemUnsteady.cpp
  LinearSystemUnsteady.hpp
  NavierStokesOps.hpp
  SetupLinearSystem.cpp
  SetupLinearSystem.hpp
)

list( APPEND coolfluid_ufem_cflibs coolfluid_mesh_sf coolfluid_solver_actions)

set( coolfluid_ufem_condition ${CF_ENABLE_PROTO} )

coolfluid_add_library( coolfluid_ufem )
