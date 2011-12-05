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

#include <boost/algorithm/string/trim.hpp>

#include "common/Assertions.hpp"
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

  std::cout << "[" << m_outgoing_header << "]" << std::endl;
  std::cout << m_outgoing_data.size() << " => " << m_outgoing_data << std::endl;
}

//////////////////////////////////////////////////////////////////////////////


void TCPConnection::handle_frame_sent( const system::error_code & error )
{
  if (!error)
  {
//    read();
  }
  else
  {
    CFerror << "An error has occured during frame sending: " << error.message() << CFendl;
  }
}

//////////////////////////////////////////////////////////////////////////////
