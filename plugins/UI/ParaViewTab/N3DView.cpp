// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// header
#include "common/Signal.hpp"
#include "common/XML/Protocol.hpp"

#include "UI/UICommon/ComponentNames.hpp"
#include "UI/Core/TreeThread.hpp"
#include "UI/Graphics/TabBuilder.hpp"
#include "UI/ParaViewTab/Widget3D.hpp"
#include "UI/ParaViewTab/N3DView.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::graphics;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
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

  regist_signal( "go_to_tab" )
      ->connect( boost::bind( &N3DView::go_to_tab, this, _1 ) )
      ->description("Activates the tab")
      ->pretty_name("Switch to tab");

  m_local_signals << "go_to_tab";

}

//////////////////////////////////////////////////////////////////////////////

N3DView::~N3DView()
{

}

//////////////////////////////////////////////////////////////////////////////

void N3DView::about_to_be_removed()
{
  TabBuilder::instance()->queue_tab( as_ptr<CNode>() );
}

//////////////////////////////////////////////////////////////////////////////

void N3DView::go_to_tab( SignalArgs& node )
{
  TabBuilder::instance()->show_tab( as_ptr<CNode>() );
}

//////////////////////////////////////////////////////////////////////////////

void N3DView::reload_client_view(){

}


//////////////////////////////////////////////////////////////////////////////

void N3DView::setup_finished()
{
  TabBuilder::instance()->widget<Widget3D>( as_ptr<CNode>() );
}

//////////////////////////////////////////////////////////////////////////////

QString N3DView::tool_tip() const
{
  return component_type();
}

void N3DView::launch_pvserver( SignalArgs& node ){
  SignalFrame& options = node.map( Protocol::Tags::key_options() );

  std::vector<std::string> data = options.get_array<std::string>("infoServer");

  std::string host = data[0];
  std::string port = data[1];

  TabBuilder::instance()->widget<Widget3D>(as_ptr<CNode>())
      ->connect_to_server(host.c_str(), port.c_str());

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

  TabBuilder::instance()->widget<Widget3D>(as_ptr<CNode>())
      ->load_paths(path_list, name_list);

}

////////////////////////////////////////////////////////////////////////////////


} // Core
} // UI
} // CF
