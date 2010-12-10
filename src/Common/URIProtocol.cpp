// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp> // for map_list_of

#include "Common/URIProtocol.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

URIProtocol::Convert::FwdMap_t URIProtocol::Convert::all_fwd = boost::assign::map_list_of
    ( URIProtocol::INVALID, "Invalid" )
    ( URIProtocol::HTTP,    "http"    )
    ( URIProtocol::HTTPS,   "https"   )
    ( URIProtocol::CPATH,   "cpath"   )
    ( URIProtocol::FILE,    "file"    );

URIProtocol::Convert::BwdMap_t URIProtocol::Convert::all_rev = boost::assign::map_list_of
    ("Invalid",  URIProtocol::INVALID )
    ("http",     URIProtocol::HTTP    )
    ("https",    URIProtocol::HTTPS   )
    ("cpath",    URIProtocol::CPATH   )
    ("file",     URIProtocol::FILE    );

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<< ( std::ostream& os, const URIProtocol::Type& in )
{
  os << URIProtocol::Convert::to_str(in);
  return os;
}

std::istream& operator>> (std::istream& is, URIProtocol::Type& in )
{
  std::string tmp;
  is >> tmp;
  in = URIProtocol::Convert::to_enum(tmp);
  return is;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////
