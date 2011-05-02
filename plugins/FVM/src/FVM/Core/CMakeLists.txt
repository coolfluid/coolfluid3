list( APPEND coolfluid_fvm_core_files
  LibCore.cpp
  LibCore.hpp
  BC.hpp
  BCReflectCons1D.hpp
  BCReflectCons1D.cpp
  BCReflectCons2D.hpp
  BCReflectCons2D.cpp
  BCDirichletCons1D.hpp
  BCDirichletCons1D.cpp
  BCDirichletCons2D.hpp
  BCDirichletCons2D.cpp
  BuildGhostStates.hpp
  BuildGhostStates.cpp
  ComputeFlux.hpp
  ComputeFlux.cpp
  ComputeUpdateCoefficient.hpp
  ComputeUpdateCoefficient.cpp
  CreateSpace.hpp
  CreateSpace.cpp
  GhostCells.hpp
  GhostCells.cpp
  FiniteVolumeSolver.hpp
  FiniteVolumeSolver.cpp
  OutputIterationInfo.hpp
  OutputIterationInfo.cpp
  PolynomialReconstructor.hpp
  PolynomialReconstructor.cpp
  RiemannSolver.hpp
  RiemannSolver.cpp
  RoeCons1D.hpp
  RoeCons1D.cpp
  RoeCons2D.hpp
  RoeCons2D.cpp
  ShockTube.hpp
  ShockTube.cpp
  UpdateSolution.hpp
  UpdateSolution.cpp
)

list( APPEND coolfluid_fvm_core_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_solver_actions coolfluid_solver coolfluid_mesh_actions )

coolfluid_add_library( coolfluid_fvm_core )

