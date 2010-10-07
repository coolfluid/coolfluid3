coolfluid_find_orphan_files()

list( APPEND coolfluid-solver_files
  coolfluid-solver.cpp
)

list(APPEND coolfluid-solver_libs Neu Gmsh Solver )

coolfluid_add_application(coolfluid-solver)

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET coolfluid-solver
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../Mesh/uTests/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})

list( APPEND Solver_files
  ForAllRegions.hpp
  CDomain.hpp
  CDomain.cpp
  CForAllElements.hpp
  CForAllElements.cpp
  CForAllNodes.hpp
  CForAllNodes.cpp
  CModel.hpp
  CModel.cpp
  LibSolver.hpp
  LibSolver.cpp
  CDiscretization.hpp
  CDiscretization.cpp
  CIterativeSolver.hpp
  CIterativeSolver.cpp
  CMethod.hpp
  CMethod.cpp
)

set( Solver_kernellib TRUE )

list(APPEND Solver_libs Mesh SF )

coolfluid_add_library(Solver)

