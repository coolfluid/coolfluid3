// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/filesystem/path.hpp>

#include "Common/OptionArray.hpp"
#include "Common/URI.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

OptionArray::OptionArray(const std::string& name,
                        const std::string& desc, const boost::any def) :
    Option(name, desc, def)
{
}

OptionArray::~OptionArray() {}

////////////////////////////////////////////////////////////////////////////////

template < typename TYPE>
OptionArrayT<TYPE>::OptionArrayT ( const std::string& name, const std::string& desc, const value_type& def) :
  OptionArray(name, desc, def)
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

template < typename TYPE >
void OptionArrayT<TYPE>::configure ( XmlNode& node )
{
  XmlAttr *attr = node.first_attribute( "type" );

  if ( !attr )
    throw ParsingFailed (FromHere(), "OptionArray does not have \'type\' attribute" );

  if ( strcmp(attr->value(),elem_type()) )
    throw ParsingFailed (FromHere(), "OptionArray expected \'type\' attribute \'"
                         +  std::string(attr->value())
                         + "\' but got \'"
                         +  std::string(elem_type()) + "\'"  );

  value_type val; // empty vector

  for (XmlNode * itr = node.first_node(); itr ; itr = itr->next_sibling() )
  {
    val.push_back(to_value<TYPE>(*itr));
  }

  XmlAttr *size_attr = node.first_attribute( "size" );
  if ( !size_attr )
    throw ParsingFailed (FromHere(), "OptionArray does not have \'size\' attribute" );

  Uint expected_size = 0;
  to_value(*size_attr,expected_size);
  if ( expected_size != val.size() )
    throw ParsingFailed (FromHere(), "OptionArray \'size\' did not match number of entries" );

  m_value = val;
  copy_to_linked_params(val);
}

template < typename TYPE >
void OptionArrayT<TYPE>::copy_to_linked_params ( const value_type& val )
{
  BOOST_FOREACH ( void* v, this->m_linked_params )
  {
    value_type* cv = static_cast<value_type*>(v);
    *cv = val;
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
  value_type values = boost::any_cast<value_type>(c);
  std::string result;

  BOOST_FOREACH ( TYPE v, values )
  {
    result += from_value ( v );
    result += ":";
  }


  if ( !result.empty() ) // remove last ":"
    result.erase(result.size()-1);

  return result;
}

template<typename TYPE>
std::vector<TYPE> OptionArrayT<TYPE>::value_vect() const
{
  return boost::any_cast<value_type>(m_value);
}

////////////////////////////////////////////////////////////////////////////////


/// explicit instantiation to avoid missing symbols in certain compilers
Common_TEMPLATE template class OptionArrayT< bool >;
Common_TEMPLATE template class OptionArrayT< int >;
Common_TEMPLATE template class OptionArrayT< std::string >;
Common_TEMPLATE template class OptionArrayT< CF::Uint >;
Common_TEMPLATE template class OptionArrayT< CF::Real >;
Common_TEMPLATE template class OptionArrayT< CF::Common::URI >;
Common_TEMPLATE template class OptionArrayT< boost::filesystem::path >;

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
