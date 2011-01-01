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
  CTakeStep.hpp
  CTakeStep.cpp
  CDiscretization.hpp
  CDiscretization.cpp
  CEigenLSS.cpp
  CEigenLSS.hpp
  CIterativeSolver.hpp
  CIterativeSolver.cpp
  CSchemeLDA.hpp
  CSchemeLDA.cpp
  CSchemeLDAT.hpp
  CTime.hpp
  CTime.cpp
  CMethod.hpp
  CMethod.cpp
  ForwardEuler.hpp
  ForwardEuler.cpp
  LoadMesh.hpp
  LoadMesh.cpp
  LibSolver.hpp
  LibSolver.cpp
  ResidualDistribution.hpp
  ResidualDistribution.cpp
  ScalarAdvection.hpp
  ScalarAdvection.cpp
)

set( coolfluid_solver_kernellib TRUE )

list(APPEND coolfluid_solver_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_mesh_gmsh coolfluid_mesh_neutral coolfluid_actions)

if( CF_HAVE_CGNS )
  list(APPEND coolfluid_solver_cflibs coolfluid_mesh_cgns3 )
endif()

coolfluid_add_library( coolfluid_solver )
