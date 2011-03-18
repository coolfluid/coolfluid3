list( APPEND coolfluid_fvm_files
  LibFVM.cpp
  LibFVM.hpp
  BCDirichlet.hpp
  BCDirichlet.cpp
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
  ComputeFluxCons1D.hpp
  ComputeFluxCons1D.cpp
  ComputeFluxCons2D.hpp
  ComputeFluxCons2D.cpp
  ComputeUpdateCoefficient.hpp
  ComputeUpdateCoefficient.cpp
  GhostCells.hpp
  GhostCells.cpp
  FiniteVolumeSolver.hpp
  FiniteVolumeSolver.cpp
  FiniteVolumeSolver2D.hpp
  FiniteVolumeSolver2D.cpp
  OutputIterationInfo.hpp
  OutputIterationInfo.cpp
  RoeFluxSplitter.hpp
  RoeFluxSplitter.cpp
  RoeFluxSplitterCons1D.hpp
  RoeFluxSplitterCons1D.cpp
  RoeFluxSplitterCons2D.hpp
  RoeFluxSplitterCons2D.cpp
  ShockTube.hpp
  ShockTube.cpp
  ShockTube1D.hpp
  ShockTube1D.cpp
  ShockTube2D.hpp
  ShockTube2D.cpp
  UpdateSolution.hpp
  UpdateSolution.cpp
)

list( APPEND coolfluid_fvm_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_solver_actions coolfluid_solver coolfluid_mesh_actions coolfluid_mesh_generation )

coolfluid_add_library( coolfluid_fvm )

