list( APPEND coolfluid-command_files  coolfluid-command.cpp  )

list(APPEND coolfluid-command_cflibs coolfluid_mesh_neutral coolfluid_mesh_gmsh coolfluid_solver )

coolfluid_add_application( coolfluid-command )

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET coolfluid-command
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
