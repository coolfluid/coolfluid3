list( APPEND coolfluid-command_files  coolfluid-command.cpp  )

list( APPEND coolfluid-command_cflibs ${CF3_KERNEL_LIBS} coolfluid_shell )

coolfluid_add_application( coolfluid-command )

# store the path for the acceptance tests
set( coolfluid-command_path "${CMAKE_CURRENT_BINARY_DIR}/coolfluid-command" CACHE "Full path to coolfluid-command" INTERNAL )
mark_as_advanced( coolfluid-command_path )

