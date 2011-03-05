list(APPEND coolfluid_shell_files
  LibShell.hpp
  LibShell.cpp
  Interpreter.cpp
  Interpreter.hpp
  BasicCommands.hpp
  BasicCommands.cpp
)

list( APPEND coolfluid_shell_cflibs coolfluid_common )

coolfluid_add_library( coolfluid_shell )
