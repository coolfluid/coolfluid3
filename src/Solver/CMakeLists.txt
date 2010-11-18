coolfluid_find_orphan_files()

list( APPEND coolfluid-solver_files  coolfluid-solver.cpp  )

list(APPEND coolfluid-solver_libs coolfluid_neutral coolfluid_gmsh coolfluid_rdm )

coolfluid_add_application( coolfluid-solver )

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET coolfluid-solver
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../Mesh/uTests/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})

list( APPEND coolfluid_rdm_files
  ForAllRegions.hpp
  CDomain.hpp
  CDomain.cpp
  CForAllElements.hpp
  CForAllElements.cpp
  CForAllNodes.hpp
  CForAllNodes.cpp
  CModel.hpp
  CModel.cpp
  CPhysicalModel.cpp
  CPhysicalModel.hpp
  LibSolver.hpp
  LibSolver.cpp
  CDiscretization.hpp
  CDiscretization.cpp
  CIterativeSolver.hpp
  CIterativeSolver.cpp
  CMethod.hpp
  CMethod.cpp
  ForwardEuler.hpp
  ForwardEuler.cpp
  ResidualDistribution.hpp
  ResidualDistribution.cpp
  ScalarAdvection.hpp
  ScalarAdvection.cpp
)

set( coolfluid_rdm_kernellib TRUE )

list(APPEND coolfluid_rdm_libs coolfluid_mesh coolfluid_mesh_sf )

coolfluid_add_library( coolfluid_rdm )

add_subdirectory(uTests)
