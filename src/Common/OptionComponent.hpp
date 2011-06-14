// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_OptionComponent_hpp
#define CF_Common_OptionComponent_hpp

///////////////////////////////////////////////////////////////////////////////

#include "rapidxml/rapidxml.hpp"

#include <boost/foreach.hpp>

#include "Common/URI.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Component.hpp"

#include "Common/CRoot.hpp"
#include "Common/Core.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/CastingFunctions.hpp"

using namespace CF::Common::XML;

///////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////

template <typename T = Component>
class Common_API OptionComponent : public OptionURI
{

public:

  typedef URI value_type;
  typedef boost::weak_ptr<T> data_t;
  typedef boost::shared_ptr<OptionComponent> Ptr;
  typedef boost::shared_ptr<OptionComponent const> ConstPtr;

  OptionComponent(const std::string & name, const std::string & desc, const URI & def)
    : OptionURI(name, desc, def)
  {
    typename T::Ptr component = Core::instance().root().access_component_ptr(def)->as_ptr<T>();
    m_default = data_t(component);
    m_value = m_default;
    supported_protocol(URI::Scheme::CPATH);

    TypeInfo::instance().regist<data_t>("CF::Common::OptionComponent<"+T::type_name()+">::data_t");
  }

  OptionComponent(const std::string & name, const std::string & desc, data_t* linked_component)
    : OptionURI(name, desc, linked_component->expired()? URI("cpath:") : linked_component->lock()->uri())
  {
    m_default = data_t(*linked_component);
    m_value = m_default;
    supported_protocol(URI::Scheme::CPATH);

    TypeInfo::instance().regist<data_t>("CF::Common::OptionComponent<"+T::type_name()+">::data_t");
    m_linked_params.push_back(linked_component);
  }
  
  OptionComponent(const std::string & name, const std::string& readable_name, const std::string & desc, const URI & def)
    : OptionURI(name, readable_name, desc, def)
  {
    typename T::Ptr component = Core::instance().root().access_component(def).as_ptr<T>();
    m_default = data_t(component);
    m_value = m_default;
    supported_protocol(URI::Scheme::CPATH);

    TypeInfo::instance().regist<data_t>("CF::Common::OptionComponent<"+T::type_name()+">::data_t");
  }

  OptionComponent(const std::string & name, const std::string& readable_name, const std::string & desc, data_t* linked_component)
    : OptionURI(name, readable_name, desc, linked_component->expired()? URI("cpath:") : linked_component->lock()->uri())
  {
    m_default = data_t(*linked_component);
    m_value = m_default;
    supported_protocol(URI::Scheme::CPATH);

    TypeInfo::instance().regist<data_t>("CF::Common::OptionComponent<"+T::type_name()+">::data_t");
    m_linked_params.push_back(linked_component);
  }
  

  static Option::Ptr create(const std::string & name, const std::string& readable_name, const std::string & desc, data_t* linked_component)
  {
    return Option::Ptr ( new OptionComponent<T>(name,readable_name,desc,linked_component) );
  }

  static Option::Ptr create(const std::string & name, const std::string & desc, data_t* linked_component)
  {
    return create(name,name,desc,linked_component);
  }

  virtual ~OptionComponent() {}

  /// @name VIRTUAL FUNCTIONS
  //@{

  virtual std::string type() const { return Protocol::Tags::type<URI>(); }

  virtual std::string data_type() const { return class_name<data_t>(); }

  /// @returns the xml tag for this option
  virtual const char * tag() const { return Protocol::Tags::type<URI>(); }

  /// @returns the value as a std::string
  virtual std::string value_str () const
  {
    try
    {
      data_t val = boost::any_cast< data_t >(m_value);
      if (is_null(val.lock()))
        return "cpath:";
      return val.lock()->uri().string();
    }
    catch(boost::bad_any_cast& e)
    {
      throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(m_value.type())+" to "+Common::class_name<data_t>());
    }
  }

  /// @returns the default value as a sd::string
  virtual std::string def_str () const
  {
    try
    {
      data_t val = boost::any_cast< data_t >(m_default);
      if (is_null(val.lock()))
        return "cpath:";
      return val.lock()->uri().string();
    }
    catch(boost::bad_any_cast& e)
    {
      throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(m_default.type())+" to "+Common::class_name<data_t>());
    }
  }

  /// updates the option value using the xml configuration
  /// @param node XML node with data for this option
  virtual void configure ( XmlNode& node )
  {
    URI val;
    XmlNode type_node(node.content->first_node(Protocol::Tags::type<URI>()));

    if( type_node.is_valid() )
      to_value( type_node, val );
    else
      throw XmlError(FromHere(), "Could not find a value for this option.");

    m_value = value_to_data(val);

  }

  //@} END VIRTUAL FUNCTIONS
  
  /// Get a reference to the stored component
  T& component()
  {
    data_t val = boost::any_cast< data_t >(m_value);
    return *val.lock();
  }
  
  T const& component() const
  {
    data_t val = boost::any_cast< data_t >(m_value);
    return *val.lock();
  }

protected: // functions


  virtual boost::any value_to_data( const boost::any& value) const
  {
    try
    {
      return data_t(Core::instance().root().access_component_ptr_checked(boost::any_cast<URI const>(value))->as_ptr_checked<T>() );
    }
    catch(boost::bad_any_cast& e)
    {
      throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(value.type())+" to "+Common::class_name<URI>());
    }
  }

  virtual boost::any data_to_value( const boost::any& data) const
  {
    try
    {
      boost::shared_ptr<T> data_shared = boost::any_cast<data_t>(data).lock();
      if ( is_null(data_shared) )
        return URI("cpath:");
      else
        return data_shared->uri();
    }
    catch(boost::bad_any_cast& e)
    {
      throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(data.type())+" to "+Common::class_name<data_t>());
    }
  }

  /// copy the configured update value to all linked parameters
  virtual void copy_to_linked_params ( const boost::any& data )
  {
    BOOST_FOREACH ( void* v, this->m_linked_params )
    {
      data_t* cv = static_cast<data_t*>(v);
      try
      {
        *cv = boost::any_cast<data_t>(data);
      }
      catch(boost::bad_any_cast& e)
      {
        throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(data.type())+" to "+Common::class_name<data_t>());
      }
    }
  }

}; // class OptionComponent

/////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

///////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionComponent_hpp
