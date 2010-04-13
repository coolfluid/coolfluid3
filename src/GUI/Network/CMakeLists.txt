
# Add the project files
LIST ( APPEND Network_files 
  HostInfos.cxx
  HostInfos.hh
  IOException.cxx
  IOException.hh
  MalformedDataException.cxx
  MalformedDataException.hh
  NetworkAPI.hh
  NetworkException.cxx
  NetworkException.hh
  NetworkFrameType.hh
  NetworkProtocol.cxx
  NetworkProtocol.hh)
 
# Use Qt Libraries
LIST( APPEND Network_libs Common ${QT_LIBRARIES} )

CF_ADD_LIBRARY(Network)
