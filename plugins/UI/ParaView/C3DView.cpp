// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

//Qt header
#include <QProcess>
#include <QHostInfo>

// header
#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/Signal.hpp"
#include "UI/Server/ServerRoot.hpp"
#include "UI/ParaView/LibParaView.hpp"
#include "UI/ParaView/C3DView.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ParaView {

  /////////////////////////////////////////////////////////////////////////////////

  ComponentBuilder < C3DView, Component, LibParaView> C3DView_Builder;

  /////////////////////////////////////////////////////////////////////////////////



C3DView::C3DView(const std::string& name) :
    Component(name)
{
  regist_signal("launch_pvserver", "Launch Paraview Server", "Launch Server")->
      signal->connect( boost::bind( &C3DView::launch_pvserver, this, _1));

  regist_signal("iteration_done", "iteration done", "iteration done")->
      signal->connect( boost::bind( &C3DView::dump_file, this, _1));

  regist_signal("send_server_info_to_client", "Send file info to client", "Get file info")->
      signal->connect( boost::bind( &C3DView::send_server_info_to_client, this, _1));

  // hide some signals from the GUI
  signal("create_component")->is_hidden = true;
  signal("iteration_done")->is_hidden = true;
  //signal("send_server_info_to_client")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;

}

void C3DView::setPort(QString port){
  m_port = port;
}


C3DView::~C3DView()
{

}

void C3DView::launch_pvserver( SignalArgs & args ){

  m_pvserver = new QProcess();

  m_pvserver->setProcessChannelMode(QProcess::ForwardedChannels);

  connect(m_pvserver, SIGNAL(readyReadStandardOutput()),
          this, SLOT(readyReadStandardOutput()));
  connect(m_pvserver, SIGNAL(readyReadStandardError()),
          this, SLOT(readyReadStandardError()));

  if(m_port.isEmpty()){
    m_port = "11111"; //default paraview port
  }

  //set the paraview server port
  QString portCommand = "-sp=";
  portCommand += m_port;

  //Use custom server.
//  m_pvserver->start("pvserver", QStringList() << portCommand);
  m_pvserver->start("/nobackup/st/wertz/./pvserver", QStringList() << portCommand);

  SignalFrame reply = args.create_reply( full_path() );
  SignalFrame& options = reply.map( Protocol::Tags::key_options() );

  std::vector<std::string> data(2);

  data[0] = QHostInfo::localHostName().toStdString();//"127.0.0.1";
  data[1] = m_port.toStdString();

  XmlNode node = options.set_array("infoServer", data, " ; ");

}

void C3DView::dump_file( SignalArgs & args ){

}

void C3DView::send_server_info_to_client( SignalArgs & args ){
  SignalFrame reply = args.create_reply( full_path() );
  SignalFrame& options = reply.map( Protocol::Tags::key_options() );

  std::vector<std::string> data(4);

  data[0] = "/nobackup/st/wertz/frog/skeleton.vtk";
  data[1] = "/nobackup/st/wertz/frog/stomach.vtk";
  data[2] = "skeleton";
  data[3] = "stomach";

  XmlNode node = options.set_array("pathinfo", data, " ; ");

}

void C3DView::readyReadStandardOutput()
{
  CFinfo << m_pvserver->readAllStandardOutput().data() << CFflush;
}

void C3DView::readyReadStandardError()
{
  CFinfo << m_pvserver->readAllStandardError().data() << CFflush;
}


/////////////////////////////////////////////////////////////////////////////////

} // ParaView
} // UI
} // CF
