// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

//Qt header
#include <QProcess>
#include <QHostInfo>
#include <QFileInfo>

// header
#include "Common/Log.hpp"
#include "Common/EventHandler.hpp"
#include "Common/CBuilder.hpp"
#include "Common/Signal.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CField.hpp"

#include "UI/Server/ServerRoot.hpp"
#include "UI/ParaView/LibParaView.hpp"
#include "UI/ParaView/C3DView.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;

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

  // options

  m_options.add_option( OptionComponent<Mesh::CMesh>::create("mesh", &m_mesh))
      ->set_description("Mesh to visualize with given refresh rate")
      ->set_pretty_name("Mesh")
      ->mark_basic();

  m_filename = "solution_field.vtk";
  m_options.add_option< OptionT<std::string> >("filename", m_filename )
      ->set_description("File name to dumpmesh in VTK format")
      ->set_pretty_name("File Name")
      ->link_to(&m_filename);

  m_refresh_rate = 1;
  m_options.add_option< OptionT<Uint> >("refresh_rate", m_refresh_rate )
      ->set_description("Number of iterations between refreshing the mesh / solution")
      ->set_pretty_name("Refresh Rate")
      ->mark_basic()
      ->link_to(&m_refresh_rate);

  m_port = 8080;
  m_options.add_option< OptionT<Uint> >("paraview_server_port", m_port )
      ->set_description("Port used on paraview server launch")
      ->set_pretty_name("Server Port")
      ->mark_basic()
      ->link_to(&m_port);

  // signals

  regist_signal("launch_pvserver", "Launch Paraview Server", "Launch Server")->
      signal->connect( boost::bind( &C3DView::launch_pvserver, this, _1));

  regist_signal("iteration_done", "iteration done", "iteration done")->
      signal->connect( boost::bind( &C3DView::signal_iteration_done, this, _1));

  regist_signal("send_server_info_to_client", "Load last dumped file", "Get file info")->
      signal->connect( boost::bind( &C3DView::send_server_info_to_client, this, _1));

  // hide some signals from the GUI
  signal("create_component")->is_hidden = true;
  signal("iteration_done")->is_hidden = true;

  // these signals are read-only
  signal("launch_pvserver")->is_read_only = true;


  // regist action to event "iteration_done"
  m_connect_iteration_done =
      Core::instance().event_handler().connect_to_event( "iteration_done", this, &C3DView::signal_iteration_done );

  cf_assert( m_connect_iteration_done.connected() );

  Mesh::CMeshWriter::Ptr meshwriter = build_component_abstract_type<Mesh::CMeshWriter>("CF.Mesh.VTKLegacy.CWriter","writer");
  add_component(meshwriter);

}

C3DView::~C3DView()
{
  // disconnet action to event "iteration_done"
  m_connect_iteration_done.disconnect();
}

void C3DView::launch_pvserver( SignalArgs & args ){

  m_pvserver = new QProcess();

  m_pvserver->setProcessChannelMode(QProcess::ForwardedChannels);

  connect(m_pvserver, SIGNAL(readyReadStandardOutput()),
          this, SLOT(readyReadStandardOutput()));
  connect(m_pvserver, SIGNAL(readyReadStandardError()),
          this, SLOT(readyReadStandardError()));

  //set the paraview server port
  QString portCommand = "-sp=";
  portCommand += QString::number( m_options["paraview_server_port"].value<Uint>() );

  //Use custom server.
  m_pvserver->start("pvserver", QStringList() << portCommand);
//  m_pvserver->start("/nobackup/st/wertz/./pvserver", QStringList() << portCommand);
  /** options **/
  //add --use-offscreen-rendering flag for offscreen rendering,
  //paraview server must be compiled with OSMesa,
  //if not, openGL rendering will be used
  //(no notification will be send to tell that paraview was not compiledwith mesa)
  //Needed in a non X environment

  //add -rc for the remote connection if support has been added (V4 normaly)

  //add openMPI command for MPI support



  SignalFrame reply = args.create_reply( uri() );
  SignalFrame& options = reply.map( Protocol::Tags::key_options() );

  std::vector<std::string> data(2);

  data[0] = QHostInfo::localHostName().toStdString();
  data[1] = QString::number(m_port).toStdString();

  XmlNode node = options.set_array("infoServer", data, " ; ");

}

void C3DView::signal_iteration_done( SignalArgs & args )
{
  static Uint curr_iteration = 0;
  SignalOptions opt(args);
  SignalFrame frame("file_dumped", uri(), uri());
  SignalOptions options(frame);

  if (m_mesh.expired())
  {

    Mesh::CMesh& mesh = find_component_recursively<Mesh::CMesh>( Core::instance().root() );
    URI mesh_path = mesh.uri();
    configure_option("mesh", mesh_path );
  }
//  throw SetupError( FromHere(), "Mesh option is not configured");


//  Uint curr_iteration = opt.option<Uint>("iteration");

  if( curr_iteration == 1 || ( curr_iteration % m_options["refresh_rate"].value<Uint>() ) == 0 )
  {
    Mesh::CMeshWriter& writer = get_child("writer").as_type<Mesh::CMeshWriter>();


    std::vector<URI> fields;
    boost_foreach(const CField& field, find_components_recursively<CField>(*m_mesh.lock()))
      fields.push_back(field.uri());

    writer.configure_option("fields",fields);

    writer.write_from_to( *m_mesh.lock(), m_filename);

    std::vector<std::string> data(2);

    data[0] =  QFileInfo( m_options["filename"].value<std::string>().c_str() )
        .absoluteFilePath().toStdString() ;
    data[1] = QFileInfo( m_options["filename"].value<std::string>().c_str())
        .fileName().section('.',0,0).toStdString();

    options.add_option< OptionArrayT<std::string> >("pathinfo", data);

    Server::ServerRoot::core()->sendSignal( *frame.xml_doc.get() );
  }

  curr_iteration++;

}

void C3DView::send_server_info_to_client( SignalArgs & args ){
  SignalFrame reply = args.create_reply( uri() );
  SignalFrame& options = reply.map( Protocol::Tags::key_options() );

  std::vector<std::string> data(2);

  data[0] =  m_options["paraview_server_port"].value<std::string>() ;
  data[1] = QFileInfo( m_options["paraview_server_port"].value<std::string>() .c_str()).fileName().section('.',0,0).toStdString();

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
