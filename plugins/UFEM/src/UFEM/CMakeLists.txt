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
  NavierStokes.hpp
  NavierStokes.cpp
  NavierStokesOps.hpp
  SparsityBuilder.hpp
  SparsityBuilder.cpp
  Tags.hpp
  Tags.cpp
  TimeLoop.hpp
  TimeLoop.cpp
)

list( APPEND coolfluid_ufem_cflibs coolfluid_mesh_lagrangep1 coolfluid_solver_actions coolfluid_physics)

set( coolfluid_ufem_condition ${CF_ENABLE_PROTO} )

coolfluid_mark_not_orphan(NavierStokes.cpp NavierStokes.hpp)

coolfluid_add_library( coolfluid_ufem )
set_source_files_properties(HeatConductionSteady.cpp NavierStokes.cpp PROPERTIES COMPILE_FLAGS "-g0 --param ggc-min-expand=20 --param ggc-min-heapsize=16384")
