// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp> // for map_list_of

#include "GUI/Network/ComponentType.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF::GUI::Network;

////////////////////////////////////////////////////////////////////////////////

ComponentType::Convert::FwdMap_t ComponentType::Convert::all_fwd = boost::assign::map_list_of
( ComponentType::INVALID, "INVALID" )
( ComponentType::ROOT,    "Root")
( ComponentType::GROUP,   "Group")
( ComponentType::LINK,    "Link");

ComponentType::Convert::BwdMap_t ComponentType::Convert::all_rev = boost::assign::map_list_of
( "INVALID", ComponentType::INVALID )
( "Root",    ComponentType::ROOT)
( "Group",   ComponentType::GROUP)
( "Link",    ComponentType::LINK);

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<< ( std::ostream& os, const ComponentType::Type& in )
{
  os << ComponentType::Convert::to_str(in);
  return os;
}

std::istream& operator>> (std::istream& is, ComponentType::Type& in )
{
  std::string tmp;
  is >> tmp;
  in = ComponentType::Convert::to_enum(tmp);
  return is;
}


////////////////////////////////////////////////////////////////////////////////
