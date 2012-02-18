// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/iterator/iterator_concepts.hpp>

#include "common/XML/FileOperations.hpp"
#include "common/XML/SignalFrame.hpp"

#include "boost-asio/ErrorHandler.hpp"
#include "boost-asio/TCPConnection.hpp"

using namespace boost;
using namespace boost::asio::ip;
using namespace cf3::common;
using namespace cf3::common::XML;

//////////////////////////////////////////////////////////////////////////////

class TCPServer
{
public: // nested structs

  struct ClientInfo
  {

  public:
    TCPConnection::Ptr connection;

    SignalFrame buffer;

    boost::shared_ptr<ErrorHandler> error_handler;

  }; // ClientInfo


public:

  TCPServer( asio::io_service& io_service, int port ) :
      m_acceptor( io_service, tcp::endpoint( tcp::v4(), port ) )
  {
    init_accept();
  }

private: // functions

  void init_accept()
  {
    TCPConnection::Ptr conn = TCPConnection::create( m_acceptor.get_io_service() );

    m_acceptor.async_accept( conn->socket(),
                             boost::bind( &TCPServer::callback_accept,
                                          this,
                                          conn,// 1st param for the callback fct
                                          asio::placeholders::error ) ); // 2nd param
  }

  /////////////////////////////////////////////////////////////////////////////

  void init_read( TCPConnection::Ptr conn, SignalArgs & buffer )
  {
    conn->read( buffer,
                boost::bind( &TCPServer::callback_read,
                             this,
                             conn, // 1st param for the callback fct
                             asio::placeholders::error ) // 2nd param
              );
  }

  /////////////////////////////////////////////////////////////////////////////

  void init_send( TCPConnection::Ptr conn, SignalFrame & buffer )
  {
    conn->send( buffer,
                boost::bind( &TCPServer::callback_send,
                             this,
                             conn, // 1st param for the callback fct
                             asio::placeholders::error // 2nd param
                           )
              );
  }

  /////////////////////////////////////////////////////////////////////////////

  void callback_accept( TCPConnection::Ptr conn, const boost::system::error_code& error )
  {
    if ( !error )
    {
      ClientInfo& info = m_clients[conn];

      info.connection = conn;
      info.buffer = SignalFrame( "message", "cpath:/", "cpath:/" );
      info.error_handler = boost::shared_ptr<ErrorHandler>(new ErrorHandler());

      info.connection->set_error_handler(info.error_handler);

      std::cout << "New client connected from " << conn->socket().remote_endpoint()
                << std::endl;

      info.buffer.set_option( "text", std::string( "Welcome to the server!" ) );

      init_read( conn, info.buffer ); // start listening process

      init_send( conn, info.buffer ); // send data

      init_accept(); // wait for other clients
    }
    else
      std::cerr << "Failed to open a network connection: " << error.message() << std::endl;
  }

  /////////////////////////////////////////////////////////////////////////////

  void callback_send( TCPConnection::Ptr conn, const boost::system::error_code & error )
  {
    if ( error )
      std::cerr << "Data could not be sent to [" << conn->socket().remote_endpoint()
      << "]: " << error.message() << std::endl;
    else
      std::cout << "Message sent" << std::endl;
  }

  /////////////////////////////////////////////////////////////////////////////

  void callback_read( TCPConnection::Ptr conn, const boost::system::error_code & error )
  {
    std::map<TCPConnection::ConstPtr, ClientInfo>::iterator it = m_clients.find( conn );

    if ( it == m_clients.end() )
      throw BadValue( FromHere(), "Received message from unknown connection." );

    if ( error == asio::error::eof )
    {
      std::cout << "Client [" << conn->socket().remote_endpoint() << "] disconnected" << std::endl;
      m_clients.erase(conn);
    }
    else if ( error )
      std::cerr << "Could not read from [" << conn->socket().remote_endpoint()
      << "]: " << error.message() << std::endl;
    else
    {
      try
      {
        SignalFrame & buffer = it->second.buffer;

        std::string str;
        XML::to_string(*buffer.xml_doc.get(), str);

        // get the message and put it to upper case
        std::string message = buffer.options().value<std::string>( "text" );
        algorithm::to_upper( message );

        // create the reply and add the string
        SignalFrame reply = buffer.create_reply();
        reply.set_option<std::string>( "text", message );

        // send back the modified frame and initiate a new read operation
        init_send( conn, buffer );
        init_read( conn, buffer );
      }
      catch ( cf3::common::Exception & e )
      {
        std::cerr << e.what() << std::endl;
      }
    }
  }

private: // data

  std::map<TCPConnection::ConstPtr, ClientInfo> m_clients;
  tcp::acceptor m_acceptor;

}; // TCPServer

//////////////////////////////////////////////////////////////////////////////

int main()
{
  asio::io_service io_service;

  try
  {
    TCPServer server( io_service, 62784 );
    io_service.run();
  }
  catch ( std::exception& e )
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
