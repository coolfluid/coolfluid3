// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sandbox_boost_asio_tcp_connection_hpp
#define cf3_sandbox_boost_asio_tcp_connection_hpp

#include <boost/asio/ip/tcp.hpp>           // TCP related classes
#include <boost/asio/placeholders.hpp>     // for placholder::error_code
#include <boost/asio/read.hpp>             // for async_read()
#include <boost/asio/write.hpp>            // for async_write()
#include <boost/bind/bind.hpp>             // for boost::bind()
#include <boost/enable_shared_from_this.hpp>
#include <boost/tuple/tuple.hpp>           // for managing multiple callback fcts
#include <boost/variant/get.hpp>           // for calling callback functions

namespace cf3 { namespace common { namespace XML { class SignalFrame; } } }

/// Manages a TCP connection between to entities where one is a
/// server and the other one is a client.@n

/// This class is intented to be used in an asynchronous network architecture.
/// Therefore, a @c TCPConnection object cannot live outside of a shared
/// pointer because a such object has to maintain asynchronous operations.
/// One cannot predict when all of those operations will be completed, using a
/// shared pointer garantees the connection stays alive until all operations
/// are done. @c

/// Frames handled by this class have two main parts:
/// @li A size-fixed header (8 bytes): contains the size in bytes of the frame
/// data.
/// @li Frame data: actual data that is sent.
/// The header is complitely tansparent to the calling code and is use as a
/// guard to check that all data has arrived and allocate the correct buffer
/// for the reading process.

/// A TCP connection is based on an I/O service that handles the asynchronous
/// operations and calls an appropriate function when of those is completed. @n

/// This class can be used in both client and server applications. However, an
/// additional step is need on the server-side: open a network connection and
/// start accepting new clients connections. @n

/// @note Due to template code, a lot of Boost headers are included by this
/// file, which might increase the compilation time. Please consider only
/// including this file in a limited number of CPP files (typically, one per
/// application is sufficient).

/// @author Quentin Gasper

class TCPConnection
      : public boost::enable_shared_from_this<TCPConnection>
{

public: // typedefs

  typedef boost::shared_ptr<TCPConnection> Ptr;
  typedef boost::shared_ptr<TCPConnection const> ConstPtr;

public:

  /// Creates a @c TCPConnection instance.
  /// @param io_service The I/O service the new connection will be based on.
  /// @return Returns the created instance as a boost share pointer.
  static Ptr create( boost::asio::io_service & ios );

  /// Destructor.
  /// Cleanly closes the socket.
  ~TCPConnection();

  /// Gives a referemce to the internal socket.
  /// @return Returns a reference to the internal socket.
  boost::asio::ip::tcp::socket & socket()
  {
    return m_socket;
  }

  /// Gives a constant referemce to the internal socket.
  /// @return Returns a constant reference to the internal socket.
  const boost::asio::ip::tcp::socket & socket() const
  {
    return m_socket;
  }

  /// Sends a message to the remote entity.
  /// The message is sent asynchronously and the function returns directly,
  /// before the data is actually send.
  /// @param data The XML data to send. Must be valid.
  /// @param callback_function The function to call when sending is finished
  /// (success or failure).
  template<typename HANDLER>
  void send( cf3::common::XML::SignalFrame & args, HANDLER callback_function )
  {
    std::vector<boost::asio::const_buffer> buffers;

    prepare_write_buffers( args, buffers );

    boost::asio::async_write( m_socket, buffers, callback_function );
  }

  /// Initiates an asynchronous reading from the remote entity.
  /// The function returns directly.
  template< typename HANDLER >
  void read( cf3::common::XML::SignalFrame & args,  HANDLER callback_function )
  {

    using namespace boost;

    // initiate the async read for the header and bind the callback function
    asio::async_read( m_socket,
                      asio::buffer( m_incoming_header ),
                      boost::bind( &TCPConnection::callback_header_read<HANDLER>,
                                   shared_from_this(),
                                   boost::ref( args ),
                                   shared_from_this(),
                                   boost::asio::placeholders::error,
                                   boost::make_tuple( callback_function )
                                 )
                    );
  }

private: // functions

  /// Function called when a reading operation is completed, successfully or not.
  /// @param error Describes the error that occured, if any.
  template< typename HANDLER >
  void callback_header_read( cf3::common::XML::SignalFrame & args,
                             TCPConnection::Ptr conn,
                             const boost::system::error_code & error,
                             boost::tuple<HANDLER> functions )
  {
    using namespace boost;

    if ( !error )
    {
      process_header();

      // initiate an async read to get the frame data
      asio::async_read( m_socket,
                        asio::buffer( m_incoming_data, m_incoming_data_size ),
                        boost::bind( &TCPConnection::callback_data_read<HANDLER>,
                                     shared_from_this(),
                                     boost::ref( args ),
                                     shared_from_this(),
                                     boost::asio::placeholders::error,
                                     functions
                                     )
                        );
    }
    else
    {
      boost::get<0>( functions )( error );
    }
  }

  /// Function called when the frame data has been read
  template< typename HANDLER >
  void callback_data_read( cf3::common::XML::SignalFrame & args,
                           TCPConnection::Ptr conn,
                           const boost::system::error_code & error,
                           boost::tuple<HANDLER> functions )
  {
    if ( !error )
    {
      parse_frame_data( args );
      boost::get<0>( functions )( error );
    }
    else
    {
      boost::get<0> ( functions )( error );
    }
  }

private: // functions

  /// Constructor.
  /// @param io_service The I/O service the connection will be based on.
  TCPConnection( boost::asio::io_service& io_service );

  void prepare_write_buffers( cf3::common::XML::SignalFrame & args,
                              std::vector<boost::asio::const_buffer> & buffers );

  void process_header ();

  void parse_frame_data ( cf3::common::XML::SignalFrame & args );

private: // data

  /// Network socket.
  boost::asio::ip::tcp::socket m_socket;

  /// Buffer for outgoing data
  std::string m_outgoing_data;

  /// Buffer for outgoing header
  std::string m_outgoing_header;

  /// Nameless enum for header length
  enum { HEADER_LENGTH = 8 };

  /// Buffer the receiving header.
  char m_incoming_header[HEADER_LENGTH];

  /// Size of the receiving buffer.
  unsigned int m_incoming_data_size;

  /// Receiving buffer.
  /// @warning This buffer does NOT finish by '\0'. Its size is given by
  /// @c m_incoming_data_size.
  char * m_incoming_data;

}; // TCPConnection

#endif // cf3_sandbox_boost_asio_tcp_connection_hpp
