list( APPEND coolfluid_rdm_core_files
  LibCore.cpp
  LibCore.hpp
  RKRD.cpp
  RKRD.hpp
  Solver.cpp
  Solver.hpp
  SupportedCells.hpp
  SupportedFaces.hpp
  Quadrature.hpp
  ElementLoop.hpp
  CellLoop.hpp
  FaceLoop.hpp
  Tags.hpp
  BoundaryConditions.hpp
  BoundaryConditions.cpp
  InitialConditions.hpp
  InitialConditions.cpp
  DomainDiscretization.hpp
  DomainDiscretization.cpp
  TimeStepping.hpp
  TimeStepping.cpp
  IterativeSolver.hpp
  IterativeSolver.cpp
  SetupFields.hpp
  SetupFields.cpp
# wizards
  SteadyExplicit.hpp
  SteadyExplicit.cpp
  MySim.cpp
  MySim.hpp
# actions
  BoundaryTerm.hpp
  BoundaryTerm.cpp
  CellTerm.hpp
  CellTerm.cpp
  FaceTerm.hpp
  FaceTerm.cpp
  BcDirichlet.hpp
  BcDirichlet.cpp
  BcBase.hpp
  SchemeBase.hpp
  WeakDirichlet.cpp
  WeakDirichlet.hpp
  Cleanup.hpp
  Cleanup.cpp
# time stepping
  ForwardEuler.hpp
  ForwardEuler.cpp
  UpdateSolution.hpp
  UpdateSolution.cpp
)

list( APPEND coolfluid_rdm_core_cflibs coolfluid_physics coolfluid_mesh coolfluid_mesh_sf coolfluid_mesh_actions coolfluid_solver_actions coolfluid_solver )

coolfluid_add_library( coolfluid_rdm_core )
