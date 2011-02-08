// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_OptionComponent_hpp
#define CF_Common_OptionComponent_hpp

///////////////////////////////////////////////////////////////////////////////

#include "Common/URI.hpp"
#include "Common/Option.hpp"
#include "Common/Component.hpp"

#include <boost/foreach.hpp>
#include "Common/CRoot.hpp"
#include "Common/Core.hpp"

///////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////

template <typename T = Component>
class Common_API OptionComponent : public Option
{

public:

  typedef boost::weak_ptr<T> linked_type;
  typedef URI value_type;
  typedef boost::shared_ptr<OptionComponent> Ptr;
  typedef boost::shared_ptr<OptionComponent const> ConstPtr;

  OptionComponent(const std::string & name, const std::string & desc, const URI & def)
    : Option(name, desc, linked_type(Core::instance().root()->look_component<T>(def)))
  {
    TypeInfo::instance().regist<linked_type>("CF::Common::OptionComponent<"+T::type_name()+">::value_type");
  }

  virtual ~OptionComponent() {}

  /// @name VIRTUAL FUNCTIONS
  //@{

  /// @returns the xml tag for this option
  virtual const char * tag() const { return XmlTag<URI>::type(); }

  /// @returns the value as a sd::string
  virtual std::string value_str () const
  { 
    linked_type val = boost::any_cast< linked_type >(m_value);

    if (is_null(val.lock()))
      throw ValueNotFound(FromHere(),"option_value invalid");

    return val.lock()->full_path().string(); 
  }

  /// @returns the default value as a sd::string
  virtual std::string def_str () const
  { 
      return from_value( def<URI>() ); 
  }

  /// updates the option value using the xml configuration
  /// @param node XML node with data for this option
  virtual void configure ( XmlNode& node )
  {
    URI val;
    XmlNode * type_node = node.first_node(XmlTag<URI>::type());

    if(type_node != nullptr)
      to_value(*type_node, val);
    else
      throw XmlError(FromHere(), "Could not find a value for this option.");

    m_value = linked_type(Core::instance().root()->look_component<T>(val));
    
  }

  //@} END VIRTUAL FUNCTIONS

protected: // functions

  /// copy the configured update value to all linked parameters
  virtual void copy_to_linked_params ( const boost::any& val )
  {
    BOOST_FOREACH ( void* v, this->m_linked_params )
    {
      linked_type* cv = static_cast<linked_type*>(v);
      *cv = linked_type(Core::instance().root()->look_component<T>(boost::any_cast<URI>(val)));
    }
  }

}; // class OptionComponent

/////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

///////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionComponent_hpp
