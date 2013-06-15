// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip> // for std::setw()

#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

#include "common/StringConversion.hpp"

#include "common/XML/SignalFrame.hpp"
#include "common/XML/FileOperations.hpp"

#include "ui/network/ErrorHandler.hpp"
#include "ui/network/TCPConnection.hpp"

using namespace boost;
using namespace boost::asio::ip;
using namespace cf3::common;
using namespace cf3::common::XML;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace network {

//////////////////////////////////////////////////////////////////////////////

TCPConnection::Ptr TCPConnection::create( asio::io_service & ios )
{
  return Ptr( new TCPConnection(ios) );
}

//////////////////////////////////////////////////////////////////////////////

TCPConnection::TCPConnection( asio::io_service & io_service )
  : m_socket(io_service),
    m_incoming_data(nullptr),
    m_incoming_data_size(0)
{

}

/////////////////////////////////////////////////////////////////////////////

TCPConnection::~TCPConnection()
{
  delete[] m_incoming_data;
  disconnect();
}

/////////////////////////////////////////////////////////////////////////////

void TCPConnection::prepare_write_buffers( SignalArgs & args,
                                        std::vector<asio::const_buffer> & buffers )
{
  cf3_assert( args.node.is_valid() );

  // prepare the outgoing data: flush to XML and convert to string
  args.flush_maps();

  XML::to_string( *args.xml_doc.get(), m_outgoing_data );

  // create the header on HEADER_LENGTH characters
  std::ostringstream header_stream;

  header_stream << std::setw(HEADER_LENGTH) << m_outgoing_data.length();

  m_outgoing_header = header_stream.str();

  // write header and data to buffers and then on the socket
  buffers.push_back( asio::buffer(m_outgoing_header) );
  buffers.push_back( asio::buffer(m_outgoing_data) );
}

//////////////////////////////////////////////////////////////////////////////

void TCPConnection::process_header( boost::system::error_code & error )
{
  std::string header_str = std::string( m_incoming_header, HEADER_LENGTH );

  try
  {
    // trim the string to remove the leading spaces (cast fails if spaces are present)
    boost::algorithm::trim( header_str );
    m_incoming_data_size = boost::lexical_cast<cf3::Uint> ( header_str );

    // destroy old buffer and allocate the new one
    delete[] m_incoming_data;
    m_incoming_data = new char[m_incoming_data_size];
  }
  catch ( boost::bad_lexical_cast & blc ) // thrown by from_str()
  {
    notify_error( "Could not cast frame header to unsigned int (header content was ["
           + header_str + "]).");
    error = asio::error::invalid_argument;
  }
  catch ( cf3::common::Exception & cfe )
  {
    notify_error(cfe.what());
    error = asio::error::invalid_argument;
  }
  catch ( std::exception & stde )
  {
    notify_error(stde.what());
    error = asio::error::invalid_argument;
  }
  catch ( ... ) // this function should catch all exception, since it is
  {             // called by some kind of event handler from boost.
    notify_error("An unknown exception has been raised during frame header processsing.");
    error = asio::error::invalid_argument;
  }
}

//////////////////////////////////////////////////////////////////////////////

void TCPConnection::parse_frame_data( SignalFrame & args, boost::system::error_code & error )
{
  try
  {
    std::string frame( m_incoming_data, m_incoming_data_size );

    args = SignalFrame( cf3::common::XML::parse_string( frame ) );
  }

  catch ( cf3::common::Exception & cfe )
  {
   notify_error(cfe.what());
   error = asio::error::invalid_argument;
  }
  catch ( std::exception & stde )
  {
    notify_error(stde.what());
    error = asio::error::invalid_argument;
  }
  catch ( ... ) // this function should catch all exception, since it is called
  {             // by some kind of event handler from boost.
    notify_error("An unknown exception has been raised during frame data processsing.");
    error = asio::error::invalid_argument;
  }

}

//////////////////////////////////////////////////////////////////////////////

void TCPConnection::disconnect()
{
  if( m_socket.is_open() )
  {
    try
    {
      // shutdown both reading and writing async ops => "shutdown_both"
      // node: this doesn't stop running async ops. It only prevents from
      // running new ones.
      m_socket.shutdown( tcp::socket::shutdown_both );
      m_socket.close();
    }
    catch( boost::system::error_code & e )
    {
      notify_error("An error occured while the socket was shutting down: " + e.message());
    }
  }

}

//////////////////////////////////////////////////////////////////////////////

void TCPConnection::set_error_handler( boost::weak_ptr<ErrorHandler> handler)
{
  m_error_handler = handler;
}

//////////////////////////////////////////////////////////////////////////////

void TCPConnection::notify_error( const std::string & message ) const
{
  if( !m_error_handler.expired() )
    m_error_handler.lock()->error(message);
}

//////////////////////////////////////////////////////////////////////////////

} // network
} // ui
} // cf3
