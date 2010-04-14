
# Add the project files
LIST ( APPEND Network_files
  HostInfos.cpp
  HostInfos.hpp
  NetworkAPI.hpp
  NetworkException.cpp
  NetworkException.hpp
  NetworkFrameType.hpp
  NetworkProtocol.cpp
  NetworkProtocol.hpp)

# Use Qt Libraries
LIST( APPEND Network_libs Common ${QT_LIBRARIES} )

CF_ADD_LIBRARY(Network)
