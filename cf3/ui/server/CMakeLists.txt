
# Add the project files (including the moc files)
list( APPEND coolfluid_ui_server_files
  CCore.cpp
  CCore.hpp
  ServerExceptions.hpp
  ServerExceptions.cpp
  ServerNetworkComm.cpp
  ServerNetworkComm.hpp
  ServerRoot.cpp
  ServerRoot.hpp )

# Use Qt Libraries
#list( APPEND coolfluid_ui_server_libs ${QT_LIBRARIES} )

list( APPEND coolfluid_ui_server_cflibs coolfluid_ui_network coolfluid_ui_common )

coolfluid_add_library(coolfluid_ui_server)

configure_file( machine.txt   ${CMAKE_CURRENT_BINARY_DIR}/machine.txt    @ONLY)
