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
    TypeInfo::instance().regist<value_type>("cf3::common::OptionComponent<"+T::type_name()+">::value_type");
  }

  virtual ~OptionComponent() {}

  virtual std::string type() const { return class_name<value_type>(); }

  /// @returns the xml tag for this option
  virtual const char * tag() const { return Protocol::Tags::type<URI>(); }

  /// @returns the value as a std::string
  virtual std::string value_str () const
  {
    value_type comp = this->template value<value_type>();
    if(is_null(comp))
      return URI().string();

    return comp->uri().string();
  }

private:

  /// updates the option value using the xml configuration
  /// @param node XML node with data for this option
  virtual boost::any extract_configured_value(XML::XmlNode& node)
  {
    URI uri;
    XmlNode type_node(node.content->first_node(Protocol::Tags::type<URI>()));

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

    value_type val = this->template value<value_type>();
    BOOST_FOREACH ( boost::any& v, linked_params )
    {
      value_type* cv = boost::any_cast<value_type*>(v);
      *cv = val;
    }
  }
}; // class OptionComponent

/////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

///////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_OptionComponent_hpp
