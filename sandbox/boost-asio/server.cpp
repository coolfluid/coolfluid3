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

#include "boost-asio/TCPConnection.hpp"

#include "common/CF.hpp" // for cf3::Uint

#include "common/XML/SignalFrame.hpp"

using namespace boost;
using namespace boost::asio::ip;

using namespace cf3::common;
using namespace cf3::common::XML;

//////////////////////////////////////////////////////////////////////////////

class TCPServer
{
public:

  TCPServer(asio::io_service& io_service, int port)
    : m_acceptor(io_service, tcp::endpoint(tcp::v4(), port))
  {
    init_accept();
  }

private: // functions

  void init_accept()
  {
    m_connection = TCPConnection::create(m_acceptor.get_io_service());

    m_acceptor.async_accept( m_connection->socket(),
                             boost::bind( &TCPServer::callback_accept,
                                          this,
                                          asio::placeholders::error));
  }

  /////////////////////////////////////////////////////////////////////////////

  void init_read ()
  {
    m_connection->read( m_args,
                        boost::bind( &TCPServer::callback_read,
                                     this,
                                     asio::placeholders::error )
                        );
  }

  /////////////////////////////////////////////////////////////////////////////

  void init_send ( SignalFrame & frame )
  {
    m_connection->send ( frame,
                         boost::bind( &TCPServer::callback_send,
                                      this,
                                      asio::placeholders::error
                                     )
                        );
  }

  /////////////////////////////////////////////////////////////////////////////

  void callback_accept( const system::error_code& error )
  {
    if (!error)
    {
      SignalFrame frame("message", "cpath:/", "cpath:/");

      std::cout << "New client connected from " << m_connection->socket().remote_endpoint() << std::endl;

      frame.set_option<std::string>("text", std::string("Welcome to the server!") );

      init_read(); // start listening process

      init_send(frame); // send data

      init_accept(); // wait for other clients
    }
    else
      std::cerr << "Failed to open a network connection: " << error.message() << std::endl;
  }

  /////////////////////////////////////////////////////////////////////////////

  void callback_send( const system::error_code & error )
  {
    if( error )
      CFerror << "Data could not be sent: " << error.message() << CFendl;
    else
      CFinfo << "Message sent" << CFendl;
  }

  /////////////////////////////////////////////////////////////////////////////

  void callback_read( const system::error_code & error )
  {
    if( error )
      CFerror << "Could not read: " << error.message() << CFendl;
    else
    {
      try
      {
        std::string message = m_args.options().value<std::string>("text");

//        std::cout << message << std::endl;

        algorithm::to_upper(message);

        SignalFrame reply = m_args.create_reply();

        reply.set_option<std::string>( "text", message );

        init_send( m_args );

        std::cout << m_connection->socket().is_open() << std::endl;
        init_read();
      }
      catch( Exception & e )
      {
        std::cerr << e.what() << std::endl;
      }
    }
  }

private: // data

  SignalFrame m_args;
  TCPConnection::Ptr m_connection;
  tcp::acceptor m_acceptor;

}; // TCPServer

//////////////////////////////////////////////////////////////////////////////

int main()
{
  asio::io_service io_service;

  try
  {
    TCPServer server(io_service, 62784);
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
