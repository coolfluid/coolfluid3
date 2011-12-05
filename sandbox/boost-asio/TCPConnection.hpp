// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sandbox_boost_asio_tcp_connection_hpp
#define cf3_sandbox_boost_asio_tcp_connection_hpp

#include <boost/algorithm/string/trim.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <boost/variant/get.hpp>

#include "common/Log.hpp" // to be removed!!!

#include "common/XML/FileOperations.hpp"
#include "common/SignalHandler.hpp"

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

/// @todo either allow a calling code to @c boost::bind a callback function for
/// completion of sending and reading operations or use the CF EnventHandler to
/// to propagate received data.

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
  boost::asio::ip::tcp::socket & socket() { return m_socket; }

  /// Gives a constant referemce to the internal socket.
  /// @return Returns a constant reference to the internal socket.
  const boost::asio::ip::tcp::socket & socket() const { return m_socket; }

  /// Sends a message to the remote entity.
  /// The message is sent asynchronously and the function returns directly,
  /// before the data is actually send.
  /// @param data The XML data to send. Must be valid.
  /// @param callback_function The function to call when sending is finished
  /// (success or failure).
  template<typename HANDLER>
  void send( cf3::common::SignalArgs & args, HANDLER callback_function )
  {
    std::vector<boost::asio::const_buffer> buffers;

    prepare_write_buffers( args, buffers );

    boost::asio::async_write( m_socket, buffers, callback_function );
  }

  /// Initiates an asynchronous reading from the remote entity.
  /// The function returns directly.
  template< typename HANDLER >
  void read( cf3::common::SignalArgs & args,  HANDLER callback_function )
  {
    // initiate the async read for the header and bind the callback function
    boost::asio::async_read( m_socket,
                             boost::asio::buffer(m_incoming_header),
                             boost::bind( &TCPConnection::handle_frame_header_read<HANDLER>,
                                          shared_from_this(),
                                          boost::ref(args),
                                          boost::asio::placeholders::error,
                                          boost::make_tuple(callback_function)
                                         )
                            );
  }

private: // functions

  /// Function called when a sending operation is completed, successfully or not.
  /// @param error Describes the error that occured, if any.
  void handle_frame_sent( const boost::system::error_code& error );

  /// Function called when a reading operation is completed, successfully or not.
  /// @param error Describes the error that occured, if any.
  template< typename HANDLER >
  void handle_frame_header_read( cf3::common::SignalArgs & args,
                                 const boost::system::error_code & error,
                                 boost::tuple<HANDLER> functions )
  {
    if( !error )
    {
      try
      {
        std::string header_str = std::string( m_incoming_header, HEADER_LENGTH );

        // trim the string to remove the leading spaces (cast fails if spaces are present)
        boost::algorithm::trim(header_str);
        cf3::Uint data_size = boost::lexical_cast<cf3::Uint>( header_str ); //from_str<cf3::Uint>( header_str );

        // resize the data vector
        m_incoming_data.resize( (size_t) data_size );

        // initiate an async read to get the frame data
        boost::asio::async_read( m_socket,
                                 boost::asio::buffer(m_incoming_data, data_size),
                                 boost::bind( &TCPConnection::handle_frame_data_read<HANDLER>,
                                              shared_from_this(),
                                              boost::ref(args),
                                              boost::asio::placeholders::error,
                                              functions
                                             )
                                );
      }
      catch ( boost::bad_lexical_cast & blc ) // thrown by from_str()
      {
        CFerror << "Could not cast frame header to unsigned int "
                << "(header content was [" /*<< m_incoming_data*/ << "])." << CFendl;
      }
      catch ( cf3::common::Exception & cfe )
      {
        CFerror << cfe.what() << CFendl;
      }
      catch ( std::exception & stde )
      {
        CFerror << stde.what() << CFendl;
      }
      catch (...) // this function should catch all exception, since it is called
      {           // by some kind of event handler from boost.
        CFerror << "An unknown exception has been raised during frame header processsing." << CFendl;
      }
    }
    else if ( m_socket.is_open() )
    {
      boost::get<0>(functions)(error);
    }
  }

  /// Function called when the frame data has been read
  template< typename HANDLER >
  void handle_frame_data_read( cf3::common::SignalArgs & args,
                               const boost::system::error_code & error,
                               boost::tuple<HANDLER> functions )
  {
    if( !error )
    {
      try
      {
        std::string frame( &m_incoming_data[0], m_incoming_data.size() );

        CFinfo << frame << CFendl;

        args = cf3::common::XML::parse_string(frame);

        boost::get<0>(functions)(error);

      }
      catch ( cf3::common::Exception & cfe )
      {
        CFerror << cfe.what() << CFendl;
      }
      catch ( std::exception & stde )
      {
        CFerror << stde.what() << CFendl;
      }
      catch (...) // this function should catch all exception, since it is called
      {           // by some kind of event handler from boost.
        CFerror << "An unknown exception has been raised during frame data processsing." << CFendl;
      }
    }
    else
    {
      boost::get<0>(functions)(error);
    }
  }

private: // functions

  /// Constructor.
  /// @param io_service The I/O service the connection will be based on.
  TCPConnection( boost::asio::io_service& io_service );

  void prepare_write_buffers( cf3::common::SignalArgs & args,
                              std::vector<boost::asio::const_buffer> & buffers );

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
