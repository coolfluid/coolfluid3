// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

//Qt header
#include <QProcess>
#include <QHostInfo>
#include <QFileInfo>

// header
#include "common/FindComponents.hpp"
#include "common/Log.hpp"
#include "common/EventHandler.hpp"
#include "common/Builder.hpp"
#include "common/Signal.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionT.hpp"
#include "common/PropertyList.hpp"
#include "common/XML/SignalOptions.hpp"

#include "mesh/MeshWriter.hpp"
#include "mesh/Field.hpp"

//#include "ui/Server/ServerRoot.hpp"
#include "ui/ParaView/LibParaView.hpp"
#include "ui/ParaView/C3DView.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace ParaView {

  /////////////////////////////////////////////////////////////////////////////////

  ComponentBuilder < C3DView, Component, LibParaView> C3DView_Builder;

  /////////////////////////////////////////////////////////////////////////////////



C3DView::C3DView(const std::string& name) :
    Component(name)
{

  // options

  options().add( "mesh", m_mesh )
      .description("Mesh to visualize with given refresh rate")
      .pretty_name("Mesh")
      .mark_basic();

  m_filename = "solution_field.vtk";
  options().add("filename", m_filename )
      .description("File name to dumpmesh in VTK format")
      .pretty_name("File Name")
      .link_to(&m_filename);

  m_refresh_rate = 1;
  options().add("refresh_rate", m_refresh_rate )
      .description("Number of iterations between refreshing the mesh / solution")
      .pretty_name("Refresh Rate")
      .mark_basic()
      .link_to(&m_refresh_rate);

  m_port = 8080;
  options().add("paraview_server_port", m_port )
      .description("Port used on paraview server launch")
      .pretty_name("Server Port")
      .mark_basic()
      .link_to(&m_port);

  // signals

  regist_signal( "launch_pvserver" )
      .description("Launch Paraview Server")
      .pretty_name("Launch Server")
      .connect( boost::bind( &C3DView::launch_pvserver, this, _1));

  regist_signal( "iteration_done" )
      .description("iteration done")
      .pretty_name("iteration done")
      .connect( boost::bind( &C3DView::signal_iteration_done, this, _1));

  regist_signal( "send_server_info_to_client" )
      .description("Load last dumped file")
      .pretty_name("Get file info")
      .connect( boost::bind( &C3DView::send_server_info_to_client, this, _1));

  // hide some signals from the GUI
  signal("create_component")->hidden(true);
  signal("iteration_done")->hidden(true);

  // these signals are read-only
  signal("launch_pvserver")->read_only(true);

  Core::instance().event_handler().connect_to_event( "iteration_done",
                                                     this,
                                                     &C3DView::signal_iteration_done );

  boost::shared_ptr<mesh::MeshWriter> meshwriter =
      build_component_abstract_type<mesh::MeshWriter>("cf3.mesh.VTKLegacy.Writer","writer");
  add_component(meshwriter);

}

C3DView::~C3DView() {}

void C3DView::launch_pvserver( SignalArgs & args )
{
  if( PE::Comm::instance().rank() == 0 )
  {
    m_pvserver = new QProcess();

    m_pvserver->setProcessChannelMode(QProcess::ForwardedChannels);

    connect(m_pvserver, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readyReadStandardOutput()));
    connect(m_pvserver, SIGNAL(readyReadStandardError()),
            this, SLOT(readyReadStandardError()));

    //set the paraview server port
    QString portCommand = "-sp=";
    portCommand += QString::number( options()["paraview_server_port"].value<Uint>() );

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

    /// @todo [QG] change this to send 1 string and 1 Uint instead of an array
    std::vector<std::string> data(2);

    data[0] = QHostInfo::localHostName().toStdString();
    data[1] = QString::number(m_port).toStdString();

    XmlNode node = options.set_array("infoServer", data, " ; ");
  }
}

void C3DView::signal_iteration_done( SignalArgs & args )
{
  if( PE::Comm::instance().rank() == 0 )
  {
    static Uint curr_iteration = 0;
    SignalOptions opt(args);
    SignalFrame frame("file_dumped", uri(), uri());
    SignalOptions options(frame);

//    if (m_mesh.expired())
    {

      mesh::Mesh& mesh = find_component_recursively<mesh::Mesh>( Core::instance().root() );
      URI mesh_path = mesh.uri();
      properties().set("mesh", mesh_path);
    }
    //  throw SetupError( FromHere(), "Mesh option is not configured");


    //  Uint curr_iteration = opt.option<Uint>("iteration");

    if( curr_iteration == 1 || ( curr_iteration % options["refresh_rate"].value<Uint>() ) == 0 )
    {
      Handle<mesh::MeshWriter> writer = get_child("writer")->handle<mesh::MeshWriter>();


      std::vector<URI> fields;
      boost_foreach(const Field& field, find_components_recursively<Field>(*m_mesh.get()))
          fields.push_back(field.uri());

      writer->options()["fields"].change_value( fields );

      writer->write_from_to( *m_mesh.get(), m_filename);

      std::vector<std::string> data(2);

      data[0] =  QFileInfo( options["filename"].value<std::string>().c_str() )
          .absoluteFilePath().toStdString() ;
      data[1] = QFileInfo( options["filename"].value<std::string>().c_str())
          .fileName().section('.',0,0).toStdString();

      options.add("pathinfo", data);

      //    Server::ServerRoot::instance().core()->sendSignal( *frame.xml_doc.get() );
    }

    curr_iteration++;
  }
}

void C3DView::send_server_info_to_client( SignalArgs & args )
{
  if( PE::Comm::instance().rank() == 0 )
  {
    SignalFrame reply = args.create_reply( uri() );
    SignalFrame& options = reply.map( Protocol::Tags::key_options() );

    std::vector<std::string> data(2);

    data[0] =  options.get_option<std::string>("paraview_server_port");
    data[1] = QFileInfo( options.get_option<std::string>("paraview_server_port") .c_str()).fileName().section('.',0,0).toStdString();

    XmlNode node = options.set_array("pathinfo", data, " ; ");
  }
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
} // ui
} // CF
