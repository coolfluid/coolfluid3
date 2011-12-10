
# List of classes to "moc"
set( coolfluid_ui_server_moc_classes
  CCore.hpp
  ServerNetworkComm.hpp
  ServerRoot.hpp )

# Create the moc files
QT4_WRAP_CPP(coolfluid_ui_server_moc_files ${coolfluid_ui_server_moc_classes})

# Add the project files (including the moc files)
list( APPEND coolfluid_ui_server_files
  CCore.cpp
  CCore.hpp
  ProcessingThread.cpp
  ProcessingThread.hpp
  ServerExceptions.hpp
  ServerExceptions.cpp
  ServerNetworkComm.cpp
  ServerNetworkComm.hpp
  ServerRoot.cpp
  ServerRoot.hpp )

# Use Qt Libraries
#list( APPEND coolfluid_ui_server_libs ${QT_LIBRARIES} )

list( APPEND coolfluid_ui_server_cflibs coolfluid_ui_network coolfluid_ui_common )

list( APPEND coolfluid_ui_server_libs ${QT_LIBRARIES} )

coolfluid_add_library(coolfluid_ui_server)

configure_file( machine.txt   ${CMAKE_CURRENT_BINARY_DIR}/machine.txt    @ONLY)
