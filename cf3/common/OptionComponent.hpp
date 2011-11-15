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

#include "common/URI.hpp"
#include "common/OptionURI.hpp"
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
class Common_API OptionComponent : public OptionURI
{

public:

  typedef URI value_type;
  typedef Handle<T> data_t;
  typedef boost::shared_ptr<OptionComponent> Ptr;
  typedef boost::shared_ptr<OptionComponent const> ConstPtr;

  OptionComponent(const std::string & name, const URI & def)
    : OptionURI(name, def)
  {
    Handle<Component> comp_ptr = Core::instance().root().access_component(def);
    typename T::Ptr component = boost::dynamic_pointer_cast<T>(comp_ptr);
    m_default = data_t(component);
    m_value = m_default;
    supported_protocol(URI::Scheme::CPATH);

    TypeInfo::instance().regist<data_t>("cf3::common::OptionComponent<"+T::type_name()+">::data_t");
  }

  OptionComponent(const std::string & name, data_t* linked_component)
    : OptionURI(name, linked_component->expired()? URI("cpath:") : linked_component->lock()->uri())
  {
    m_default = data_t(*linked_component);
    m_value = m_default;
    supported_protocol(URI::Scheme::CPATH);

    TypeInfo::instance().regist<data_t>("cf3::common::OptionComponent<"+T::type_name()+">::data_t");
    m_linked_params.push_back(linked_component);
  }

  static Option::Ptr create(const std::string & name, data_t* linked_component)
  {
    return Option::Ptr ( new OptionComponent<T>(name, linked_component) );
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
      throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(m_value.type())+" to "+common::class_name<data_t>());
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
      throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(m_default.type())+" to "+common::class_name<data_t>());
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

  /// Check if the stored component is valid
  bool check() const
  {
    return !boost::any_cast<data_t>(m_value).expired();
  }

  /// Get a reference to the stored component
  T& component()
  {
    try
    {
      data_t val = boost::any_cast< data_t >(m_value);
      return *val.lock();
    }
    catch(boost::bad_any_cast& e)
    {
      throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(m_value.type())+" to "+class_name<data_t>());
    }
  }

  T const& component() const
  {
    try
    {
      data_t val = boost::any_cast< data_t >(m_value);
      return *val.lock();
    }
    catch(boost::bad_any_cast& e)
    {
      throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(m_value.type())+" to "+class_name<data_t>());
    }
  }

protected: // functions


  virtual boost::any value_to_data( const boost::any& value) const
  {
    try
    {
      // try casting to a shared_ptr
      return data_t (boost::any_cast< boost::shared_ptr<T> >(value) );
    }
    catch(boost::bad_any_cast&)
    {
      try
      {
        // try casting to a weak_ptr
        return data_t (boost::any_cast< boost::weak_ptr<T> >(value) );
      }
      catch(boost::bad_any_cast&)
      {
        // finally try a URI
        try
        {
          return data_t(Core::instance().root().access_component_checked(boost::any_cast<URI const>(value)));
        }
        catch(boost::bad_any_cast& e)
        {
          throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(value.type())+" to "+common::class_name<URI>());
        }
      }
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
      throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(data.type())+" to "+common::class_name<data_t>());
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
        throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(data.type())+" to "+common::class_name<data_t>());
      }
    }
  }

}; // class OptionComponent

/////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

///////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_OptionComponent_hpp
