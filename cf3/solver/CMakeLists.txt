coolfluid_find_orphan_files()

list( APPEND coolfluid_solver_files
  Action.hpp
  Action.cpp
  ActionDirector.hpp
  ActionDirector.cpp
  Model.hpp
  Model.cpp
  ModelSteady.hpp
  ModelSteady.cpp
  ModelUnsteady.hpp
  ModelUnsteady.cpp
  EigenLSS.cpp
  EigenLSS.hpp
  SimpleSolver.hpp
  SimpleSolver.cpp
  Solver.hpp
  Solver.cpp
  Time.hpp
  Time.cpp
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
)

set( coolfluid_solver_kernellib TRUE )

list(APPEND coolfluid_solver_cflibs coolfluid_physics coolfluid_mesh coolfluid_mesh_lagrangep1 coolfluid_mesh_gmsh coolfluid_mesh_neu)

if( CF3_HAVE_TRILINOS )
  list(APPEND coolfluid_solver_cflibs ${TRILINOS_LIBRARIES})
  list(APPEND coolfluid_solver_includedirs ${TRILINOS_INCLUDE_DIRS})
elseif( CF3_HAVE_SUPERLU )
  list(APPEND coolfluid_solver_cflibs ${SUPERLU_LIBRARIES})
  list(APPEND coolfluid_solver_includedirs ${SUPERLU_INCLUDE_DIRS})
endif()

if( coolfluid_mesh_cgns3_builds )
  list(APPEND coolfluid_solver_cflibs coolfluid_mesh_cgns3 )
endif()

coolfluid_add_library( coolfluid_solver )

add_subdirectory( actions ) # actions the solver uses
