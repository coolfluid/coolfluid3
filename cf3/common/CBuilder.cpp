// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include "common/CBuilder.hpp"

#include "common/Signal.hpp"
#include "common/LibCommon.hpp"

namespace cf3 {
namespace common {

RegistTypeInfo<CBuilder,LibCommon> CBuilder_TypeRegistration();

////////////////////////////////////////////////////////////////////////////////

CBuilder::CBuilder ( const std::string& name) : Component ( name )
{
  regist_signal( "create_component" )
    ->connect( boost::bind( &CBuilder::signal_create_component, this, _1 ) )
    ->description("builds a component")
    ->pretty_name("Build component");

  signal("create_component")->
      signature(boost::bind(&CBuilder::signature_signal_create_component, this, _1));

}

CBuilder::~CBuilder() {}

////////////////////////////////////////////////////////////////////////////////

void CBuilder::signal_create_component ( SignalArgs& args )
{
  XML::SignalFrame params = args.map( XML::Protocol::Tags::key_options() );

  URI path ( params.get_option<URI>("path") );

  Component::Ptr comp = build ( path.name() );
  Component::Ptr parent = access_component_ptr_checked( path.base_path() );
  parent->add_component( comp );
}

////////////////////////////////////////////////////////////////////////////////

void CBuilder::signature_signal_create_component ( SignalArgs& args )
{
  XML::SignalFrame p = args.map( XML::Protocol::Tags::key_options() );

  p.set_option<URI>("path", URI("cpath:"), "Full path for the created component" );
}

////////////////////////////////////////////////////////////////////////////////////////////

std::string CBuilder::extract_namespace (const std::string& builder_name)
{
  using namespace boost::algorithm;

  return std::string( builder_name.begin(), find_last(builder_name,".").begin() );
}

////////////////////////////////////////////////////////////////////////////////////////////

std::string CBuilder::extract_reduced_name (const std::string& builder_name)
{
  using namespace boost::algorithm;

  return std::string( find_last(builder_name,".").end(), builder_name.end() );
}

////////////////////////////////////////////////////////////////////////////////////////////

std::string CBuilder::extract_library_name (const std::string& builder_name)
{
  return CLibraries::namespace_to_libname( extract_namespace( builder_name ) );
}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
