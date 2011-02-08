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
#include "Common/Log.hpp"

///////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////

template <typename T = Component>
class Common_API OptionComponent : public Option
{

public:

  typedef URI value_type;
  typedef boost::weak_ptr<T> data_t;
  typedef boost::shared_ptr<OptionComponent> Ptr;
  typedef boost::shared_ptr<OptionComponent const> ConstPtr;

  OptionComponent(const std::string & name, const std::string & desc, const URI & def)
    : Option(name, desc, data_t(Core::instance().root()->look_component<T>(def)))
  { 
    TypeInfo::instance().regist<data_t>("CF::Common::OptionComponent<"+T::type_name()+">::data_t");    
  }

  OptionComponent(const std::string & name, const std::string & desc, data_t* linked_component)
    : Option(name, desc, data_t(*linked_component))
  {    
    TypeInfo::instance().regist<data_t>("CF::Common::OptionComponent<"+T::type_name()+">::data_t");    
    m_linked_params.push_back(linked_component);
  }

  static Option::Ptr create(const std::string & name, const std::string & desc, data_t* linked_component)
  {
    return Option::Ptr ( new OptionComponent<T>(name,desc,linked_component) );
  }
  
  virtual ~OptionComponent() {}

  /// @name VIRTUAL FUNCTIONS
  //@{

  virtual std::string type() const { return XmlTag<URI>::type(); }
  
  virtual std::string data_type() const { return class_name<data_t>(); }
  
  /// @returns the xml tag for this option
  virtual const char * tag() const { return XmlTag<URI>::type(); }

  /// @returns the value as a std::string
  virtual std::string value_str () const
  { 
    data_t val = boost::any_cast< data_t >(m_value);
    if (is_null(val.lock()))
      return "cpath:";
    return val.lock()->full_path().string(); 
  }

  /// @returns the default value as a sd::string
  virtual std::string def_str () const
  { 
    data_t val = boost::any_cast< data_t >(m_default);
    if (is_null(val.lock()))
      return "cpath:";
    return val.lock()->full_path().string(); 
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

    m_value = value_to_data(val);
    
  }

  //@} END VIRTUAL FUNCTIONS

protected: // functions


  virtual boost::any value_to_data( const boost::any& value)
  {
    return data_t(Core::instance().root()->look_component<T>(boost::any_cast<URI>(value)));
  }

  /// copy the configured update value to all linked parameters
  virtual void copy_to_linked_params ( const boost::any& data )
  {
    BOOST_FOREACH ( void* v, this->m_linked_params )
    {
      data_t* cv = static_cast<data_t*>(v);
      *cv = boost::any_cast<data_t>(data);
    }
  }

}; // class OptionComponent

/////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

///////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionComponent_hpp
