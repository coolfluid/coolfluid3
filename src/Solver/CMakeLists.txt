coolfluid_find_orphan_files()

list( APPEND coolfluid_solver_files
  CModel.hpp
  CModel.cpp
  CModelSteady.hpp
  CModelSteady.cpp
  CModelUnsteady.hpp
  CModelUnsteady.cpp
  CPhysicalModel.cpp
  CPhysicalModel.hpp
  CDiscretization.hpp
  CDiscretization.cpp
  CEigenLSS.cpp
  CEigenLSS.hpp
  CSolver.hpp
  CSolver.cpp
  CTime.hpp
  CTime.cpp
  CMethod.hpp
  CMethod.cpp
  CPlotter.cpp
  CPlotter.hpp
  CPlotXY.cpp
  CPlotXY.hpp
  LibSolver.hpp
  LibSolver.cpp
)

set( coolfluid_solver_kernellib TRUE )

list(APPEND coolfluid_solver_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_mesh_gmsh coolfluid_mesh_neutral)
if( CF_HAVE_SUPERLU )
  list(APPEND coolfluid_solver_cflibs ${SUPERLU_LIBRARIES})
  list(APPEND coolfluid_solver_includedirs ${SUPERLU_INCLUDE_DIR})
endif( CF_HAVE_SUPERLU )

if( coolfluid_mesh_cgns3_builds )
  list(APPEND coolfluid_solver_cflibs coolfluid_mesh_cgns3 )
endif()

coolfluid_add_library( coolfluid_solver )

add_subdirectory( Actions ) # actions the solver uses
