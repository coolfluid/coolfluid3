
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
  NetworkException.hpp)

# Use Qt Libraries
LIST( APPEND Network_libs ${QT_LIBRARIES} )
LIST( APPEND Network_cflibs Common)

CF_ADD_LIBRARY(Network)
