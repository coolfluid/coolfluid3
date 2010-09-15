// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/Property.hpp"
#include "Common/URI.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  Property::Property ( const std::string& name,
                   const std::string& type,
                   const std::string& desc,
                   boost::any def,
                   bool is_option) :
  m_value(def),
  m_default(def),
  m_name(name),
  m_type(type),
  m_description(desc),
  m_is_option(is_option)
  {
  }

  Property::~Property()
  {
  }

  void Property::configure_option ( XmlNode& node )
  {
    this->configure(node); // update the value

    // call all process functors
    BOOST_FOREACH( Property::Trigger_t& process, m_triggers )
        process();
  }


  void Property::change_value ( const boost::any& value )
  {
    m_value = value; // update the value

    // call all process functors
    BOOST_FOREACH( Property::Trigger_t& process, m_triggers )
      process();
  };

  template<>
  Common_API const char * Property::type_to_str<bool>() const { return "bool"; }

  template<>
  Common_API const char * Property::type_to_str<int>() const { return "integer"; };

  template<>
  Common_API const char * Property::type_to_str<CF::Uint>() const { return "unsigned"; }

  template<>
  Common_API const char * Property::type_to_str<CF::Real>() const { return "real"; }

  template<>
  Common_API const char * Property::type_to_str<std::string>() const { return "string"; }

  template<>
  Common_API const char * Property::type_to_str<boost::filesystem::path>() const { return "file"; }

  template<>
  Common_API const char * Property::type_to_str<URI>() const { return "uri"; }

  void Property::mark_basic()
  {
    if(!has_tag("basic"))
      add_tag("basic");
  }

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
