
# Add the project files
LIST ( APPEND Network_files
  ComponentNames.hpp
  ComponentType.cpp
  ComponentType.hpp
  HostInfos.cpp
  HostInfos.hpp
  LogMessage.cpp
  LogMessage.hpp
  NetworkAPI.hpp
  NetworkException.cpp
  NetworkException.hpp
  NetworkFrameType.hpp
  NetworkProtocol.cpp
  NetworkProtocol.hpp
  SignalInfo.cpp
  SignalInfo.hpp)

# Use Qt Libraries
LIST( APPEND Network_libs Common ${QT_LIBRARIES} )

CF_ADD_LIBRARY(Network)
