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

#include "common/Log.hpp"

#include "common/XML/FileOperations.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/XML/XmlNode.hpp"

#include "boost-asio/ErrorHandler.hpp"
#include "boost-asio/TCPConnection.hpp"

using namespace boost;
using namespace boost::asio::ip;
using namespace cf3::common;
using namespace cf3::common::XML;

//////////////////////////////////////////////////////////////////////////////

class TCPClient
{

public:

  TCPClient( asio::io_service& io_service, tcp::endpoint& endpoint )
      : m_io_service( io_service ),
        m_error_handler(new ErrorHandler())
  {
    init_connect( endpoint );
  }

private: // functions

  void init_connect( tcp::endpoint& endpoint )
  {
    m_connection = TCPConnection::create( m_io_service );
    tcp::socket& socket = m_connection->socket();

    m_connection->set_error_handler(m_error_handler);

    socket.async_connect( endpoint,
                          boost::bind( &TCPClient::callback_connect,
                                       this,
                                       asio::placeholders::error )
                        );
  }

  /////////////////////////////////////////////////////////////////////////////

  void init_read()
  {
    m_connection->read( m_args,
                        boost::bind( &TCPClient::callback_read,
                                     this,
                                     asio::placeholders::error )
                       );
  }

  /////////////////////////////////////////////////////////////////////////////

  void init_send( SignalFrame & frame )
  {
    m_connection->send( frame,
                        boost::bind( &TCPClient::callback_send,
                                     this,
                                     asio::placeholders::error
                                   )
                       );
  }

  /////////////////////////////////////////////////////////////////////////////

  void callback_send( const boost::system::error_code & error )
  {
    if ( error )
      std::cerr << error.message() << std::endl;
    else
      std::cout << "Message sent" << std::endl;
  }

  /////////////////////////////////////////////////////////////////////////////

  void callback_connect( const boost::system::error_code & error )
  {
    if ( !error )
    {
      std::cout << "Connected to " << m_connection->socket().remote_endpoint() << std::endl;
      init_read();
    }

    else
      std::cerr << "Could not connect to host: " << error.message() << std::endl;
  }

  /////////////////////////////////////////////////////////////////////////////

  void callback_read( const boost::system::error_code & error )
  {
    if ( error )
      std::cerr << "Could not read: " << error.message() << std::endl;
    else
    {
      try
      {
        std::string message = m_args.options().value<std::string>( "text" );
        std::string text;

        if( m_args.has_reply() )
          message = m_args.get_reply().options().value<std::string>( "text" );

        std::cout << message << std::endl;

        while ( text.empty() )
        {
          std::cout << "String to capitalize: " << std::flush;
          std::getline( std::cin, text );
        }

        m_args = SignalFrame( "message", "cpath:/", "cpath:/" );

        m_args.set_option<std::string>( "text", text );

        init_send( m_args );

        init_read();
      }

      catch ( cf3::common::Exception & e )
      {
        std::cerr << e.what() << std::endl;
      }
    }
  }

private: // data

  TCPConnection::Ptr m_connection;
  SignalArgs m_args;
  asio::io_service & m_io_service;
  boost::shared_ptr<ErrorHandler> m_error_handler;

}; // TCPClient

//////////////////////////////////////////////////////////////////////////////

int main()
{
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionOutputs = false;

  try
  {
    asio::io_service io_service;
    tcp::endpoint endpoint( asio::ip::address::from_string( "127.0.0.1" ), 62784 );

    TCPClient client( io_service, endpoint );
    io_service.run();
    std::cout << "Finished" << std::endl;
  }

  catch ( std::exception& e )
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
