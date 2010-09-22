
# Add the project files
list( APPEND Network_files
  ComponentNames.hpp
  ComponentType.cpp
  ComponentType.hpp
  HostInfos.cpp
  HostInfos.hpp
  LibNetwork.cpp
  LibNetwork.hpp
  LogMessage.cpp
  LogMessage.hpp
  NetworkException.cpp
  NetworkException.hpp)

# Use Qt Libraries
list( APPEND Network_libs ${QT_LIBRARIES} )
list( APPEND Network_cflibs Common)

coolfluid_add_library(Network)
