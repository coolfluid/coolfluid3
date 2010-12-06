#
# Compile Qt classes (with slots/signals)
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# List of classes to "moc"
set( coolfluid-server_moc_classes
  ServerNetworkComm.hpp
  RemoteClientAppender.hpp
  CCore.hpp
  Notifier.hpp
#  SimulationWorker.hpp
  SimulationManager.hpp
#  MPIListener.hpp
  ServerRoot.hpp
  )

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
  ServerNetworkComm.cpp
  ServerNetworkComm.hpp
  ServerRoot.cpp
  ServerRoot.hpp
  SimulationManager.cpp
  SimulationManager.hpp
  TypesNotFoundException.cpp
  TypesNotFoundException.hpp
  UnknownClientIdException.cpp
  UnknownClientIdException.hpp)

# Use Qt Libraries
list( APPEND coolfluid-server_libs ${QT_LIBRARIES} )
list( APPEND coolfluid-server_cflibs ${CF_KERNEL_LIBS} coolfluid_network coolfluid_solver )

coolfluid_add_application(coolfluid-server)

configure_file( Tree.xml      ${CMAKE_CURRENT_BINARY_DIR}/Tree.xml       @ONLY )
configure_file( TypesList.xml ${CMAKE_CURRENT_BINARY_DIR}/TypesList.xml  @ONLY )
configure_file( machine.txt   ${CMAKE_CURRENT_BINARY_DIR}/machine.txt    @ONLY)

coolfluid_mark_not_orphan(  MPIListener.cpp
  MPIListener.hpp
  MPIListeningInfo.cpp
  MPIListeningInfo.hpp
  SimulationWorker.cpp
  SimulationWorker.hpp)
