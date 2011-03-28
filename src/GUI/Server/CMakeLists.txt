
# List of classes to "moc"
set( coolfluid-server_moc_classes
  CCore.hpp
  Notifier.hpp
  RemoteClientAppender.hpp
  ServerNetworkComm.hpp
  ServerRoot.hpp )

# Create the moc files
QT4_WRAP_CPP(coolfluid-server_moc_files ${coolfluid-server_moc_classes})

# Add the project files (including the moc files)
list( APPEND coolfluid-server_files
  CCore.cpp
  CCore.hpp
  main.cpp
  Notifier.cpp
  Notifier.hpp
  ProcessingThread.cpp
  ProcessingThread.hpp
  RemoteClientAppender.cpp
  RemoteClientAppender.hpp
  ServerExceptions.hpp
  ServerExceptions.cpp
  ServerNetworkComm.cpp
  ServerNetworkComm.hpp
  ServerRoot.cpp
  ServerRoot.hpp )

# Use Qt Libraries
list( APPEND coolfluid-server_libs ${QT_LIBRARIES} )
list( APPEND coolfluid-server_cflibs ${CF_KERNEL_LIBS} )

coolfluid_add_application(coolfluid-server)

configure_file( machine.txt   ${CMAKE_CURRENT_BINARY_DIR}/machine.txt    @ONLY)
