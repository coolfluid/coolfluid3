// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include "Common/BoostFilesystem.hpp"

#include "rapidxml/rapidxml.hpp"

#include "Common/OptionArray.hpp"
#include "Common/StringConversion.hpp"
#include "Common/URI.hpp"
#include "Common/Log.hpp"

#include "Common/XML/Map.hpp"
#include "Common/XML/CastingFunctions.hpp"

using namespace CF::Common::XML;

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

OptionArray::OptionArray(const std::string& name, const boost::any def) :
    Option(name, def)
{
}

OptionArray::~OptionArray() {}

////////////////////////////////////////////////////////////////////////////////

template < typename TYPE>
OptionArrayT<TYPE>::OptionArrayT ( const std::string& name, const value_type& def) :
  OptionArray(name, def)
{
  //    CFinfo
  //        << " creating OptionArray of " << elem_type() <<  "\'s [" << m_name << "]"
  //        << " of type [" << m_type << "]"
  //        << " w default [" << def_str() << "]"
  //        << " w desc [" << m_description << "]\n"
  //        << CFendl;

  typename value_type::const_iterator it = def.begin();

  for( ; it != def.end() ; it++)
    restricted_list().push_back(*it);
}

////////////////////////////////////////////////////////////////////////////////

template < typename TYPE >
    void OptionArrayT<TYPE>::configure ( XmlNode& node )
{
  rapidxml::xml_attribute<>* attr = node.content->first_attribute( "type" );
//  rapidxml::xml_attribute<> * delim_attr = nullptr;
//  XmlNode itr = node.content->first_node();
//  using namespace boost::algorithm;


  if ( !attr )
    throw ParsingFailed (FromHere(), "OptionArray does not have \'type\' attribute" );

  if ( strcmp(attr->value(),elem_type()) )
    throw ParsingFailed (FromHere(), "OptionArray expected \'type\' attribute \'"
                         +  std::string(attr->value())
                         + "\' but got \'"
                         +  std::string(elem_type()) + "\'"  );

  m_value = Map().array_to_vector<TYPE>( node );

  copy_to_linked_params(m_value);
}

template < typename TYPE >
void OptionArrayT<TYPE>::copy_to_linked_params ( const boost::any& val )
{
  BOOST_FOREACH ( void* v, this->m_linked_params )
  {
    value_type* cv = static_cast<value_type*>(v);
    try
    {
      *cv = boost::any_cast<value_type>(val);
    }
    catch(boost::bad_any_cast& e)
    {
      throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(val.type())+" to "+Common::class_name<value_type>());
    }
  }
}

template < typename TYPE >
std::string OptionArrayT<TYPE>::value_str () const
{
  return dump_to_str(m_value);
}

template < typename TYPE >
std::string OptionArrayT<TYPE>::def_str () const
{
  return "";//dump_to_str(m_default);
}

template < typename TYPE >
std::string OptionArrayT<TYPE>::dump_to_str ( const boost::any& c ) const
{
  std::string result;

  try
  {
    value_type values = boost::any_cast<value_type>(c);
    BOOST_FOREACH ( TYPE v, values )
    {
      if(!result.empty())
        result += separator();

      result += to_str( v );
    }
  }
  catch(boost::bad_any_cast& e)
  {
    throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(c.type())+" to "+Common::class_name<value_type>());
  }

  return result;
}

template<typename TYPE>
std::vector<TYPE> OptionArrayT<TYPE>::value_vect() const
{
  try
  {
    return boost::any_cast<value_type>(m_value);
  }
  catch(boost::bad_any_cast& e)
  {
    throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(m_value.type())+" to "+Common::class_name<value_type>());
  }
}

////////////////////////////////////////////////////////////////////////////////


/// explicit instantiation to avoid missing symbols in certain compilers
Common_TEMPLATE template class OptionArrayT< bool >;
Common_TEMPLATE template class OptionArrayT< int >;
Common_TEMPLATE template class OptionArrayT< std::string >;
Common_TEMPLATE template class OptionArrayT< CF::Uint >;
Common_TEMPLATE template class OptionArrayT< CF::Real >;
Common_TEMPLATE template class OptionArrayT< CF::Common::URI >;

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
