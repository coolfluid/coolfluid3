list( APPEND coolfluid-command_files  coolfluid-command.cpp  )

list( APPEND coolfluid-command_cflibs ${CF_KERNEL_LIBS} coolfluid_shell )

coolfluid_add_application( coolfluid-command )
