// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// header
#include "Common/Signal.hpp"
#include "Common/XML/Protocol.hpp"

#include "UI/UICommon/ComponentNames.hpp"
#include "UI/Core/TreeThread.hpp"
#include "UI/Graphics/TabBuilder.hpp"
#include "UI/ParaViewTab/Widget3D.hpp"
#include "UI/ParaViewTab/N3DView.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::UI::Graphics;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ParaViewTab {

////////////////////////////////////////////////////////////////////////////////


N3DView::N3DView(const std::string & name) :
    CNode( name, "N3DView", CNode::STANDARD_NODE )
{
  regist_signal( "launch_pvserver" )
      ->description("Launch Paraview Server")
      ->pretty_name("Launch Server")
      ->connect( boost::bind( &N3DView::launch_pvserver, this, _1));

  regist_signal( "file_dumped" )
      ->description("Load last dumped file")
      ->pretty_name("Get file info")
      ->connect( boost::bind( &N3DView::send_server_info_to_client, this, _1));
}

//////////////////////////////////////////////////////////////////////////////

N3DView::~N3DView()
{

}

//////////////////////////////////////////////////////////////////////////////

void N3DView::aboutToBeRemoved()
{
  TabBuilder::instance()->queueTab( as_ptr<CNode>() );
}

//////////////////////////////////////////////////////////////////////////////

void N3DView::reload_client_view(){

}


//////////////////////////////////////////////////////////////////////////////

void N3DView::setUpFinished()
{
  TabBuilder::instance()->getWidget<Widget3D>( as_ptr<CNode>() );
}

//////////////////////////////////////////////////////////////////////////////

QString N3DView::toolTip() const
{
  return componentType();
}

void N3DView::launch_pvserver( SignalArgs& node ){
  SignalFrame& options = node.map( Protocol::Tags::key_options() );

  std::vector<std::string> data = options.get_array<std::string>("infoServer");

  std::string host = data[0];
  std::string port = data[1];

  TabBuilder::instance()->getWidget<Widget3D>(as_ptr<CNode>())
      ->connectToServer(host.c_str(), port.c_str());

}

//////////////////////////////////////////////////////////////////////////////

void N3DView::send_server_info_to_client( SignalArgs& node ){
  SignalFrame& options = node.map( Protocol::Tags::key_options() );

  std::vector<std::string> data = options.get_array<std::string>("pathinfo");

  std::string path = data[0];
  std::string name = data[1];

  std::vector<QString> path_list(1);
  path_list[0] = QString(path.c_str());

  std::vector<QString> name_list(1);
  name_list[0] = QString(name.c_str());

  TabBuilder::instance()->getWidget<Widget3D>(as_ptr<CNode>())
      ->loadPaths(path_list, name_list);

}

////////////////////////////////////////////////////////////////////////////////


} // Core
} // UI
} // CF
