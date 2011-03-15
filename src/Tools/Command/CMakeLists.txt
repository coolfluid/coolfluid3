list( APPEND coolfluid-command_files  coolfluid-command.cpp  )

list( APPEND coolfluid-command_cflibs ${CF_KERNEL_LIBS} coolfluid_shell )

coolfluid_add_application( coolfluid-command )

# store the path for the acceptance tests
get_property( coolfluid-command_path TARGET coolfluid-command PROPERTY LOCATION )
set( coolfluid-command_path ${coolfluid-command_path} CACHE "Full path to coolfluid-command" INTERNAL )
mark_as_advanced( coolfluid-command_path )

