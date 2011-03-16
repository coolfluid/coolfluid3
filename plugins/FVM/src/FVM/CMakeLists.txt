list( APPEND coolfluid_fvm_files
  LibFVM.cpp
  LibFVM.hpp
  BCDirichlet.hpp
  BCDirichlet.cpp
  BuildGhostStates.hpp
  BuildGhostStates.cpp
  ComputeFlux.hpp
  ComputeFlux.cpp
  ComputeUpdateCoefficient.hpp
  ComputeUpdateCoefficient.cpp
  GhostCells.hpp
  GhostCells.cpp
  FiniteVolumeSolver.hpp
  FiniteVolumeSolver.cpp
  OutputIterationInfo.hpp
  OutputIterationInfo.cpp
  RoeFluxSplitter.hpp
  RoeFluxSplitter.cpp
  ShockTube.hpp
  ShockTube.cpp
  UpdateSolution.hpp
  UpdateSolution.cpp
)

list( APPEND coolfluid_fvm_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_solver_actions coolfluid_solver coolfluid_mesh_actions coolfluid_mesh_generation )

coolfluid_add_library( coolfluid_fvm )

