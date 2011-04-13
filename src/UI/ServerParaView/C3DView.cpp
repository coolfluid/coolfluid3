#include "Common/CBuilder.hpp"
#include "Common/Signal.hpp"

#include <QHostInfo>

#include "UI/Server/ServerRoot.hpp"

#include "UI/ServerParaView/LibServerParaView.hpp"
#include "UI/ServerParaView/C3DView.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ServerParaView {

  /////////////////////////////////////////////////////////////////////////////////

  ComponentBuilder < C3DView, Component, LibServerParaView> C3DView_Builder;

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

/*
void C3DView::launch_pvserver(QString port,QString machinesFilePath,QString numberOfProcess){

  pvserver = new QProcess();

  pvserver->setProcessChannelMode(QProcess::ForwardedChannels);

  QString portCommand = "-sp=";
  portCommand += port;

  //Use custom server.
  pvserver->start("/nobackup/st/wertz/./pvserver", QStringList() << portCommand);

  //If Paraview is well installed.
  //pvserver->start("pvserver", QStringList() << portCommand);
*/
  /*
   //launch server with mpi
   QStringList args();
   QString numProc = "-np ";
   numProc += numberOfProcess;
   QString machinePath = "-hostfile ";
   machinePath += machinesFilePath;
   QString process = "pvserver";
   QString portArg = "-sp=";
   portArg += port;
   args << numProc;
   args << machinePath;
   args << process;
   args << portArg;
   //pvserver->start("mpirun", args);
   */

//}


void C3DView::launch_pvserver( SignalArgs & args ){

  pvserver = new QProcess();

  pvserver->setProcessChannelMode(QProcess::ForwardedChannels);

  if(m_port.isEmpty()){
    m_port = "11111"; //default paraview port
  }

  //set the paraview server port
  QString portCommand = "-sp=";
  portCommand += m_port;

  //Use custom server.
  pvserver->start("pvserver", QStringList() << portCommand);

  SignalFrame reply = args.create_reply( full_path() );
  SignalFrame& options = reply.map( Protocol::Tags::key_options() );

  std::vector<std::string> data(2);

  data[0] = QHostInfo::localHostName().toStdString();//"127.0.0.1";
  data[1] = m_port.toStdString();

  XmlNode node = options.set_array("infoServer", data, " ; ");

  //send_server_info_to_client();

}

void C3DView::dump_file( SignalArgs & args ){
/*
  SignalFrame reply = args.create_reply( full_path() );
  SignalFrame& options = reply.map( Protocol::Tags::key_options() );

  std::vector<std::string> data(3);

  data[0] = "127.0.0.1";
  data[1] = "8080";
  data[2] = "/nobackup/st/wertz/frog/skeleton.vtk";

  XmlNode node = options.set_array("infoServer", data, " ; ");
*/
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

/////////////////////////////////////////////////////////////////////////////////

} // ServerParaView
} // UI
} // CF
