// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/Property.hpp"
#include "Common/URI.hpp"

#include "Common/Option.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  Property::Property (boost::any value)
    : m_value(value),
      m_is_option(false)
  {
  }

  Property::~Property()
  {
  }


  void Property::change_value ( const boost::any& value )
  {
    m_value = value; // update the value
  }

  Option & Property::as_option()
  {
    cf_assert(m_is_option);
    return *(static_cast< Option* >(this));
  }

  const Option & Property::as_option() const
  {
    cf_assert(m_is_option);
    return *(static_cast<const Option* >(this));
  }

  const char * Property::tag() const
  {
    if(is_option())
      return as_option().tag();

    return CF::class_name_from_typeinfo(m_value.type()).c_str();
  }


  Property & Property::operator = (const boost::any & value)
  {
    change_value(value);
    return *this;
  }

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
