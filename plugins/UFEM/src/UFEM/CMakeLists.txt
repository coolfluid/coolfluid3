list( APPEND coolfluid_ufem_files
  BoundaryConditions.hpp
  BoundaryConditions.cpp
  LibUFEM.cpp
  LibUFEM.hpp
  LinearProblem.hpp
  LinearProblem.cpp
  Model.hpp
  Model.cpp
  NavierStokesOps.hpp
  SteadyModel.hpp
  SteadyModel.cpp
  TimeLoop.hpp
  UnsteadyModel.hpp
  UnsteadyModel.cpp
)

coolfluid_mark_not_orphan(
  HeatConductionLinearSteady.cpp
  HeatConductionLinearSteady.hpp
  HeatConductionLinearUnsteady.cpp
  HeatConductionLinearUnsteady.hpp
)

list( APPEND coolfluid_ufem_cflibs coolfluid_mesh_sf coolfluid_solver_actions)

set( coolfluid_ufem_condition ${CF_ENABLE_PROTO} )

coolfluid_add_library( coolfluid_ufem )