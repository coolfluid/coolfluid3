// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"
#include "Common/PropertyList.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  Property::Ptr PropertyList::getProperty( const std::string& pname)
  {
    PropertyStorage_t::iterator itr = m_properties.find(pname);
    if ( itr != m_properties.end() )
      return itr->second;
    else
      throw ValueNotFound(FromHere(), "Property with name [" + pname + "] not found" );
  }

  void PropertyList::erase( const std::string& pname)
  {
    PropertyStorage_t::iterator itr = m_properties.find(pname);
    if ( itr != m_properties.end() )
      m_properties.erase(itr);
    else
      throw ValueNotFound(FromHere(), "Property with name [" + pname + "] not found" );
  }

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

