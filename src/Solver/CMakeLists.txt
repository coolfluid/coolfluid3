CF_FIND_ORPHAN_FILES()

LIST( APPEND coolfluid-solver_files
  coolfluid-solver.cpp
  ForAllRegions.hpp
  ComputeVolumes.hpp
)

LIST (APPEND coolfluid-solver_libs Neu P1)

CF_ADD_APP (coolfluid-solver)

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET coolfluid-solver
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../Mesh/uTests/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
