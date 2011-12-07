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

#include "common/XML/Protocol.hpp"


using namespace cf3::common::XML;

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

template < typename TYPE>
OptionT<TYPE>::OptionT ( const std::string& name, value_type def) :
    Option(name, def)
{
//    CFinfo
//        << " creating OptionT [" << m_name << "]"
//        << " of type [" << m_type << "]"
//        << " w default [" << def_str() << "]"
//        << " w desc [" << m_description << "]\n"
//        << CFendl;
  m_restricted_list.push_back(def);
}

////////////////////////////////////////////////////////////////////////////////

template < typename TYPE>
void OptionT<TYPE>::configure ( XmlNode& node )
{
  TYPE val;
  const char * type_str = Protocol::Tags::type<TYPE>();
  XmlNode type_node(node.content->first_node(type_str));

  if( type_node.is_valid() )
    val = from_str<TYPE>( type_node.content->value() );
  else
    throw XmlError(FromHere(), "Could not find a value of this type ["+std::string(type_str)+"].");

  m_value = val;
  copy_to_linked_params(val);
}


template < typename TYPE >
void OptionT<TYPE>::copy_to_linked_params (const boost::any& val )
{
  BOOST_FOREACH ( void* v, this->m_linked_params )
  {
    TYPE* cv = static_cast<TYPE*>(v);
    try
    {
      *cv = boost::any_cast<TYPE>(val);
    }
    catch(boost::bad_any_cast& e)
    {
      throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(val.type())+" to "+common::class_name<TYPE>());
    }

  }
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
std::string OptionT<TYPE>::def_str () const
{
  return to_str( def<TYPE>() );
}

////////////////////////////////////////////////////////////////////////////////

/// explicit instantiation to avoid missing symbols in certain compilers
Common_TEMPLATE template class OptionT< bool >;
Common_TEMPLATE template class OptionT< int >;
Common_TEMPLATE template class OptionT< std::string >;
Common_TEMPLATE template class OptionT< cf3::Uint >;
Common_TEMPLATE template class OptionT< cf3::Real >;
//Common_TEMPLATE template class OptionT< cf3::common::URI >;

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
