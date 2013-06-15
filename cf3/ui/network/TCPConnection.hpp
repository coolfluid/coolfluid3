// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_network_connection_hpp
#define cf3_ui_network_connection_hpp

#include <boost/asio/ip/tcp.hpp>           // TCP related classes
#include <boost/asio/placeholders.hpp>     // for placholder::error_code
#include <boost/asio/read.hpp>             // for async_read()
#include <boost/asio/write.hpp>            // for async_write()
#include <boost/bind/bind.hpp>             // for boost::bind()
#include <boost/enable_shared_from_this.hpp>
#include <boost/tuple/tuple.hpp>           // for managing multiple callback fcts
#include <boost/variant/get.hpp>           // for calling callback functions

#include "ui/network/LibNetwork.hpp"

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common
{
namespace XML { class SignalFrame; }
}

namespace ui {
namespace network {

//////////////////////////////////////////////////////////////////////////////

class ErrorHandler;

/// @brief Manages a TCP/IP connection between two entities where one is a
/// server and the other one is a client.

/// This class is intented to be used in an asynchronous network architecture.
/// Therefore, a @c Connection object cannot live outside of a shared pointer
/// because a such object has to maintain asynchronous operations. One cannot
/// predict when all of those operations will be completed; using a shared
/// pointer garantees the connection stays alive until all operations are done.
/// Use static function @c #create() to create a @c Connection object. @n@n

/// A TCP/IP connection is based on an I/O service that handles the asynchronous
/// operations and calls an appropriate function when one of those is
/// completed. @n@n

/// Frames handled by this class have two main parts:
/// @li A size-fixed header (8 bytes): contains the size in bytes of the frame
/// data.
/// @li Frame data: actual data that is sent, in XML format.@n@n
///
/// The header is completely tansparent to the calling code and is used as a
/// safeguard to check that all data has arrived and allocate the correct buffer
/// for the reading process. @n@n

/// This class can be used in both client and server applications. However, an
/// additional step is needed on the server-side: open a network connection and
/// start accepting new clients connections. @n@n

/// Most interesting method provided by this class are obviously @c #read() and
/// @c #send(). Both take the same parameters:
/// @li a non-const reference to a
/// @link cf3::common::XML::SignalFrame @c SignalFrame @endlink object.
/// @c #read() uses it as a buffer to which read data will be written. For
/// @c #send(), it contains the data to send (the reference is non-const because
/// @link cf3::common::XML::SignalFrame::flush_maps() @c SignalFrame::flush_maps() @endlink
/// is called before converting the XML to string).
/// @li the callback function which will be called when asynchronous
/// operations finishes (on succes or failure). @n@n
///
/// The callback function can be created using @c Boost.Bind. Developers are
/// free on the choice of the paramaters to give to this callback function, but
/// have to handle those parameters by themselves. However, both @c read and
/// @c send methods require one placeholder of type @c boost::system::error_code
/// to store the error code. The placeholder value is given by the network layer.
/// Examples:
/// @li call @c read with a callback function that only takes the error code:
/// @code
/// connection->read( m_read_buffer, // SignalFrame object. Has to live until the end of the read!
///                   boost::bind( &MyClass::callback_read, // pointer to callback function
///                                this,                    // object on which to call the function
///                                boost::asio::placeholders::error ) // placeholder for error code
///                 );
///
/// // definition of the callback function
/// void MyClass::callback_read( const boost::system::error_code error )
/// {
///   if ( error )
///   {
///     // operation failed: m_read_buffer was not modified
///   {
///   else
///   {
///     // operation succeeded: m_read_buffer contains the parsed XML data
///   {
/// }
/// @endcode
/// @li call @c read with a callback function that takes the error code and
/// a @c Connection shared pointer (i.e. a multi-client server application that
/// wants to know which client is speaking):
/// @code
/// connection->read( m_read_buffer, // SignalFrame object. Has to live until the end of the read!
///                   boost::bind( &MyClass::callback_read, // pointer to callback function
///                                this,                    // object on which to call the function
///                                connection,              // 1st parameter for the callback function
///                                boost::asio::placeholders::error ) // placeholder for error code
///                 );
///
/// // definition of the callback function
/// void MyClass::callback_read( Connection::Ptr conn, const boost::system::error_code error )
/// {
///   /// "conn" is a copy of the shared pointer we gave to @c boost::bind()
///   if ( error )
///   {
///     // operation failed: m_read_buffer was not modified
///   {
///   else
///   {
///     // operation succeeded: m_read_buffer contains the parse XML data
///   {
/// }
/// @endcode
/// Similar codes can be applied to @c send function.
///
/// The internal socket can be retrieve by calling @c socket(). Developer
/// can set an error handler by calling @c set_error_handler().
///
/// @n@warning Due to template code, a lot of Boost headers are included by this
/// file, which might increase the compilation time. Please consider only
/// including this file in a limited number of CPP files (typically, one per
/// application is sufficient).

/// @sa ErrorHandler

/// @author Quentin Gasper



class Network_API TCPConnection
    : public boost::enable_shared_from_this<TCPConnection>
{

public: // typedefs

  typedef boost::shared_ptr<TCPConnection> Ptr;
  typedef boost::shared_ptr<TCPConnection const> ConstPtr;

public:

  /// @brief Creates a @c TCPConnection instance.
  /// @param io_service The I/O service the new connection will be based on.
  /// @return Returns the created instance as a boost share pointer.
  static Ptr create( boost::asio::io_service & ios );

  /// @brief Destructor.
  /// Cleanly closes the socket.
  ~TCPConnection();

  /// @brief Gives a reference to the internal socket.
  /// @return Returns a reference to the internal socket.
  boost::asio::ip::tcp::socket & socket()
  {
    return m_socket;
  }

  /// @brief Gives a constant referemce to the internal socket.
  /// @return Returns a constant reference to the internal socket.
  const boost::asio::ip::tcp::socket & socket() const
  {
    return m_socket;
  }

  /// @brief Sends a message to the remote entity.
  /// The message is sent asynchronously and the function returns directly,
  /// before the data is actually send. Once the sending is finished,
  /// the @c callback_function is called with error code
  /// @param data The XML data to send. Must be valid.
  /// @param callback_function The callback function to call when
  /// asynchronous operation is finished. See this class description for further
  /// information about callback functions.
  /// @note
  /// @li this methods calls @c flush_maps() on @c args
  /// @li the XML data is converted to string and stored in an internal buffer.
  /// The calling can the freely reuse the object reference by @c args derectly
  /// after this method returns.
  template<typename HANDLER>
  void send( cf3::common::XML::SignalFrame & args, HANDLER callback_function )
  {
    std::vector<boost::asio::const_buffer> buffers;

    prepare_write_buffers( args, buffers );

    boost::asio::async_write( m_socket, buffers, callback_function );
  }

  /// @brief Initiates an asynchronous reading from the remote entity.
  /// The function returns directly.
  /// @param args The buffer used to write recieved data.
  /// @param callback_function The callback function to call when
  /// asynchronous operation is finished. See this class description for further
  /// information about callback functions.
  /// @warning Calling code has to garantee that the object referenced by @c args
  /// remains alive until the end of the asynchronous operation.
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
                                   boost::asio::placeholders::error,
                                   boost::make_tuple( callback_function )
                                 )
                    );
  }

  /// Disconnects the socket from the remote entity.
  void disconnect();

  /// Sets an error handler.
  /// @param handler Error handler to set. Can be expired.
  void set_error_handler ( boost::weak_ptr<ErrorHandler> handler );

private: // functions

  /// @brief Function called when a frame header has been read, successfully or not.
  /// This function allocates the frame data buffer depending on the header
  /// received. If reading has failed or header is not valid, the
  /// callback function is called with appropriate error code.
  /// @param error Describes the error that occured, if any.
  /// @param conn The connection
  /// @param error Error code, if any.
  /// @param functions Callback function
  template< typename HANDLER >
  void callback_header_read( cf3::common::XML::SignalFrame & args,
                             const boost::system::error_code & error,
                             boost::tuple<HANDLER> functions )
  {
    using namespace boost;

    if ( !error )
    {
      boost::system::error_code err(error);

      process_header(err);

      if( err )
        boost::get<0>( functions )( err );
      else
      {

      // initiate an async read to get the frame data
      asio::async_read( m_socket,
                        asio::buffer( m_incoming_data, m_incoming_data_size ),
                        boost::bind( &TCPConnection::callback_data_read<HANDLER>,
                                     shared_from_this(),
                                     boost::ref( args ),
                                     boost::asio::placeholders::error,
                                     functions
                                   )
                      );
      }
    }
    else
    {
      boost::get<0>( functions )( error );
    }
  }

  /// @brief Function called when the frame data has been read.
  /// @param error Describes the error that occured, if any.
  /// @param conn The connection
  /// @param error Error code, if any.
  /// @param functions Callback function
  template< typename HANDLER >
  void callback_data_read( common::XML::SignalFrame & args,
                           const boost::system::error_code & error,
                           boost::tuple<HANDLER> functions )
  {
    boost::system::error_code err(error);

    if ( !error )
      parse_frame_data( args, err );

    boost::get<0>( functions )( err );
  }

private: // functions

  /// @brief Constructor.
  /// @param io_service The I/O service the connection will be based on.
  TCPConnection( boost::asio::io_service& io_service );

  /// @brief Builds the data to be sent on the network.
  /// @param args XML data. @c flush_maps() is called before converting to string.
  /// @param buffer Data buffer. First item is the header and second item is
  /// the frame data. Vector is cleared before first use.
  void prepare_write_buffers( common::XML::SignalFrame & args,
                              std::vector<boost::asio::const_buffer> & buffers );

  /// @brief Processes a frame header.
  /// Tries to cast the header to an @c unsigned @c int. On success, allocates
  /// the data buffer to this size.
  void process_header ( boost::system::error_code & error );

  /// @brief Parses frame data from string to XML.
  /// @param args Object where the parsed XML will be written.
  void parse_frame_data ( common::XML::SignalFrame & args,
                          boost::system::error_code & error);

  /// @brief Notifies an error if an error handler has been set.
  /// @param message Error message.
  void notify_error( const std::string & message ) const;

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
  /// @warning This buffer does NOT end by '\0'. Its size is given by
  /// @c m_incoming_data_size.
  char * m_incoming_data;

  /// Weak pointer to the error handler.
  boost::weak_ptr<ErrorHandler> m_error_handler;

}; // TCPConnection

//////////////////////////////////////////////////////////////////////////////

} // network
} // ui
} // cf3


//////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_network_connection_hpp
