// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Common/Signal.hpp"

namespace CF {
namespace Common {

RegistTypeInfo<CBuilder> CBuilder_TypeRegistration();

////////////////////////////////////////////////////////////////////////////////

CBuilder::CBuilder ( const std::string& name) : Component ( name )
{
  regist_signal ( "build_component" , "builds a component", "Build component" )->
      signal->connect ( boost::bind ( &CBuilder::signal_build_component, this, _1 ) );

  signal("build_component")->
      signature->connect(boost::bind(&CBuilder::signature_signal_build_component, this, _1));

}

CBuilder::~CBuilder() {}

////////////////////////////////////////////////////////////////////////////////

void CBuilder::signal_build_component ( SignalArgs& args )
{
  XML::SignalFrame params = args.map( XML::Protocol::Tags::key_options() );

  Component::Ptr comp = build ( params.get_option<std::string>("Component name") );
  URI parent_path ( params.get_option<URI>("Parent component") );
  Component::Ptr parent = access_component_ptr_checked( parent_path );
  parent->add_component( comp );
}

////////////////////////////////////////////////////////////////////////////////

void CBuilder::signature_signal_build_component ( SignalArgs& args )
{
  XML::SignalFrame p = args.map( XML::Protocol::Tags::key_options() );

  p.set_option<std::string>("Component name", std::string(), "Name for created component" );
  p.set_option<URI>("Parent component", URI(), "Path to component where place the newly built component");
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
