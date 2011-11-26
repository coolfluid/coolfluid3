// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>
#include <boost/asio.hpp>

using namespace boost;
using namespace boost::asio::ip;

int main( int argc, char * argv[] )
{
  if( argc != 2 )
  {
    std::cerr << "Usage : " << argv[0] << " <port>";
    return 1;
  }


  return 0;
}
