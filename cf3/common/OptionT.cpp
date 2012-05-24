// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "rapidxml/rapidxml.hpp"

#include "common/BoostFilesystem.hpp"
#include "common/BasicExceptions.hpp"
#include "common/OptionT.hpp"
#include "common/StringConversion.hpp"
#include "common/URI.hpp"
#include "common/UUCount.hpp"

#include "common/XML/Protocol.hpp"


using namespace cf3::common::XML;

namespace cf3 {
namespace common {

namespace detail
{
  /// Helper function to set the value
  template<typename TYPE>
  void change_value(boost::any& to_set, const boost::any& new_value)
  {
    to_set = new_value; // update the value
  }

  template<>
  void change_value<Uint>(boost::any& to_set, const boost::any& new_value)
  {
    if(new_value.type() == to_set.type())
    {
      to_set = new_value;
    }
    else
    {
      try
      {
        int int_val = boost::any_cast<int>(new_value);
        if(int_val < 0)
          throw BadValue(FromHere(), "Tried to store a negative value in an unsigned int option");
        to_set = static_cast<Uint>(int_val);
      }
      catch(boost::bad_any_cast& e)
      {
        throw CastingFailed(FromHere(), std::string("Failed to cast object of type ") + new_value.type().name() + " to type Uint");
      }
    }
  }

  template<>
  void change_value<int>(boost::any& to_set, const boost::any& new_value)
  {
    if(new_value.type() == to_set.type())
    {
      to_set = new_value;
    }
    else
    {
      try
      {
        Uint uval = boost::any_cast<Uint>(new_value);
        to_set = static_cast<int>(uval);
      }
      catch(boost::bad_any_cast& e)
      {
        throw CastingFailed(FromHere(), std::string("Failed to cast object of type ") + new_value.type().name() + " to type int");
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////////////

template < typename TYPE>
OptionT<TYPE>::OptionT ( const std::string& name, value_type def) :
    Option(name, def)
{
}

template < typename TYPE >
void OptionT<TYPE>::copy_to_linked_params(std::vector< boost::any >& linked_params )
{
  TYPE val = this->template value<TYPE>();
  BOOST_FOREACH ( boost::any& v, linked_params )
  {
    TYPE* cv = boost::any_cast<TYPE*>(v);
    *cv = val;
  }
}

template < typename TYPE >
boost::any OptionT<TYPE>::extract_configured_value(XmlNode& node)
{
  const char * type_str = Protocol::Tags::type<TYPE>();
  XmlNode type_node(node.content->first_node(type_str));

  if( type_node.is_valid() )
    return from_str<TYPE>( type_node.content->value() );
  else
    throw XmlError(FromHere(), "Could not find a value of this type ["+std::string(type_str)+"].");
}


template < typename TYPE >
const char* OptionT<TYPE>::tag () const
{
  return Protocol::Tags::type<TYPE>();
}

template<typename TYPE>
std::string OptionT<TYPE>::value_str () const
{
  return to_str( value<TYPE>() );
}

template<typename TYPE>
void OptionT<TYPE>::change_value_impl(const boost::any& value)
{
  detail::change_value<TYPE>(m_value, value);
}


////////////////////////////////////////////////////////////////////////////////

/// explicit instantiation to avoid missing symbols in certain compilers
Common_TEMPLATE template class OptionT< bool >;
Common_TEMPLATE template class OptionT< int >;
Common_TEMPLATE template class OptionT< std::string >;
Common_TEMPLATE template class OptionT< cf3::Uint >;
Common_TEMPLATE template class OptionT< cf3::Real >;
Common_TEMPLATE template class OptionT< cf3::common::URI >;

//vivian bolsee
Common_TEMPLATE template class OptionT< cf3::common::UUCount >;

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
