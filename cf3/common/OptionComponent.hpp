// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_OptionComponent_hpp
#define cf3_common_OptionComponent_hpp

///////////////////////////////////////////////////////////////////////////////

#include "rapidxml/rapidxml.hpp"

#include <boost/foreach.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/remove_const.hpp>

#include "common/Option.hpp"
#include "common/Component.hpp"

#include "common/Core.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/CastingFunctions.hpp"

using namespace cf3::common::XML;

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////

template <typename T = Component>
class Common_API OptionComponent : public Option
{
public:

  typedef Handle<T> value_type;

  OptionComponent(const std::string & name, const Handle<T> linked_component)
    : Option(name, linked_component)
  {
  }

  virtual ~OptionComponent() {}

  /// @returns the value as a std::string
  virtual std::string value_str () const
  {
    value_type comp = Option::template value<value_type>();
    if(is_null(comp))
      return URI().string();

    return comp->uri().string();
  }

  /// The returned value contains a Component handle, useful for dynamic use in i.e. Python
  virtual boost::any value() const
  {
    // Determine if we should use a handle-to-const or not
    typedef typename boost::mpl::if_< boost::is_const<T>, Handle<Component const>, Handle<Component> >::type BaseT;

    return BaseT(Option::template value<value_type>());
  }

  /// Redirect form the base class due to conflict with the non-template
  template<typename OutputT>
  OutputT value() const
  {
    return Option::template value<OutputT>();
  }

  virtual std::string restricted_list_str() const
  {
    std::vector< URI > restr_list_vec;
    BOOST_FOREACH(const boost::any& restr_item, restricted_list())
    {
      value_type comp = boost::any_cast<value_type>(restr_item);
      if(is_null(comp))
        restr_list_vec.push_back(URI().string());
      else
        restr_list_vec.push_back(comp->uri().string());
    }
    return option_vector_to_str(restr_list_vec, separator());
  }

  virtual void set_restricted_list_str(const std::vector< std::string >& list)
  {
    BOOST_FOREACH(const std::string& item, list)
    {
      const URI comp_path(item, cf3::common::URI::Scheme::CPATH);
      Handle<T> found_comp(Core::instance().root().access_component(comp_path));
      if(is_not_null(found_comp))
        restricted_list().push_back(found_comp);
    }
  }
  
  virtual std::string type() const
  {
    return class_name< Handle<typename boost::remove_const<T>::type> >();
  }

private:

  virtual void change_value_impl(const boost::any& value)
  {
    // Allowed types to be held by the any
    typedef typename boost::mpl::if_
    <
    boost::is_const<T>,
    // const and non-const may be assigned to a const handle
    boost::mpl::vector4< Handle<Component>, Handle<Component const>, Handle<typename boost::remove_const<T>::type>, Handle<T> >,
    // only non-const may be assigned to a non-const handle
    boost::mpl::vector2< Handle<Component>, Handle<T> >
    >::type AllowedTypes;

    bool success = false;
    boost::mpl::for_each<AllowedTypes>(ValueExtractor(name(), value, m_value, success));

    if(!success)
    {
      throw CastingFailed(FromHere(), "Bad value of type " + demangle(value.type().name()) + " passed where handle of type " + T::type_name() + " was expected for option " + name());
    }
  }

  /// updates the option value using the xml configuration
  /// @param node XML node with data for this option
  virtual boost::any extract_configured_value(XML::XmlNode& node)
  {
    URI uri;
    // Try concrete type first
    XmlNode type_node(node.content->first_node(type().c_str()));
    // Otherwise try the base component
    if(!type_node.is_valid())
      type_node = XmlNode(node.content->first_node("handle[cf3.common.Component]"));

    if( type_node.is_valid() )
      to_value( type_node, uri );
    else
      throw XmlError(FromHere(), "Could not find a value for this option.");

    value_type val(Core::instance().root().access_component(uri));
    if(is_null(val))
      throw SetupError(FromHere(), "Failed to find the component located at " + uri.string());

    return val;
  }

  virtual void copy_to_linked_params(std::vector< boost::any >& linked_params)
  {
    if(linked_params.empty())
      return;

    value_type val = Option::template value<value_type>();
    BOOST_FOREACH ( boost::any& v, linked_params )
    {
      value_type* cv = boost::any_cast<value_type*>(v);
      *cv = val;
    }
  }

  /// MPL functor to extract a value from an any
  struct ValueExtractor
  {
    ValueExtractor(const std::string& opt_name, const boost::any& input_val, boost::any& output_val, bool& b) :
      option_name(opt_name),
      input_value(input_val),
      output_value(output_val),
      success(b)
    {
    }

    template<typename HandleT>
    void operator()(const HandleT)
    {
      // Value was found, we're done
      if(success)
        return;

      const HandleT* any_handle = boost::any_cast< HandleT >(&input_value); // test any cast
      if(is_not_null(any_handle)) // if we have a successful any-cast
      {
        Handle<T> result(*any_handle); // try to cast to concrete type
        if(is_not_null(result) || is_null(*any_handle))
        {
          success = true;
          output_value = result;
        }
        else
        {
          throw CastingFailed(FromHere(), "Could not cast OptionComponent value from type " + (*any_handle)->derived_type_name() + " to type " + T::type_name() + " for option " + option_name);
        }
      }
    }


    const std::string option_name;
    const boost::any& input_value;
    boost::any& output_value;
    bool& success;
  };
}; // class OptionComponent

/////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

///////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_OptionComponent_hpp
