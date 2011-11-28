// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_sandbox_boost_asio_tcp_connection_hpp
#define cf3_sandbox_boost_asio_tcp_connection_hpp

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/enable_shared_from_this.hpp>

/// Manages a TCP connection between to entities where one is a
/// server and the other one is a client.@n

/// This class is intented to be used in an asynchronous network architecture.
/// Therefore, a @c TCPConnection object cannot live outside of a shared
/// pointer because a such object has to maintain asynchronous operations.
/// One cannot predict when all of those operations will be completed, using a
/// shared pointer garantees the connection stays alive until all operations
/// are done.@c

/// A TCP connection is based on an I/O service that handles the asynchronous
/// operations and calls an appropriate function when of those is completed.@n

/// This class can be used in both client and server applications.

/// @todo either allow a calling code to @c boost::bind a callback function for
/// completion of sending and reading operations or use the CF EnventHandler to
/// to propagate received data.

/// @author Quentin Gasper
class TCPConnection : public boost::enable_shared_from_this<TCPConnection>
{
public: // typdefs

  typedef boost::shared_ptr<TCPConnection> Ptr;
  typedef boost::shared_ptr<TCPConnection const> ConstPtr;

public:

  /// Creates a instance of @c TCPConnection.
  /// @param ios The I/O service the connection is based on.
  /// @return Returns a shared pointer containing the new instance.
  static Ptr create( boost::asio::io_service& ios );

  /// Gives a referemce to the internal socket.
  /// @return Returns a reference to the internal socket.
  boost::asio::ip::tcp::socket & socket() { return m_socket; }

  /// Gives a constant referemce to the internal socket.
  /// @return Returns a constant reference to the internal socket.
  const boost::asio::ip::tcp::socket & socket() const { return m_socket; }

  /// Sends a message to the remote entity.
  /// The message is sent asynchronously and the function returns directly,
  /// before the data is actually send.
  /// @param message The message to send.
  void send( const std::string & message );

  /// Initiates an asynchronous reading from the remote entity.
  /// The function returns directly.
  /// @param timeout Time is seconds after which the reading is stopped and
  /// considered to have failed. If less than 0, the reading will never time out.
  void read( int timeout = -1 );

private: // functions

  /// Constructor.
  /// @param io_service The I/O service the connection will be based on.
  TCPConnection( boost::asio::io_service& io_service);

  /// Function called when a sending operation is completed, successfully or not.
  /// @param error Describes the error that occured, if any.
  void handle_frame_sent(const boost::system::error_code& error);

  /// Function called when a reading operation is completed, successfully or not.
  /// @param error Describes the error that occured, if any.
  /// @param n Number of bytes read.
  void handle_frame_read(const boost::system::error_code& error, size_t n);

  /// Function called when a reading operation times out.
  void close();

private: // data

  /// Timer for reading operations.
  boost::asio::deadline_timer timer;

  /// Network socket.
  boost::asio::ip::tcp::socket m_socket;

  /// Buffer for reading operations
  boost::array<char, 128> m_network_buffer;

}; // TCPConnection

#endif // cf3_sandbox_boost_asio_tcp_connection_hpp
