#
# Compile Qt classes (with slots/signals)
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# List of classes to "moc"
set( coolfluid-server_moc_classes
  ServerNetworkComm.hpp
  CSimulator.hpp
  RemoteClientAppender.hpp
  CCore.hpp
  SimulationWorker.hpp
  SimulationManager.hpp
  MPIListener.hpp
  ServerRoot.hpp
  )

# Create the moc files
QT4_WRAP_CPP(coolfluid-server_moc_files ${coolfluid-server_moc_classes})

# Add the project files (including the moc files)
list( APPEND coolfluid-server_files
  MPIListener.cpp
  MPIListener.hpp
  MPIListeningInfo.cpp
  MPIListeningInfo.hpp
  RemoteClientAppender.cpp
  RemoteClientAppender.hpp
  CCore.cpp
  CCore.hpp
  ServerNetworkComm.cpp
  ServerNetworkComm.hpp
  ServerRoot.cpp
  ServerRoot.hpp
  CSimulator.cpp
  CSimulator.hpp
  ProcessingThread.cpp
  ProcessingThread.hpp
  SimulationManager.cpp
  SimulationManager.hpp
  SimulationWorker.cpp
  SimulationWorker.hpp
  TypesNotFoundException.cpp
  TypesNotFoundException.hpp
  UnknownClientIdException.cpp
  UnknownClientIdException.hpp
  main.cpp)

# Use Qt Libraries
list( APPEND coolfluid-server_libs ${QT_LIBRARIES} )
list( APPEND coolfluid-server_cflibs ${CF_KERNEL_LIBS} Network )

coolfluid_add_application(coolfluid-server)

configure_file( Tree.xml      ${CMAKE_CURRENT_BINARY_DIR}/Tree.xml       @ONLY )
configure_file( TypesList.xml ${CMAKE_CURRENT_BINARY_DIR}/TypesList.xml  @ONLY )
configure_file(machine.txt ${CMAKE_CURRENT_BINARY_DIR}/machine.txt  @ONLY)

# scripts to start the server
configure_file( start_coolfluid_server.sh ${CMAKE_CURRENT_BINARY_DIR}/start_coolfluid_server.sh  @ONLY )
configure_file( check_coolfluid_server.sh ${CMAKE_CURRENT_BINARY_DIR}/check_coolfluid_server.sh  @ONLY )

configure_file( fsm_advect_wave.CFcase ${CMAKE_CURRENT_BINARY_DIR}/fsm_advect_wave.CFcase @ONLY )
configure_file( advect_wave.thor ${CMAKE_CURRENT_BINARY_DIR}/advect_wave.thor @ONLY )
configure_file( advect_wave.SP ${CMAKE_CURRENT_BINARY_DIR}/advect_wave.SP @ONLY )

coolfluid_mark_not_orphan(ServerSimulation.cpp)
