list( APPEND coolfluid_ufem_files
  BoundaryConditions.hpp
  BoundaryConditions.cpp
  HeatConductionSteady.hpp
  HeatConductionSteady.cpp
  LibUFEM.cpp
  LibUFEM.hpp
  LinearSolver.hpp
  LinearSolver.cpp
  LinearSolverUnsteady.hpp
  LinearSolverUnsteady.cpp
  NavierStokesOps.hpp
  TimeLoop.hpp
  TimeLoop.cpp
)

list( APPEND coolfluid_ufem_cflibs coolfluid_mesh_sf coolfluid_solver_actions coolfluid_physics)

set( coolfluid_ufem_condition ${CF_ENABLE_PROTO} )

coolfluid_add_library( coolfluid_ufem )