list( APPEND coolfluid_ui_server_files
  CCore.cpp
  CCore.hpp
  ServerExceptions.hpp
  ServerExceptions.cpp
  ServerNetworkComm.cpp
  ServerNetworkComm.hpp
  ServerRoot.cpp
  ServerRoot.hpp )

coolfluid3_add_library( TARGET  coolfluid_ui_server
                       SOURCES
                            ${coolfluid_ui_server_files}
                       LIBS
                            coolfluid_ui_network coolfluid_ui_common )

configure_file( machine.txt ${CMAKE_CURRENT_BINARY_DIR}/machine.txt  @ONLY)
