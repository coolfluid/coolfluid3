// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "Common/TypeInfo.hpp"
#include "Common/BoostFilesystem.hpp"

#include "Common/Property.hpp"
#include "Common/URI.hpp"

#include "Common/Option.hpp"
#include "Common/StringConversion.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

Property::Property (boost::any value)
  : m_value(value),
    m_is_option(false)
{
}

////////////////////////////////////////////////////////////////////////////////

Property::~Property()
{
}

////////////////////////////////////////////////////////////////////////////////

std::string Property::value_str () const
{
  std::string value_type = type();
  if (value_type == "bool")
    return to_str(value<bool>());
  else if (value_type == "unsigned")
    return to_str(value<Uint>());
  else if (value_type == "integer")
    return to_str(value<int>());
  else if (value_type == "real")
    return to_str(value<Real>());
  else if (value_type == "string")
    return value<std::string>();
  else if (value_type == "uri")
    return to_str(value<URI>());
  else
    throw ProtocolError(FromHere(),"Property has illegal value type: "+value_type);
}

////////////////////////////////////////////////////////////////////////////////

void Property::change_value ( const boost::any& value )
{
  m_value = value; // update the value
}

////////////////////////////////////////////////////////////////////////////////

Option & Property::as_option()
{
  cf_assert(m_is_option);
  throw NotImplemented(FromHere(), "Property::as_option() -> should be removed");
//  return *(static_cast< Option* >(this));
}

////////////////////////////////////////////////////////////////////////////////

const Option & Property::as_option() const
{
  cf_assert(m_is_option);
  throw NotImplemented(FromHere(), "Property::as_option() -> should be removed");
//  return *(static_cast<const Option* >(this));
}

////////////////////////////////////////////////////////////////////////////////

const char * Property::tag() const
{
  if(is_option())
    return as_option().tag();

  return class_name_from_typeinfo(m_value.type()).c_str();
}

////////////////////////////////////////////////////////////////////////////////

Property & Property::operator = (const boost::any & value)
{
  change_value(value);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

std::string Property::type() const
{
  return class_name_from_typeinfo(m_value.type());
}

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
