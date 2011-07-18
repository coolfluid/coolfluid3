// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// header
#include "Common/CBuilder.hpp"
#include "Common/Signal.hpp"
#include "Common/XML/SignalOptions.hpp"
#include "UI/ParaView/LibParaView.hpp"
#include "UI/ParaView/C3DViewBuilder.hpp"
#include "Common/StringConversion.hpp"


////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ParaView {

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder < C3DViewBuilder, Component, LibParaView > C3DViewBuilder_Builder;

////////////////////////////////////////////////////////////////////////////////

C3DViewBuilder::C3DViewBuilder(const std::string & name) :
   Component(name)
{
  // signals
  regist_signal( "create_3dview" )
      ->description("Creates a 3D View")
      ->pretty_name("New 3D View")
      ->connect( boost::bind(&C3DViewBuilder::signal_create_3dview, this, _1) )
      ->signature( boost::bind(&C3DViewBuilder::signature_create_3dview, this, _1) );

  // hide some signals from the GUI

  signal("create_component")->hidden(true);
  signal("delete_component")->hidden(true);
  signal("move_component")->hidden(true);
  signal("rename_component")->hidden(true);

}

////////////////////////////////////////////////////////////////////////////////

void C3DViewBuilder::signal_create_3dview(SignalArgs &args)
{
  SignalOptions options( args );

  std::string name = options.value<std::string>("3DView name");
  URI parent = options.value<URI>("Parent");

  // some checks
  if(name.empty())
    throw BadValue(FromHere(), "The 3DView name is empty.");

  if(parent.empty())
    throw BadValue(FromHere(), "The parent is empty.");

  if(parent.scheme() != URI::Scheme::CPATH)
    throw InvalidURI(FromHere(), "The parent scheme is not CPATH");

  // create and add the component
  Component::Ptr parent_comp = Core::instance().root().access_component_ptr(parent);
  C3DView::Ptr view(new C3DView(name));

  parent_comp->add_component( view );
  view->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void C3DViewBuilder::signature_create_3dview(SignalArgs &args)
{
  SignalFrame& options = args.map( Protocol::Tags::key_options() );

  options.set_option("3DView name", std::string(), "Name for the new 3DView");
  options.set_option("Parent", Core::instance().root().uri(), "Parent of the new component");
}


////////////////////////////////////////////////////////////////////////////////

} // ParaView
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////
