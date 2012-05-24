// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "common/BoostFilesystem.hpp"

#include "rapidxml/rapidxml.hpp"

#include "common/OptionArray.hpp"
#include "common/StringConversion.hpp"
#include "common/URI.hpp"
#include "common/BasicExceptions.hpp"

#include "common/XML/Map.hpp"
#include "common/XML/CastingFunctions.hpp"

using namespace cf3::common::XML;

namespace cf3 {
namespace common {

namespace detail
{
  // Handle the int <-> Uint conflicts
  /// Helper function to set the value
  template<typename TYPE>
  void change_array_value(boost::any& to_set, const boost::any& new_value)
  {
    cf3_assert(new_value.type() == to_set.type());
    to_set = new_value; // update the value
  }

  template<>
  void change_array_value<Uint>(boost::any& to_set, const boost::any& new_value)
  {
    if(new_value.type() == to_set.type())
    {
      to_set = new_value;
    }
    else
    {
      try
      {
        std::vector<int> int_vals = boost::any_cast< std::vector<int> >(new_value);
        const Uint nb_vals = int_vals.size();
        std::vector<Uint> result(nb_vals);
        for(Uint i = 0; i != nb_vals; ++i)
        {
          if(int_vals[i] < 0)
            throw BadValue(FromHere(), "Tried to store a negative value in an unsigned int option array at index " + boost::lexical_cast<std::string>(i));
          result[i] = static_cast<Uint>(int_vals[i]);
        }
        to_set = result;
      }
      catch(boost::bad_any_cast& e)
      {
        throw CastingFailed(FromHere(), std::string("Failed to cast object of type ") + new_value.type().name() + " to type std::vector<Uint>");
      }
    }
  }

  template<>
  void change_array_value<int>(boost::any& to_set, const boost::any& new_value)
  {
    if(new_value.type() == to_set.type())
    {
      to_set = new_value;
    }
    else
    {
      try
      {
        std::vector<Uint> int_vals = boost::any_cast< std::vector<Uint> >(new_value);
        const Uint nb_vals = int_vals.size();
        std::vector<int> result(nb_vals);
        for(Uint i = 0; i != nb_vals; ++i)
        {
          result[i] = static_cast<int>(int_vals[i]);
        }
        to_set = result;
      }
      catch(boost::bad_any_cast& e)
      {
        throw CastingFailed(FromHere(), std::string("Failed to cast object of type ") + new_value.type().name() + " to type std::vector<int>");
      }
    }
  }

  template<>
  void change_array_value<Real>(boost::any& to_set, const boost::any& new_value)
  {
    if(new_value.type() == to_set.type())
    {
      to_set = new_value;
    }
    else
    {
      try
      {
        std::vector<int> int_vals = boost::any_cast< std::vector<int> >(new_value);
        const Uint nb_vals = int_vals.size();
        std::vector<Real> result(nb_vals);
        for(Uint i = 0; i != nb_vals; ++i)
        {
          result[i] = static_cast<Real>(int_vals[i]);
        }
        to_set = result;
      }
      catch(boost::bad_any_cast& e)
      {
        throw CastingFailed(FromHere(), std::string("Failed to cast object of type ") + new_value.type().name() + " to type std::vector<Real>");
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

template < typename TYPE>
OptionArray<TYPE>::OptionArray ( const std::string& name, const value_type& def) :
  Option(name, def)
{
}

////////////////////////////////////////////////////////////////////////////////

template < typename TYPE>
boost::any OptionArray<TYPE>::extract_configured_value(XmlNode& node)
{
  rapidxml::xml_attribute<>* attr = node.content->first_attribute( "type" );

  if ( !attr )
    throw ParsingFailed (FromHere(), "OptionArray does not have \'type\' attribute" );

  if ( strcmp(attr->value(),element_type().c_str()) )
    throw ParsingFailed (FromHere(), "OptionArray expected \'type\' attribute \'"
                         +  std::string(attr->value())
                         + "\' but got \'"
                         +  std::string(element_type()) + "\'"  );

  return Map().array_to_vector<TYPE>( node );
}

////////////////////////////////////////////////////////////////////////////////

template < typename TYPE >
void OptionArray<TYPE>::copy_to_linked_params(std::vector< boost::any >& linked_params )
{
  std::vector<TYPE> val = this->template value< std::vector<TYPE> >();
  BOOST_FOREACH ( boost::any& v, linked_params )
  {
    std::vector<TYPE>* cv = boost::any_cast<std::vector<TYPE>*>(v);
    *cv = val;
  }
}

template<typename TYPE>
void OptionArray<TYPE>::change_value_impl(const boost::any& value)
{
  detail::change_array_value<TYPE>(m_value, value);
}


////////////////////////////////////////////////////////////////////////////////

/// explicit instantiation to avoid missing symbols in certain compilers
Common_TEMPLATE template class OptionArray< bool >;
Common_TEMPLATE template class OptionArray< int >;
Common_TEMPLATE template class OptionArray< std::string >;
Common_TEMPLATE template class OptionArray< cf3::Uint >;
Common_TEMPLATE template class OptionArray< cf3::Real >;
Common_TEMPLATE template class OptionArray< cf3::common::URI >;

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
