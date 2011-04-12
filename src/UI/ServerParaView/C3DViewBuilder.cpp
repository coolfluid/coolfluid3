
#include <QString>

#include "Common/CBuilder.hpp"
#include "Common/Signal.hpp"

#include "Common/XML/SignalOptions.hpp"

//#include "Solver/LibSolver.hpp"
#include "UI/ServerParaView/LibServerParaView.hpp"

#include "UI/ServerParaView/C3DViewBuilder.hpp"


////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ServerParaView {

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder < C3DViewBuilder, Component, LibServerParaView > C3DViewBuilder_Builder;

////////////////////////////////////////////////////////////////////////////////

C3DViewBuilder::C3DViewBuilder(const std::string & name) :
   Component(name)
{
  // signals
  regist_signal("create_3dview", "Creates a 3D View", "New 3D View")->
      signal->connect( boost::bind(&C3DViewBuilder::signal_create_3dview, this, _1) );

  // hide some signals from the GUI
  signal("create_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;

  // signatures
  signal("create_3dview")->signature->connect( boost::bind(&C3DViewBuilder::signature_create_3dview, this, _1) );
}

////////////////////////////////////////////////////////////////////////////////

void C3DViewBuilder::signal_create_3dview(SignalArgs &args)
{
  SignalOptions options( args );

  std::string name = options.option<std::string>("3DView name");
  URI parent = options.option<URI>("Parent");

  // some checks
  if(name.empty())
    throw BadValue(FromHere(), "The 3DView name is empty.");

  if(parent.empty())
    throw BadValue(FromHere(), "The parent is empty.");

  if(parent.scheme() != URI::Scheme::CPATH)
    throw InvalidURI(FromHere(), "The parent scheme is not CPATH");

  // create and add the component
  Component::Ptr parent_comp = Core::instance().root()->access_component_ptr(parent);
  C3DView::Ptr view(new C3DView(name));
  view->setPort("8080");
  parent_comp->add_component( view );
  view->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void C3DViewBuilder::signature_create_3dview(SignalArgs &args)
{
  SignalFrame& options = args.map( Protocol::Tags::key_options() );

  options.set_option("3DView name", std::string(), "Name for the new 3DView");
  options.set_option("Parent", Core::instance().root()->full_path(), "Parent of the new component");
}

////////////////////////////////////////////////////////////////////////////////

} // ServerParaView
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////
