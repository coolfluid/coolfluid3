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

#include "common/SignalHandler.hpp" // for cf3::common::SignalArgs typedef

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
class TCPConnection
    : public cf3::common::SignalHandler,
      public boost::enable_shared_from_this<TCPConnection>
{
public:

  /// Constructor.
  /// @param io_service The I/O service the connection will be based on.
  TCPConnection( boost::asio::io_service& io_service );

  /// Gives a referemce to the internal socket.
  /// @return Returns a reference to the internal socket.
  boost::asio::ip::tcp::socket & socket() { return m_socket; }

  /// Gives a constant referemce to the internal socket.
  /// @return Returns a constant reference to the internal socket.
  const boost::asio::ip::tcp::socket & socket() const { return m_socket; }

  /// Sends a message to the remote entity.
  /// The message is sent asynchronously and the function returns directly,
  /// before the data is actually send.
  /// @param data The XML data to send. Must be valid.
  void send( cf3::common::SignalArgs & data );

  /// Initiates an asynchronous reading from the remote entity.
  /// The function returns directly.
  void read();

private: // functions

  /// Function called when a sending operation is completed, successfully or not.
  /// @param error Describes the error that occured, if any.
  void handle_frame_sent( const boost::system::error_code& error );

  /// Function called when a reading operation is completed, successfully or not.
  /// @param error Describes the error that occured, if any.
  void handle_frame_header_read( const boost::system::error_code & error );

  /// Function called when the frame data has been read
  void handle_frame_data_read( const boost::system::error_code & error, size_t count );

private: // data

  /// Network socket.
  boost::asio::ip::tcp::socket m_socket;

  std::string m_outgoing_data;

  std::string m_outgoing_header;

  enum { HEADER_LENGTH = 8 };

  char m_incoming_header[HEADER_LENGTH];

  std::vector<char> m_incoming_data;

}; // TCPConnection

#endif // cf3_sandbox_boost_asio_tcp_connection_hpp
