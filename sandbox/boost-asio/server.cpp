// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>


#include "common/CF.hpp" // for cf3::Uint

using namespace boost;
using namespace boost::asio::ip;

//////////////////////////////////////////////////////////////////////////////

/// Manages the communication with ONE client. Each client wil have its
/// TCPConnection instance.
class TCPConnection : public boost::enable_shared_from_this<TCPConnection>
{
public:
  typedef boost::shared_ptr<TCPConnection> Ptr;
  typedef boost::shared_ptr<TCPConnection const> ConstPtr;

  static Ptr create(asio::io_service& ios)
  {
    Ptr new_connection(new TCPConnection(ios) );
    return new_connection;
  }

  tcp::socket& socket()
  {
    return m_socket;
  }

  void do_read()
  {
    // Start listening
    asio::async_read(m_socket, asio::buffer(m_buffer),
                     boost::bind(&TCPConnection::handle_read, shared_from_this(),
                                 asio::placeholders::error)
                     );
    timer.expires_from_now(posix_time::seconds(5));
    timer.async_wait(boost::bind(&TCPConnection::close, shared_from_this() ));
  }


  void start()
  {
    m_message = "Welcome on the server!";

    asio::async_write(m_socket, asio::buffer(m_message),
                      boost::bind(&TCPConnection::handle_write, shared_from_this(),
                                  asio::placeholders::error)
                      );
  }

private:
  TCPConnection(asio::io_service& io_service)
    : m_socket(io_service),
      timer(io_service, posix_time::seconds(5)) // Starts counting on creation
  {
  }

  void handle_write(const system::error_code& error)
  {
    if (!error)
    {
      do_read(); // (2)
    }
    else {
      std::cout << error.message() << std::endl;
    }
  }

  void handle_read(const system::error_code& error)
  {
    if (!error)
    {
      // Listen again
      do_read();
    }
    else
    {
      close();
    }
  }

  void close()
  {
    std::cout << "Timeout! Kicking off " << m_socket.remote_endpoint() << std::endl;
    m_socket.close();
  }

  asio::deadline_timer timer;
  tcp::socket m_socket;
  std::string m_message;
  boost::array<char, 128> m_buffer;
};

//////////////////////////////////////////////////////////////////////////////

class TCPServer
{
public:
  TCPServer(asio::io_service& io_service, int port)
    : m_acceptor(io_service, tcp::endpoint(tcp::v4(), port))
  {
    start_accept();
  }

private:

  void start_accept()
  {
    TCPConnection::Ptr new_connection = TCPConnection::create(m_acceptor.io_service());

    m_acceptor.async_accept(new_connection->socket(),
                            boost::bind(&TCPServer::handle_accept, this, new_connection,
                                        asio::placeholders::error));
  }

  void handle_accept(TCPConnection::Ptr new_connection, const system::error_code& error) // (4)
  {
    if (!error)
    {
      std::cout << "New client connected from " << new_connection->socket().remote_endpoint() << std::endl;
      new_connection->start();
      start_accept();
    }
  }


  tcp::acceptor m_acceptor;
};

//////////////////////////////////////////////////////////////////////////////

int main()
{
  try
  {
    asio::io_service io_service;

    TCPServer server(io_service, 7171);
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
