// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

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
