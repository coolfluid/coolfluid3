list(APPEND coolfluid_shell_files
  LibShell.hpp
  LibShell.cpp
  Interpreter.cpp
  Interpreter.hpp
  BasicCommands.hpp
  BasicCommands.cpp
)

coolfluid3_add_library( TARGET  coolfluid_shell
                        KERNEL
                        SOURCES ${coolfluid_shell_files}
                        LIBS    coolfluid_common )
