// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/BasicExceptions.hpp"
#include "common/StringConversion.hpp"
#include "common/TypeInfo.hpp"
#include "common/URI.hpp"
#include "common/UUCount.hpp"

#include "common/BoostAnyConversion.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////

std::string any_to_str( const boost::any & value )
{
  std::string value_type = any_type(value);

  try
  {
    if (value_type == "bool")
      return to_str(boost::any_cast<bool>(value));
    else if (value_type == "unsigned")
      return to_str(boost::any_cast<Uint>(value));
    else if (value_type == "integer")
      return to_str(boost::any_cast<int>(value));
    else if (value_type == "real")
      return to_str(boost::any_cast<Real>(value));
    else if (value_type == "string")
      return boost::any_cast<std::string>(value);
    else if (value_type == "uri")
      return to_str(boost::any_cast<URI>(value));
    else if (value_type == "uucount")
      return to_str(boost::any_cast<UUCount>(value));
    else
      throw ProtocolError(FromHere(),"Value has illegal type: "+value_type);
  }
  catch(boost::bad_any_cast e)
  {
    throw CastingFailed( FromHere(), "Unable to cast from [" + value_type + "] to string");
  }
}

/////////////////////////////////////////////////////////////////////////////

std::string any_type( const boost::any & value )
{
  return class_name_from_typeinfo( value.type() );
}

/////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
TYPE any_to_value( const boost::any & value )
{
  try
  {
    return boost::any_cast< TYPE >( value );
  }
  catch(boost::bad_any_cast& e)
  {
    throw CastingFailed( FromHere(), "Bad boost::any cast from  ["+ any_type(value) +
                         "] to [" + class_name<TYPE>() + "].");
  }
}

/////////////////////////////////////////////////////////////////////////////

Common_TEMPLATE template bool any_to_value<bool>( const boost::any& );
Common_TEMPLATE template int any_to_value<int>( const boost::any& );
Common_TEMPLATE template Uint any_to_value<Uint>( const boost::any& );
Common_TEMPLATE template Real any_to_value<Real>( const boost::any& );
Common_TEMPLATE template std::string any_to_value<std::string>( const boost::any& );
Common_TEMPLATE template URI any_to_value<URI>( const boost::any& );
Common_TEMPLATE template UUCount any_to_value<UUCount>( const boost::any& );


Common_TEMPLATE template std::vector<Real> any_to_value< std::vector<Real> >( const boost::any& );

/////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
