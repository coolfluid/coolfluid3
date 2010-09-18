// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"
#include "Common/ConfigObject.hpp"
#include "Common/XmlHelpers.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  void ConfigObject::configure ( XmlNode& node )
  {
    XmlParams pn ( node );

    if ( pn.option_map == 0 )
      throw  Common::XmlError( FromHere(), "ConfigObject received  XML without a \'" + std::string(XmlParams::tag_node_valuemap()) + "\' node" );

    // get the list of options
    PropertyList::PropertyStorage_t& options = m_property_list.m_properties;

    // loop on the param nodes
    for (XmlNode* itr =  pn.option_map->first_node(); itr; itr = itr->next_sibling() )
    {
      // search for the attribute with key
      XmlAttr* att = itr->first_attribute( XmlParams::tag_attr_key() );
      if ( att )
      {
        PropertyList::PropertyStorage_t::iterator opt = options.find( att->value() );
        if (opt != options.end() && opt->second->is_option())
          opt->second->as_option().configure_option(*itr);
      }
    }
  }

  const Property & ConfigObject::property( const std::string& optname ) const
  {
    return m_property_list.getProperty(optname);
  }

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
