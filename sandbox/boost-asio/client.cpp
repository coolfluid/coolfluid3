// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind/bind.hpp>

#include "common/StringConversion.hpp"

using namespace boost;
using namespace boost::asio::ip;

using namespace cf3;
using namespace cf3::common;

class NetworkManager
{
public:

  NetworkManager()
  {
    m_query = nullptr;
  }

  ~NetworkManager()
  {

  }

  void connect_to_host( const std::string & hostname, Uint port )
  {

  }

private: // boost handlers

  void handler_connected ( const system::error_code & err_code )
  {

  }

  void handler_resolved ( const system::error_code & err_code,
                          const tcp::resolver::iterator it_host )
  {

  }

  void handler_read_data ( const system::error_code & err_code, Uint count )
  {

  }

private: // data

  tcp::resolver::query * m_query;

};

int main( int argc, char * argv[] )
{
  if( argc != 3 )
  {
    std::cerr << "Usage : " << argv[0] << " <host> <port>";
    return 1;
  }

  tcp::resolver::query query( argv[1], cf3::common::from_str<Uint>(argv[2]) );


  return 0;
}
