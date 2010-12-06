list( APPEND coolfluid_solver_files
  CModel.hpp
  CModel.cpp
  CPhysicalModel.cpp
  CPhysicalModel.hpp
  CTakeStep.hpp
  CTakeStep.cpp
  CDiscretization.hpp
  CDiscretization.cpp
  CIterativeSolver.hpp
  CIterativeSolver.cpp
  CSchemeLDA.hpp
  CSchemeLDA.cpp
  CSchemeLDAT.hpp
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

list(APPEND coolfluid_solver_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_gmsh coolfluid_neutral coolfluid_actions)

if( CF_HAVE_CGNS )
  list(APPEND coolfluid_solver_cflibs coolfluid_cgns3 )
endif()

coolfluid_add_library( coolfluid_solver )

add_subdirectory(uTests)
