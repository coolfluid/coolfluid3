list(APPEND coolfluid_cli_files
  LibCommandLineInterpreter.cpp
  LibCommandLineInterpreter.hpp
  CommandLineInterpreter.cpp
  CommandLineInterpreter.hpp
  BasicCommands.hpp
  BasicCommands.cpp
)

list( APPEND coolfluid_cli_cflibs coolfluid_common )

coolfluid_add_library( coolfluid_cli )
