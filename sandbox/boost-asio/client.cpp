// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/asio/io_service.hpp> // main I/O service
#include <boost/asio/ip/tcp.hpp>     // TCP services
#include <boost/asio/placeholders.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "common/Signal.hpp"

#include "common/XML/FileOperations.hpp"
#include "common/XML/SignalFrame.hpp"

#include "boost-asio/TCPConnection.hpp"

using namespace boost;
using namespace boost::asio::ip;

using namespace cf3::common;

//////////////////////////////////////////////////////////////////////////////

class TCPClient
{
public:

  TCPClient( asio::io_service& io_service, tcp::endpoint& endpoint)
    : m_io_service (io_service)
  {
    // Try to connect to the server
    connect(endpoint);
  }

private:

  void connect(tcp::endpoint& endpoint)
  {
    TCPConnection::Ptr new_connection = TCPConnection::create(m_io_service);
    tcp::socket& socket = new_connection->socket();

    socket.async_connect( endpoint,
                          boost::bind( &TCPClient::handle_connect,
                                       this,
                                       new_connection,
                                       asio::placeholders::error )
                         );
  }

  void handle_connect( TCPConnection::Ptr new_connection, const system::error_code & error)
  {
    if ( !error )
    {
      std::cout << "Connected to " << new_connection->socket().remote_endpoint() << std::endl;
      new_connection->read( m_args,
                            boost::bind( &TCPClient::handle_read,
                                         this,
                                         m_args,
                                         asio::placeholders::error )
                            );
    }
  }

  void handle_read( SignalArgs & args, const system::error_code & error )
  {
    if( error )
      CFerror << "Could not read: " << error.message() << CFendl;
    else
    {
      try
      {
        std::string str;

        XML::to_string( args.options().main_map.content, str );

        std::cout << str << std::endl ;

        std::string message = args.options().value<std::string>("options");

        std::cout << message << std::endl;

      }
      catch( Exception & e )
      {
        std::cerr << e.what() << std::endl;
      }
    }
  }

  SignalArgs m_args;
  asio::io_service & m_io_service;
};

//////////////////////////////////////////////////////////////////////////////

int main()
{
  try
  {
    asio::io_service io_service;
    tcp::endpoint endpoint(asio::ip::address::from_string("127.0.0.1"), 7171);

    TCPClient client(io_service, endpoint);
    io_service.run();
    std::cout << "Finished" << std::endl;
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
