// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDebug>

#include "Common/Signal.hpp"

#include "Common/XML/Protocol.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/TreeThread.hpp"

#include "UI/Core/N3DView.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////////


N3DView::N3DView(const QString & name) :
    CNode( name, "N3DView", VIEW3D_NODE )
{
  regist_signal("launch_pvserver", "Launch Paraview Server", "Launch Server")->
      signal->connect( boost::bind( &N3DView::launch_pvserver, this, _1));

  regist_signal("send_server_info_to_client", "Send file info to client", "Get file info")->
      signal->connect( boost::bind( &N3DView::send_server_info_to_client, this, _1));
}

void N3DView::reload_client_view(){

}

QString N3DView::toolTip() const
{
  return getComponentType();
}

void N3DView::launch_pvserver( SignalArgs& node ){
  SignalFrame& options = node.map( Protocol::Tags::key_options() );

  std::vector<std::string> data = options.get_array<std::string>("infoServer");

  std::string host = data[0];
  std::string port = data[1];

  qDebug() << "Receiving server info";

  qDebug() << QString(host.c_str());
  qDebug() << QString(port.c_str());

  N3DViewNotifier::instance().notify_server_spec(QString(host.c_str()) ,QString(port.c_str()));

}

void N3DView::send_server_info_to_client( SignalArgs& node ){
  SignalFrame& options = node.map( Protocol::Tags::key_options() );

  std::vector<std::string> data = options.get_array<std::string>("pathinfo");

  std::string path = data[0];
  std::string path2 = data[1];
  std::string name = data[2];
  std::string name2 = data[3];

  qDebug() << "Receiving Paths";

  qDebug() << QString(name.c_str());
  qDebug() << QString(name2.c_str());
  qDebug() << QString(path.c_str());
  qDebug() << QString(path2.c_str());

  std::vector<QString> path_list(2);
  path_list[0] = QString(path.c_str());
  path_list[1] = QString(path2.c_str());

  std::vector<QString> name_list(2);
  name_list[0] = QString(name.c_str());
  name_list[1] = QString(name2.c_str());

  N3DViewNotifier::instance().notify_path_spec(path_list,name_list);

}

////////////////////////////////////////////////////////////////////////////////


} // Core
} // UI
} // CF
