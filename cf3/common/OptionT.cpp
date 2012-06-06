// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "rapidxml/rapidxml.hpp"

#include "common/BoostFilesystem.hpp"
#include "common/BasicExceptions.hpp"
#include "common/OptionFactory.hpp"
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

  template<>
  void change_value<Real>(boost::any& to_set, const boost::any& new_value)
  {
    if(new_value.type() == to_set.type())
    {
      to_set = new_value;
    }
    else
    {
      try
      {
        int uval = boost::any_cast<int>(new_value);
        to_set = static_cast<Real>(uval);
      }
      catch(boost::bad_any_cast& e)
      {
        throw CastingFailed(FromHere(), std::string("Failed to cast object of type ") + new_value.type().name() + " to type Real");
      }
    }
  }

  template<typename TYPE>
  boost::any extract_configured_value(XmlNode& node)
  {
    const std::string type_str = common::class_name<TYPE>();
    XmlNode type_node(node.content->first_node(type_str.c_str()));
    if(!type_node.is_valid() && type_str == "real")
      type_node = XmlNode(node.content->first_node("integer"));

    if( type_node.is_valid() )
      return from_str<TYPE>( type_node.content->value() );
    else
      throw XmlError(FromHere(), "Could not find a value of this type ["+std::string(type_str)+"].");
  }

  /// Extracts from both int and Uint nodes
  boost::any extract_int_types(XmlNode& node)
  {
    XmlNode type_node_int(node.content->first_node(common::class_name<int>().c_str()));
    XmlNode type_node_uint(node.content->first_node(common::class_name<Uint>().c_str()));

    if(type_node_int.is_valid())
      return from_str<int>( type_node_int.content->value() );
    else if(type_node_uint.is_valid())
      return from_str<Uint>( type_node_uint.content->value() );
    else
      throw XmlError(FromHere(), "Could not find a value of either signed or unsigned integer type.");
  }

  template<>
  boost::any extract_configured_value<Uint>(XmlNode& node)
  {
    return extract_int_types(node);
  }

  template<>
  boost::any extract_configured_value<int>(XmlNode& node)
  {
    return extract_int_types(node);
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
  return detail::extract_configured_value<TYPE>(node);
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

template<typename TYPE>
std::string OptionT<TYPE>::restricted_list_str() const
{
  std::vector<TYPE> restr_list_vec;
  BOOST_FOREACH(const boost::any& restr_item, restricted_list())
  {
    restr_list_vec.push_back(boost::any_cast<TYPE>(restr_item));
  }
  return option_vector_to_str(restr_list_vec, separator());
}

template<typename TYPE>
void OptionT<TYPE>::set_restricted_list_str(const std::vector< std::string >& list)
{
  BOOST_FOREACH(const std::string& item, list)
  {
    restricted_list().push_back(from_str<TYPE>(item));
  }
}


////////////////////////////////////////////////////////////////////////////////

/// Builders for OptionT components
template<typename TYPE>
class OptionTBuilder : public OptionBuilder
{
public:
  virtual boost::shared_ptr< Option > create_option(const std::string& name, const boost::any& default_value)
  {
    const TYPE val = from_str<TYPE>(boost::any_cast<std::string>(default_value));
    return boost::shared_ptr<Option>(new OptionT<TYPE>(name, val));
  }
};

// register the different builders
RegisterOptionBuilder bool_builder(common::class_name<bool>(), new OptionTBuilder<bool>());
RegisterOptionBuilder int_builder(common::class_name<int>(), new OptionTBuilder<int>());
RegisterOptionBuilder string_builder(common::class_name<std::string>(), new OptionTBuilder<std::string>());
RegisterOptionBuilder uint_builder(common::class_name<Uint>(), new OptionTBuilder<Uint>());
RegisterOptionBuilder Real_builder(common::class_name<Real>(), new OptionTBuilder<Real>());
RegisterOptionBuilder uucount_builder(common::class_name<UUCount>(), new OptionTBuilder<UUCount>());

////////////////////////////////////////////////////////////////////////////////

/// explicit instantiation to avoid missing symbols in certain compilers
Common_TEMPLATE template class OptionT< bool >;
Common_TEMPLATE template class OptionT< int >;
Common_TEMPLATE template class OptionT< std::string >;
Common_TEMPLATE template class OptionT< cf3::Uint >;
Common_TEMPLATE template class OptionT< cf3::Real >;
Common_TEMPLATE template class OptionT< cf3::common::URI >;
Common_TEMPLATE template class OptionT< cf3::common::UUCount >;

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
