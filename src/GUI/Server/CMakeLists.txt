#
# Compile Qt classes (with slots/signals)
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# List of classes to "moc"
SET ( coolfluid-server_moc_classes
  ServerNetworkComm.hpp
  CSimulator.hpp
  RemoteClientAppender.hpp
  CCore.hpp
  SimulationWorker.hpp
  SimulationManager.hpp
  MPIListener.hpp)

# Create the moc files
QT4_WRAP_CPP(coolfluid-server_moc_files ${coolfluid-server_moc_classes})

# Add the project files (including the moc files)
LIST ( APPEND coolfluid-server_files
  MPIListener.cpp
  MPIListener.hpp
  MPIListeningInfo.cpp
  MPIListeningInfo.hpp
  MPIReceiver.cpp
  MPIReceiver.hpp
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
# TARGET_LINK_LIBRARIES( app_server network ${QT_LIBRARIES} ${CF_KERNEL_LIBS} ${CF_KERNEL_STATIC_LIBS} ${MPI_LIBRARIES} ${CF_Boost_LIBRARIES} )

LIST( APPEND coolfluid-server_libs Network ${QT_LIBRARIES})

CF_ADD_APP(coolfluid-server)

CONFIGURE_FILE ( Tree.xml      ${CMAKE_CURRENT_BINARY_DIR}/Tree.xml       @ONLY )
CONFIGURE_FILE ( TypesList.xml ${CMAKE_CURRENT_BINARY_DIR}/TypesList.xml  @ONLY )
CONFIGURE_FILE (machine.txt ${CMAKE_CURRENT_BINARY_DIR}/machine.txt  @ONLY)

# scripts to start the server
CONFIGURE_FILE ( start_coolfluid_server.sh ${CMAKE_CURRENT_BINARY_DIR}/start_coolfluid_server.sh  @ONLY )
CONFIGURE_FILE ( check_coolfluid_server.sh ${CMAKE_CURRENT_BINARY_DIR}/check_coolfluid_server.sh  @ONLY )

CONFIGURE_FILE ( fsm_advect_wave.CFcase ${CMAKE_CURRENT_BINARY_DIR}/fsm_advect_wave.CFcase @ONLY )
CONFIGURE_FILE ( advect_wave.thor ${CMAKE_CURRENT_BINARY_DIR}/advect_wave.thor @ONLY )
CONFIGURE_FILE ( advect_wave.SP ${CMAKE_CURRENT_BINARY_DIR}/advect_wave.SP @ONLY )

CF_MARK_NOT_ORPHAN(ServerSimulation.cpp)
