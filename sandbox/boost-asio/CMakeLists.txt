list( APPEND sandbox-boost-asio-server_libs coolfluid_common ${PTHREAD_LIBRARIES} )
list( APPEND sandbox-boost-asio-server_files server.cpp
                                              TCPConnection.hpp
                                              TCPConnection.cpp
                                              ErrorHandler.hpp
                                              ErrorHandler.cpp )

set( sandbox-boost-asio-server_sandbox_app TRUE )

coolfluid_add_application( sandbox-boost-asio-server )

list( APPEND sandbox-boost-asio-client_libs coolfluid_common ${PTHREAD_LIBRARIES} )
list( APPEND sandbox-boost-asio-client_files client.cpp
                                              TCPConnection.hpp
                                              TCPConnection.cpp
                                              ErrorHandler.hpp
                                              ErrorHandler.cpp )

set( sandbox-boost-asio-client_sandbox_app TRUE )

coolfluid_add_application( sandbox-boost-asio-client )

