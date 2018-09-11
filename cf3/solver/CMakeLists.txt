coolfluid_find_orphan_files()

list( APPEND coolfluid_solver_files
  Action.hpp
  Action.cpp
  ActionDirector.hpp
  ActionDirector.cpp
  ActionDirectorWithSkip.hpp
  ActionDirectorWithSkip.cpp
  BC.hpp
  BC.cpp
  Criterion.hpp
  Criterion.cpp
  CriterionAbsResidual.hpp
  CriterionAbsResidual.cpp
  CriterionMaxIterations.hpp
  CriterionMaxIterations.cpp
  CriterionMilestoneIteration.hpp
  CriterionMilestoneIteration.cpp
  CriterionMilestoneTime.hpp
  CriterionMilestoneTime.cpp
  CriterionTime.hpp
  CriterionTime.cpp
  ComputeCFL.hpp
  ComputeCFL.cpp
  ComputeLNorm.cpp
  ComputeLNorm.hpp
  ComputeRHS.hpp
  ComputeRHS.cpp
  Model.hpp
  Model.cpp
  ModelSteady.hpp
  ModelSteady.cpp
  ModelUnsteady.hpp
  ModelUnsteady.cpp
  History.hpp
  History.cpp
  ImposeCFL.hpp
  ImposeCFL.cpp
  SimpleSolver.hpp
  SimpleSolver.cpp
  RiemannSolver.hpp
  Solver.hpp
  Solver.cpp
  Time.hpp
  Time.cpp
  Term.hpp
  Term.cpp
  TermComputer.hpp
  TermComputer.cpp
  PDE.hpp
  PDE.cpp
  PDESolver.hpp
  PDESolver.cpp
  Plotter.cpp
  Plotter.hpp
  PlotXY.cpp
  PlotXY.hpp
  Wizard.hpp
  Wizard.cpp
  LibSolver.hpp
  LibSolver.cpp
  Tags.hpp
  Tags.cpp
  TimeStepping.hpp
  TimeStepping.cpp
  TimeStepComputer.hpp
  TimeStepComputer.cpp
)

list(APPEND coolfluid_solver_libs 
  coolfluid_physics 
  coolfluid_mesh 
  coolfluid_mesh_lagrangep1 
  coolfluid_mesh_gmsh 
  coolfluid_mesh_neu
)

if( coolfluid_mesh_cgns_builds )
  list(APPEND coolfluid_solver_libs coolfluid_mesh_cgns )
endif()

coolfluid3_add_library( TARGET   coolfluid_solver 
                        KERNEL
                        SOURCES  ${coolfluid_solver_files}
                        LIBS     ${coolfluid_solver_libs}
                                 ${TRILINOS_LIBRARIES}
                        INCLUDES ${TRILINOS_INCLUDE_DIRS})

add_subdirectory( actions ) # actions the solver uses
