// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/bind/bind.hpp>

#include <boost/asio/placeholders.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>


#include "boost-asio/TCPConnection.hpp"

using namespace boost;
using namespace boost::asio::ip;

//////////////////////////////////////////////////////////////////////////////

TCPConnection::Ptr TCPConnection::create(asio::io_service& ios )
{
  return Ptr(new TCPConnection(ios));
}

/////////////////////////////////////////////////////////////////////////////

void TCPConnection::send( const std::string & message )
{
  asio::async_write(m_socket, asio::buffer(message),
                    boost::bind(&TCPConnection::handle_frame_sent, shared_from_this(),
                                asio::placeholders::error)
                    );
}

//////////////////////////////////////////////////////////////////////////////

void TCPConnection::read( int timeout )
{
  boost::asio::async_read(m_socket, asio::buffer(m_network_buffer),
                          boost::asio::transfer_at_least(20),
                          bind(&TCPConnection::handle_frame_read, shared_from_this(),
                               asio::placeholders::error,
                               asio::placeholders::bytes_transferred)
                          );

  if( timeout > 0 )
  {
    timer.expires_from_now(posix_time::seconds(timeout));
    timer.async_wait(boost::bind(&TCPConnection::close, shared_from_this() ));
  }
}

//////////////////////////////////////////////////////////////////////////////

TCPConnection::TCPConnection(asio::io_service& io_service)
  : m_socket(io_service),
    timer(io_service, posix_time::seconds(5)) // Starts counting on creation
{
}

//////////////////////////////////////////////////////////////////////////////

void TCPConnection::handle_frame_sent(const system::error_code& error)
{
  if (!error)
  {
    read(); // (2)
  }
  else
  {
    std::cout << error.message() << std::endl;
  }
}

//////////////////////////////////////////////////////////////////////////////

void TCPConnection::handle_frame_read(const boost::system::error_code& error, size_t n)
{
  if (!error)
  {
    std::cout.write(&m_network_buffer[0], n );
    read();
  }
  else
  {
    std::cout << error.message() ;
  }
}

//////////////////////////////////////////////////////////////////////////////

void TCPConnection::close()
{
  std::cout << "Timeout! Kicking off " << m_socket.remote_endpoint() << std::endl;
  m_socket.close();
}

