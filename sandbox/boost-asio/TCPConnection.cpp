// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include <boost/bind/bind.hpp>

#include <boost/asio/placeholders.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <boost/lexical_cast.hpp>

#include <boost/algorithm/string/trim.hpp>

#include "common/Assertions.hpp"
#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/StringConversion.hpp"

#include "common/XML/SignalFrame.hpp"
#include "common/XML/FileOperations.hpp"

#include "boost-asio/TCPConnection.hpp"

using namespace boost;
using namespace boost::asio::ip;

using namespace cf3::common;
using namespace cf3::common::XML;

//////////////////////////////////////////////////////////////////////////////

TCPConnection::Ptr TCPConnection::create( asio::io_service & ios )
{
  return Ptr( new TCPConnection(ios) );
}

//////////////////////////////////////////////////////////////////////////////

TCPConnection::TCPConnection( asio::io_service & io_service )
  : m_socket(io_service)
{
  regist_signal("new_frame")
      ->description("Signal called when a frame has been received.")
      ->hidden(true);
}

/////////////////////////////////////////////////////////////////////////////

TCPConnection::~TCPConnection()
{
  try
  {
    // shutdown both reading and writing async ops => "shutdown_both"
    m_socket.shutdown( tcp::socket::shutdown_both );
    m_socket.close();
  }
  catch( system::error_code & e )
  {
    std::cerr << "An error occured while the socket was shutting down: "
              << e.message() << std::endl;
  }
}

/////////////////////////////////////////////////////////////////////////////

void TCPConnection::send( cf3::common::SignalArgs & message )
{
  cf3_assert( message.node.is_valid() );
  std::vector< asio::const_buffer > write_buffers;

  // prepare the outgoing data: flush to XML and convert to string
  message.flush_maps();
  XML::to_string( *message.xml_doc.get(), m_outgoing_data );

  // create the header on HEADER_LENGTH characters
  std::ostringstream header_stream;
  header_stream << std::setw(HEADER_LENGTH) << m_outgoing_data.length();

  m_outgoing_header = header_stream.str();

  // write header and data to buffers and then on the socket
  write_buffers.push_back( asio::buffer(m_outgoing_header) );
  write_buffers.push_back( asio::buffer(m_outgoing_data) );

  std::cout << "[" << m_outgoing_header << "]" << std::endl;
  std::cout << m_outgoing_data.size() << " => " << m_outgoing_data << std::endl;

  asio::async_write( m_socket,
                     write_buffers,
                     boost::bind( &TCPConnection::handle_frame_sent,
                                  shared_from_this(),
                                  asio::placeholders::error )
                    );
}

//////////////////////////////////////////////////////////////////////////////

void TCPConnection::read()
{
  // initiate the async read for the header and bind the callback function
  asio::async_read( m_socket,
                    asio::buffer(m_incoming_header),
                    boost::bind( &TCPConnection::handle_frame_header_read,
                                 shared_from_this(),
                                 asio::placeholders::error ) // callback 1st argument
                   );
}


//////////////////////////////////////////////////////////////////////////////

void TCPConnection::handle_frame_sent( const system::error_code & error )
{
  if (!error)
  {
    read();
  }
  else
  {
    CFerror << "An error has occured during frame sending: " << error.message() << CFendl;
  }
}

//////////////////////////////////////////////////////////////////////////////

void TCPConnection::handle_frame_header_read( const system::error_code & error )
{

  if( !error )
  {
    try
    {
      std::string header_str = std::string( m_incoming_header, HEADER_LENGTH );

      // trim the string to remove the leading spaces (cast fails if spaces are present)
      algorithm::trim(header_str);
      cf3::Uint data_size = from_str<cf3::Uint>( header_str );

      // resize the data vector
      m_incoming_data.resize( (size_t) data_size );

      // initiate an async read to get the frame data
      asio::async_read( m_socket,
                        asio::buffer(m_incoming_data, data_size),
                        boost::bind( &TCPConnection::handle_frame_data_read,
                                     shared_from_this(),
                                     asio::placeholders::error, // callback 1st argument
                                     asio::placeholders::bytes_transferred )
                       );
    }
    catch ( boost::bad_lexical_cast & blc ) // thrown by from_str()
    {
      CFerror << "Could not cast frame header to unsigned int "
              << "(header content was [" /*<< m_incoming_data*/ << "])." << CFendl;
    }
    catch ( Exception & cfe )
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
    CFerror << "An error occured during frame header reading : "
            << error.message() << CFendl;
  }
}

//////////////////////////////////////////////////////////////////////////////

void TCPConnection::handle_frame_data_read( const system::error_code & error,
                                             size_t count )
{
  if( !error )
  {
    try
    {
      std::string frame (&m_incoming_data[0], count);

      SignalFrame signal_frame = XML::parse_string(frame);

      call_signal( "new_frame", signal_frame );
    }
    catch ( Exception & cfe )
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
  else if ( m_socket.is_open() )
  {
    CFerror << "An error occured during frame data reading: "
            << error.message() << CFendl;
  }

  read();
}

//////////////////////////////////////////////////////////////////////////////
