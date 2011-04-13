// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// Qt headers
//#include <QHBoxLayout>
#include <QLabel>
#include <QDialog>
#include <QIntValidator>
//#include <QMap>
//#include <QList>
#include <QFileDialog>
#include <QColorDialog>
#include <QDebug>

// ParaViewTab header
#include "pqDataRepresentation.h"
#include "vtkSmartPointer.h"
#include "vtkPolyData.h"
#include "vtkPNGWriter.h"
#include "vtkWindowToImageFilter.h"
#include "pqSettings.h"
#include <vtksys/ios/sstream>
#include "pqStandardColorLinkAdaptor.h"
#include "vtkSMPVRepresentationProxy.h"
#include "pqSMAdaptor.h"
#include "vtkSMProperty.h"
#include "pqStandardViewModules.h"
#include "pqPQLookupTableManager.h"
#include "vtkSMPropertyHelper.h"
#include "vtkGlyph3DRepresentation.h"

// header
#include "UI/Core/NLog.hpp"

#include "UI/ParaViewTab/N3DView.hpp"

#include "UI/ParaViewTab/Widget3D.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF::UI::Core;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ParaViewTab {

////////////////////////////////////////////////////////////////////////////////

Widget3D::Widget3D(QWidget *parent) :
  QWidget(parent)
{
  //ParaViewTab instance and builder
  // automatically make a server connection
  m_core = pqApplicationCore::instance();
  m_core->disableOutputWindow();

  /*
        pqSettings* settings = m_core->settings();


        settings->beginGroup("renderModule");

//        settings->setValue("ImageReductionFactor", 2000); // reduction of pixels while manipulating  1 no reduction
//        settings->setValue("StillRenderImageReductionFactor", 1000);  // reduction of pixels after rendering  1 no reduction

        settings->setValue("CompressionEnabled",1);
        settings->setValue("CompressorType",1); //COMPRESSOR_SQUIRT
        vtkstd::ostringstream os;
        os << "vtkSquirtCompressor 0 "
           << 5;//16 : 16bit
        settings->setValue("CompressorConfig",os.str().c_str());
     settings->endGroup();
   */

  m_object_builder = m_core->getObjectBuilder();

  // Register ParaViewTab interfaces.
  m_plugin_manager = m_core->getPluginManager();

  // adds support for standard ParaViewTab views.
  m_plugin_manager->addInterface(new pqStandardViewModules(m_plugin_manager));

  // set the lookuptable to be able using legend.
  m_core->setLookupTableManager(new pqPQLookupTableManager(this));

  //define timeout of connection process
  //        pqProgressManager * m_progress_manager = m_core->getProgressManager();
  //        m_progress_manager->setEnableProgress(false);

  //main layout
  m_layout_v = new QVBoxLayout();
  this->setLayout(m_layout_v);

  //Server advanced options
  //pqGlobalRenderViewOptions * opt = new pqGlobalRenderViewOptions(this);
  //pqRenderViewOptions * opt = new pqRenderViewOptions(this);
  //pqApplicationOptionsDialog * opt = new pqApplicationOptionsDialog(this);

  //qDebug() << opt->getPageList();
  //opt->setPage(opt->getPageList().at(3));
  //m_layout_v->addWidget(opt);

  //horisontal layout view + server (remove from widget) and view options
  m_layout_h = new QHBoxLayout();

  //vertical layout containing server and view options
  m_layout_option = new QVBoxLayout();

  //Options buttons
  m_connect_to_server_button = new QPushButton(QIcon(":/paraview_icons/pqConnect24.png"),"Connect",this);

  m_load_file = new QPushButton(QIcon(":/paraview_icons/pqOpen24.png"),"Load File",this);

  m_load_file->setVisible(false);

  m_set_rotation_center = new QPushButton(QIcon(":/paraview_icons/pqResetCenter24.png"),"Fix Center Rotation",this);

  m_screen_shot = new QPushButton(QIcon(":/paraview_icons/pqCaptureScreenshot24.png"),"Screen Shot",this);

  m_reset_camera = new QPushButton(QIcon(":/paraview_icons/pqResetCamera24.png"),"Reset Camera",this);

  m_show_color_palette = new QPushButton(QIcon(":/paraview_icons/pqScalarBar24.png"),"Show Color Palette",this);

  //        m_reload = new QPushButton(QIcon(":/paraview_icons/ShowCenterButton.png"),"Reload Mesh",this);
  //        m_reload->setVisible(false);

  //Combo box that handle styles
  m_mesh_style = new pqDisplayRepresentationWidget(this);

  //Combo box of predefined camera orientation
  m_preDefined_rotation = new QComboBox(this);
  m_preDefined_rotation->addItem(QIcon(":/paraview_icons/pqXPlus16.png"),"+X",0);
  m_preDefined_rotation->addItem(QIcon(":/paraview_icons/pqYPlus16.png"),"+Y",1);
  m_preDefined_rotation->addItem(QIcon(":/paraview_icons/pqZPlus16.png"),"+Z",2);
  m_preDefined_rotation->addItem(QIcon(":/paraview_icons/pqXMinus16.png"),"-X",3);
  m_preDefined_rotation->addItem(QIcon(":/paraview_icons/pqYMinus16.png"),"-Y",4);
  m_preDefined_rotation->addItem(QIcon(":/paraview_icons/pqZMinus16.png"),"-Z",5);
  m_preDefined_rotation->setEditable(false);

  // the color selector and legend
  m_dataSet_selector = new pqDisplayColorWidget(this);

  //create a builtin server to have axes shown
  m_server = m_object_builder->createServer(pqServerResource("builtin:"));
  //m_server->setHeartBeatTimeoutSetting(10);

  if(m_server){
    //create the builtin server view
    createView();
  }else{
    NLog::globalLog()->addError("Error while creating widget3d");
  }

  // Regions list
  m_actor_list = new QListWidget(this);

  // Set Solide Color button
  m_mesh_solid_color_set = new QPushButton("Set Color ...");

  //Opacity spinner
  m_spin_opacity = new QDoubleSpinBox(this);
  //m_spin_opacity->setObjectName("Opacity");
  m_spin_opacity->setMaximum(1);
  m_spin_opacity->setSingleStep(0.1);
  m_spin_opacity->setMinimum(0);
  m_spin_opacity->setEnabled(false);

  // representation initialisation
  representation = 0;

  //disposition
  m_server_options = new QGroupBox("Server Options",this);
  m_server_options->setMaximumWidth(200);
  m_camera_options = new QGroupBox("Camera Options",this);
  m_camera_options->setMaximumWidth(200);
  m_mesh_options = new QGroupBox("Mesh Options",this);
  m_mesh_options->setVisible(false);
  m_regions_box = new QGroupBox("Regions",this);
  m_regions_box->setMaximumWidth(200);
  m_regions_box->setVisible(false);

  m_layout_server_options = new QVBoxLayout();
  m_layout_camera_options = new QVBoxLayout();
  m_layout_mesh_options = new QHBoxLayout();
  m_layout_regions_box = new QVBoxLayout();

  m_server_options->setLayout(m_layout_server_options);
  m_camera_options->setLayout(m_layout_camera_options);
  m_mesh_options->setLayout(m_layout_mesh_options);
  m_regions_box->setLayout(m_layout_regions_box);

  m_layout_server_options->addWidget(this->m_connect_to_server_button);
  m_layout_server_options->addWidget(this->m_load_file);
  m_layout_mesh_options->addWidget(this->m_mesh_style);
  m_layout_camera_options->addWidget(this->m_set_rotation_center);
  m_layout_mesh_options->addWidget(this->m_dataSet_selector);
  m_layout_camera_options->addWidget(this->m_reset_camera);
  m_layout_camera_options->addWidget(this->m_screen_shot);
  m_layout_camera_options->addWidget(this->m_preDefined_rotation);
  m_layout_mesh_options->addWidget(this->m_show_color_palette);
  //m_layout_server_options->addWidget(this->m_reload);
  m_layout_mesh_options->addWidget(this->m_mesh_solid_color_set);
  m_layout_mesh_options->addWidget(this->m_spin_opacity);


  m_layout_option->addWidget(this->m_server_options);
  m_layout_option->addWidget(this->m_camera_options);
  m_layout_option->addWidget(this->m_regions_box);

  m_layout_h->addLayout(this->m_layout_option);

  m_layout_v->addLayout(m_layout_h);
  m_layout_v->addWidget(this->m_mesh_options);

  m_layout_regions_box->addWidget(this->m_actor_list);

  //connect
  connect(m_connect_to_server_button,SIGNAL(released()),this,SLOT(showConnectDialog()));
  connect(m_load_file,SIGNAL(released()),this,SLOT(showLoadFileDialog()));
  connect(m_set_rotation_center,SIGNAL(released()),this,SLOT(set_rotation_center()));
  connect(m_preDefined_rotation,SIGNAL(activated(int)),this,SLOT(set_rotation(int)));
  connect(m_screen_shot,SIGNAL(released()),this,SLOT(take_screen_shot()));
  connect(m_reset_camera,SIGNAL(released()),this,SLOT(reset_camera()));
  connect(m_show_color_palette,SIGNAL(released()),this,SLOT(show_color_editor()));
  //connect(m_reload,SIGNAL(released()),this,SLOT(reload()));
  connect(m_actor_list,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(show_hide_actor(QListWidgetItem*)));
  connect(m_actor_list,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(actor_changed(QListWidgetItem*)));
  connect(m_mesh_solid_color_set,SIGNAL(released()),this,SLOT(setColor()));
  //connect(m_spin_opacity,SIGNAL(valueChanged(double)),this,SLOT(opacityChange(double)));
  connect(m_dataSet_selector,SIGNAL(variableChanged(pqVariableType, const QString)),this,SLOT(update_solide_color_button_state(pqVariableType, const QString)));

  ////Connection Boost
  N3DViewNotifier::instance().notify_server_spec.connect(
        boost::bind(&Widget3D::connectToServer, this, _1, _2) );

  N3DViewNotifier::instance().notify_path_spec.connect(
        boost::bind(&Widget3D::loadPaths, this, _1, _2) );


}

void Widget3D::connectToServer(QString given_host,QString port)
{
  //pqServerResource configuration
  QString host = "cs://";//cs:// =>client-server    rc:// => remote server connection (not implemented yet by ParaViewTab)
  host += given_host;
  host += ":";
  host += port;

  qDebug() << host;

  if(m_server)
    //if any remote or builtin server, delete them to free ressources
    m_object_builder->removeServer(m_server);

  //create new remote server
  m_server = m_object_builder->createServer(pqServerResource(host));

  //if the server is remote then :
  if(m_server && m_server->isRemote()){
    //show user info
    NLog::globalLog()->addMessage("Connected to ParaViewTab server");

    //show the "load file" button
    m_load_file->setVisible(true);

    //change connect button to disconnect button
    this->m_connect_to_server_button->setText("Disconnect");
    this->m_connect_to_server_button->setIcon(QIcon(":/paraview_icons/pqDisconnect24.png"));
    disconnect(m_connect_to_server_button,SIGNAL(released()),this,SLOT(showConnectDialog()));
    connect(m_connect_to_server_button,SIGNAL(clicked()),this,SLOT(disconnectFromServer()));

    //create server view
    createView();
  }else{
    NLog::globalLog()->addError("Error while connecting to ParaViewTab server");
    m_server = m_object_builder->createServer(pqServerResource("builtin:"));
    createView();
  }

}

void Widget3D::openFile(QString file_path,QString file_name)
{
  std::vector<QString> paths;
  paths.push_back(file_path);
  std::vector<QString> names;
  names.push_back(file_name);

  loadPaths(paths,names);
  /*
        //if source exist, delete it to free ressources
        if(m_source)
            m_object_builder->destroy(m_source);

        //we save the path
        m_file_path = file_path;

        //Now that we have loaded a file we can reload it without giving any path
        m_reload->setVisible(true);

        //check if there is a server and if it is a remote one
        if(m_server && m_server->isRemote()){

          //detecting extention and launching correct reader
          QString extention = m_file_path.section('.',-1);

          //create reader on server side depending on file type
          if(!extention.compare("vtk")){ //vtk
              m_source = m_object_builder->createReader("sources", "LegacyVTKFileReader",QStringList(m_file_path), m_server);
          }
          if(!extention.compare("ex2")){ //ex2
              m_source = m_object_builder->createReader("sources", "ExodusIIReader",QStringList(m_file_path), m_server);
          }

          //if source has been created proprely
          if(m_source){

              //update the pipeline
              vtkSMSourceProxy::SafeDownCast(m_source->getProxy())->UpdatePipeline();

              //if source and server has been created proprely
              if(m_source && m_server){
                //show the render
                showRender();
                //show mesh options
                m_mesh_options->setVisible(true);
                m_regions_box->setVisible(true);
              }
          }else{
            m_mesh_options->setVisible(false);
            m_regions_box->setVisible(false);
            NLog::globalLog()->addError("Source of this file path don't exist.");
          }
      }else{
        m_mesh_options->setVisible(false);
        m_regions_box->setVisible(false);
        NLog::globalLog()->addError("Cannot load a file if no ParaViewTab server connection is set.");
      }
      */
}

void Widget3D::showRender()
{
  if(m_source){
    /*
            //create the filter
            //pqPipelineSource * m_filter = m_object_builder->createFilter("filters","DataSetTriangleFilter",m_source);

            QMap<QString, QList<pqOutputPort*> > map;

            map.insert("Input",m_source->getOutputPorts());

            //pqPipelineSource * m_filter = m_object_builder->createFilter("filters","DataSetTriangleFilter",m_source);
            //pqPipelineSource * m_filter = m_object_builder->createFilter("filters","DataSetTriangleFilter",map,m_server); // Any -> unstructured Grid
            //pqPipelineSource * m_filter = m_object_builder->createFilter("filters","DataSetSurfaceFilter",map,m_server); // Any -> vtkPolyData
            //pqPipelineSource * m_filter = m_object_builder->createFilter("filters","TriangleFilter",map,m_server); //need vtkPolyData
            //pqPipelineSource * m_filter = m_object_builder->createFilter("filters","TessellatorFilter",map,m_server);
            //pqPipelineSource * m_filter = m_object_builder->createFilter("filters","GeometryFilter",map,m_server); // vtkDataSet -> vtkPolyData

            QString numPort = "Number of source port : ";
            numPort += QString::number(m_source->getNumberOfOutputPorts());
            ClientNLog::globalLog()->addMessage(numPort);

            pqPipelineSource * m_filter = m_object_builder->createFilter("filters","DataSetSurfaceFilter",map,m_server);

            numPort = "Number of filter 1 port : ";
            numPort += QString::number(m_filter->getNumberOfOutputPorts());
            ClientNLog::globalLog()->addMessage(numPort);

            QMap<QString, QList<pqOutputPort*> > map2;

            map2.insert("Input",m_filter->getOutputPorts());

            pqPipelineSource * m_filter2 = m_object_builder->createFilter("filters","TriangleFilter",map2,m_server);

            numPort = "Number of filter 2 port : ";
            numPort += QString::number(m_filter2->getNumberOfOutputPorts());
            ClientNLog::globalLog()->addMessage(numPort);

*/
    //save the "final proxy" as m_input, then we can easly add filters without changing lots of code
    m_input = m_source;

    //create a data representation in server side, for the render window with the input
    m_object_builder->createDataRepresentation(m_input->getOutputPort(0), this->m_RenderView);

    //test multi
    QPointer<pqPipelineSource> m_source2 = m_object_builder->createReader("sources", "LegacyVTKFileReader",QStringList("/nobackup/st/wertz/frog/stomach.vtk"), m_server);
    vtkSMSourceProxy::SafeDownCast(m_source2->getProxy())->UpdatePipeline();
    m_object_builder->createDataRepresentation(m_source2->getOutputPort(0), this->m_RenderView);

    //set the color selector representation (after it will directly apply changes to this representation)
    this->m_dataSet_selector->setRepresentation(m_input->getRepresentation(m_RenderView));

    //set the style selector representation (after it will directly apply changes to this representation)
    this->m_mesh_style->setRepresentation(m_input->getRepresentation(m_RenderView));

    //zoom to object
    this->m_RenderView->resetCamera();

    //make sure we update the view
    this->m_RenderView->render();

  }else{
    NLog::globalLog()->addError("There is no file to render.");
  }
}

void Widget3D::createView(){
  if(m_server){
    // create a graphics window and put it in our main window
    this->m_RenderView = qobject_cast<pqRenderView*>(
          m_object_builder->createView(pqRenderView::renderViewType(), m_server));

    if(m_RenderView){
      //put the view in the 0 index so it is the first widget of the layout (avoid bugs)
      m_layout_h->insertWidget(0,this->m_RenderView->getWidget());
    }else{
      NLog::globalLog()->addError("Problem when creating a RenderView.");
    }
  }else{
    NLog::globalLog()->addError("Cannot create RenderView if no ParaView server connection is set.");
  }
}

void Widget3D::disconnectFromServer(){

  //remove server if any (will remove all object on the server)
  if(m_server)
    m_object_builder->removeServer(m_server);

  //remove view if any
  if(m_RenderView)
    m_object_builder->destroy(m_RenderView);

  //create a builtin server to have axes shown
  m_server = m_object_builder->createServer(pqServerResource("builtin:"));

  //empty Region list
  if(m_actor_list)
    m_actor_list->clear();

  //empty Source list
  if(!m_source_list.isEmpty())
    m_source_list.clear();

  if(m_server){
    //create the builtin server view
    createView();
  }

  //hide mesh options
  m_mesh_options->setVisible(false);
  m_regions_box->setVisible(false);

  //change disconnect button to connect button
  this->m_connect_to_server_button->setText("Connect");
  this->m_connect_to_server_button->setIcon(QIcon(":/paraview_icons/pqConnect24.png"));
  disconnect(m_connect_to_server_button,SIGNAL(clicked()),this,SLOT(disconnectFromServer()));
  connect(m_connect_to_server_button,SIGNAL(released()),this,SLOT(showConnectDialog()));

  //show user info
  NLog::globalLog()->addMessage("Disconnected from ParaViewTab server");

  //disconnected, cannot load or reload file while not connected
  m_load_file->setVisible(false);
  //m_reload->setVisible(false);

}

void Widget3D::showLoadFileDialog(){
  //the popup dialog box
  QPointer<QDialog> loadFileDialog = new QDialog(this);

  //adding Exit and Connect button
  QPointer<QPushButton> btn_load = new QPushButton("Load", loadFileDialog);
  QPointer<QPushButton> btn_exit = new QPushButton("Exit", loadFileDialog);

  // line edit
  m_Name_line = new QLineEdit("skeleton",this);
  m_Path_File_line = new QLineEdit("/nobackup/st/wertz/frog/skeleton.vtk",this);

  // labels
  QPointer<QLabel> path_label = new QLabel("File Path");
  QPointer<QLabel> name_label = new QLabel("Name");

  // popup layout
  QPointer<QVBoxLayout> vertical_popup_layout = new QVBoxLayout();
  QPointer<QHBoxLayout> horisontal_path_layout = new QHBoxLayout();
  QPointer<QHBoxLayout> horisontal_name_layout = new QHBoxLayout();
  QPointer<QHBoxLayout> horisontal_button_layout = new QHBoxLayout();

  horisontal_name_layout->addWidget(name_label);
  horisontal_name_layout->addWidget(m_Name_line);

  horisontal_path_layout->addWidget(path_label);
  horisontal_path_layout->addWidget(m_Path_File_line);

  horisontal_button_layout->addWidget(btn_load);
  horisontal_button_layout->addWidget(btn_exit);

  vertical_popup_layout->addLayout(horisontal_name_layout);
  vertical_popup_layout->addLayout(horisontal_path_layout);
  vertical_popup_layout->addLayout(horisontal_button_layout);

  loadFileDialog->setLayout(vertical_popup_layout);

  //connect them to theirs actions
  connect(btn_exit, SIGNAL(released()),loadFileDialog,SLOT(close()));
  connect(btn_load, SIGNAL(released()),this,SLOT(loadFile()));
  connect(btn_load, SIGNAL(released()),loadFileDialog,SLOT(close()));

  //set popup visiblem modal and 350 * 100 tall
  loadFileDialog->resize(350,100);
  loadFileDialog->setModal(true);
  loadFileDialog->show();
}

void Widget3D::showConnectDialog(){
  //the popup dialog box
  QPointer<QDialog> connectFileDialog = new QDialog(this);

  //adding Exit and Connect button
  QPointer<QPushButton> btn_connect = new QPushButton("Connect", connectFileDialog);
  QPointer<QPushButton> btn_exit = new QPushButton("Exit", connectFileDialog);

  // line edit
  m_port_line = new QLineEdit("8080",this);
  m_port_line->setValidator(new  QIntValidator(nullptr));
  m_host_line = new QLineEdit("10.120.0.24",this);

  // labels
  QPointer<QLabel> port_label = new QLabel("port");
  QPointer<QLabel> host_label = new QLabel("host");

  // popup layout
  QPointer<QVBoxLayout> vertical_popup_layout = new QVBoxLayout();
  QPointer<QHBoxLayout> horisontal_host_layout = new QHBoxLayout();
  QPointer<QHBoxLayout> horisontal_port_layout = new QHBoxLayout();
  QPointer<QHBoxLayout> horisontal_button_layout = new QHBoxLayout();

  horisontal_host_layout->addWidget(host_label);
  horisontal_host_layout->addWidget(m_host_line);

  horisontal_port_layout->addWidget(port_label);
  horisontal_port_layout->addWidget(m_port_line);

  horisontal_button_layout->addWidget(btn_connect);
  horisontal_button_layout->addWidget(btn_exit);

  vertical_popup_layout->addLayout(horisontal_host_layout);
  vertical_popup_layout->addLayout(horisontal_port_layout);
  vertical_popup_layout->addLayout(horisontal_button_layout);

  connectFileDialog->setLayout(vertical_popup_layout);

  //connect them to their actions
  connect(btn_exit, SIGNAL(released()),connectFileDialog,SLOT(close()));
  connect(btn_connect, SIGNAL(released()),this,SLOT(connectToServer()));
  connect(btn_connect, SIGNAL(released()),connectFileDialog,SLOT(close()));

  //set popup visiblem modal and 100 * 80 tall
  connectFileDialog->resize(100,80);
  connectFileDialog->setModal(true);
  connectFileDialog->show();
}

void Widget3D::addFilter(){

}

void Widget3D::set_rotation_center(){
  //set the rotation center in center of the mesh
  m_RenderView->resetCenterOfRotation();
  //be sure the mesh well placed
  m_RenderView->forceRender();
}

void Widget3D::set_rotation(int value){

  //set camera orientation
  switch (value)
  {

  case 0://RESET_POSITIVE_X:
    m_RenderView->resetViewDirection(1, 0, 0, 0, 0, 1);
    break;

  case 1://RESET_POSITIVE_Y:
    m_RenderView->resetViewDirection(0, 1, 0, 0, 0, 1);
    break;

  case 2://RESET_POSITIVE_Z:
    m_RenderView->resetViewDirection(0, 0, 1, 0, 1, 0);
    break;

  case 3://RESET_NEGATIVE_X:
    m_RenderView->resetViewDirection(-1, 0, 0, 0, 0, 1);
    break;

  case 4://RESET_NEGATIVE_Y:
    m_RenderView->resetViewDirection(0, -1, 0, 0, 0, 1);
    break;
  case 5://RESET_NEGATIVE_Z:
    m_RenderView->resetViewDirection(0, 0, -1, 0, 1, 0);
    break;
  }
}

void Widget3D::take_screen_shot(){

  // Screenshot
  //default file name
  QString file_name = "coolfluid3DScreenShot.png";

  //getting the file name and path for saving file
  file_name = QFileDialog::getSaveFileName(
        this, "Export File Name", file_name,
        "png Images (*.png)");

  //if the file name is set
  if ( !file_name.isEmpty() )
  {
    //create new PNG writer
    vtkPNGWriter *writer = vtkPNGWriter::New();
    //set rendered view as the input
    writer->SetInput(m_RenderView->captureImage(0));
    //set the file name
    writer->SetFileName(file_name.toStdString().c_str());
    //set dimention as 1-1 ratio
    writer->SetFileDimensionality(3);
    //write the file
    writer->Write();
    //show user info
    NLog::globalLog()->addMessage("Screen shot saved.");

    //or
    //          file_name = QFileDialog::getSaveFileName(
    //              this, "Export File Name", file_name,
    //              "png Images (*.png)");
    //          m_RenderView->saveImage(0,0,file_name);

  }
}

void Widget3D::reset_camera(){
  //reset camera position
  m_RenderView->resetCamera();
}

void Widget3D::show_color_editor(){
  if(m_RenderView && m_input){
    //get the view representation
    pqDataRepresentation* repr = m_source_list.at(m_actor_list->currentRow())->getRepresentation(m_RenderView);
    //pqDataRepresentation* repr = m_input->getRepresentation(m_RenderView);
    if(repr && m_RenderView->getWidget() && m_server && m_server->isRemote() && m_source){
      //create a scale color selector depending of the representation
      m_scaleEdit = new pqColorScaleEditor(m_RenderView->getWidget());
      m_scaleEdit->setRepresentation(repr);
      m_scaleEdit->show();
    }
  }
}
/*
    void Widget3D::reload(){
        NLog::globalLog()->addMessage("Reloading file");
        //reload the latest loaded file
        openFile(m_file_path);
        NLog::globalLog()->addMessage("File reloaded");
    }
*/
void Widget3D::loadFile(){
  NLog::globalLog()->addMessage("Loading file");
  //get file path from user input
  m_file_path = m_Path_File_line->text();
  //get file name from user input
  m_file_name = m_Name_line->text();
  //open this file
  openFile(m_file_path,m_file_name);
  NLog::globalLog()->addMessage("File loaded");
}

void Widget3D::connectToServer(){
  QString given_host = m_host_line->text();
  QString port = m_port_line->text();
  //connect to the chosen server
  connectToServer(given_host,port);
}

void Widget3D::loadPaths(std::vector<QString> paths,std::vector<QString> names){

  int save_last_index = m_source_list.size();

  //create source for eatch path
  for(int i=0;i< paths.size();++i){
    create_source(paths.at(i));
    m_source_list.push_back(m_source);

    QListWidgetItem * newItem = new QListWidgetItem(names.at(i));
    newItem->setIcon(QIcon(":/paraview_icons/pqEyeball16.png"));
    m_actor_list->addItem(newItem);
  }

  //add each source to the render
  for(int i=0;i< paths.size();++i){
    m_input = m_source_list.at(i+save_last_index);
    //create a data representation in server side, for the render window with the input
    m_object_builder->createDataRepresentation(m_input->getOutputPort(0), this->m_RenderView);
    vtkSMSourceProxy::SafeDownCast(m_input->getProxy())->UpdatePipeline();
  }

  //zoom to object
  this->m_RenderView->resetCamera();

  //make sure we update the view
  this->m_RenderView->render();

  m_mesh_options->setVisible(true);
  m_regions_box->setVisible(true);
}

void Widget3D::create_source(QString path){
  if(m_server && m_server->isRemote()){

    //detecting extention and launching correct reader
    QString extention = path.section('.',-1);

    //create reader on server side depending on file type
    if(!extention.compare("vtk")){ //vtk
      m_source = m_object_builder->createReader("sources", "LegacyVTKFileReader",QStringList(path), m_server);
    }
    if(!extention.compare("ex2")){ //ex2
      m_source = m_object_builder->createReader("sources", "ExodusIIReader",QStringList(path), m_server);
    }

    //if source has been created proprely
    if(m_source){

      //update the pipeline
      vtkSMSourceProxy::SafeDownCast(m_source->getProxy())->UpdatePipeline();
    }else{
      m_mesh_options->setVisible(false);
      m_regions_box->setVisible(false);
      NLog::globalLog()->addError("Source of this file path don't exist.");
    }
  }else{
    m_mesh_options->setVisible(false);
    m_regions_box->setVisible(false);
    NLog::globalLog()->addError("Cannot load a file if no ParaViewTab server connection is set.");
  }
}

void Widget3D::show_hide_actor(QListWidgetItem * item){
  if(m_RenderView->getRepresentation(m_actor_list->currentRow())->isVisible()){
    item->setTextColor(Qt::gray);
    item->setIcon(QIcon(":/paraview_icons/pqEyeballd16.png"));
  }else{
    item->setTextColor(Qt::black);
    item->setIcon(QIcon(":/paraview_icons/pqEyeball16.png"));
  }
  m_RenderView->getRepresentation(m_actor_list->currentRow())->setVisible(
        !m_RenderView->getRepresentation(m_actor_list->currentRow())->isVisible());
  this->m_RenderView->render();

}

void Widget3D::actor_changed(QListWidgetItem * item){
  //set the color selector representation (after it will directly apply changes to this representation)
  this->m_dataSet_selector->setRepresentation(m_source_list.at(m_actor_list->currentRow())->getRepresentation(m_RenderView));

  //set the style selector representation (after it will directly apply changes to this representation)
  this->m_mesh_style->setRepresentation(m_source_list.at(m_actor_list->currentRow())->getRepresentation(m_RenderView));

  representation = qobject_cast<pqPipelineRepresentation*>(m_source_list.at(m_actor_list->currentRow())->getRepresentation(m_RenderView));

  disconnect(m_spin_opacity,SIGNAL(valueChanged(double)),this,SLOT(opacityChange(double)));
  m_spin_opacity->setEnabled(true);
  m_spin_opacity->setValue(representation->getOpacity());
  connect(m_spin_opacity,SIGNAL(valueChanged(double)),this,SLOT(opacityChange(double)));

  //m_mesh_solid_color_set->setEnabled(this->m_dataSet_selector->getCurrentText() == representation->solidColor());

  this->m_RenderView->render();

  //qDebug() << this->m_dataSet_selector->getCurrentText();
  //m_mesh_solid_color_set->setEnabled(this->m_dataSet_selector->getCurrentText() == representation->solidColor());

}

void Widget3D::setColor(){

  if(representation){

    pqPipelineRepresentation* repr = qobject_cast<pqPipelineRepresentation*>(
          m_source_list.at(m_actor_list->currentRow())->getRepresentation(m_RenderView));
    if (!repr)
    {
      qCritical() << "No active representation.";
      return;
    }

    if (repr->getColorField() == pqPipelineRepresentation::solidColor())
    {
      // Get the color property.
      vtkSMProxy *proxy = repr->getProxy();
      vtkSMProperty *diffuse = proxy->GetProperty("DiffuseColor");
      vtkSMProperty* ambient = proxy->GetProperty("AmbientColor");
      int reprType = repr->getRepresentationType();
      bool use_ambient = (reprType == vtkSMPVRepresentationProxy::WIREFRAME ||
                          reprType == vtkSMPVRepresentationProxy::POINTS ||
                          reprType == vtkSMPVRepresentationProxy::OUTLINE);
      if (diffuse && ambient)
      {
        // Get the current color from the property.
        QList<QVariant> rgb =
            pqSMAdaptor::getMultipleElementProperty(diffuse);
        QColor color(Qt::white);
        if(rgb.size() >= 3)
        {
          color = QColor::fromRgbF(rgb[0].toDouble(), rgb[1].toDouble(),
                                   rgb[2].toDouble());
        }

        // Let the user pick a new color.
        color = QColorDialog::getColor(color, this);
        if(color.isValid())
        {
          // Set the properties to the new color.
          rgb.clear();
          rgb.append(color.redF());
          rgb.append(color.greenF());
          rgb.append(color.blueF());
          pqSMAdaptor::setMultipleElementProperty(
                use_ambient? ambient : diffuse, rgb);
          proxy->UpdateVTKObjects();
          // need to break any global-property link that might have existed
          // with this property.
          pqStandardColorLinkAdaptor::breakLink(proxy,
                                                use_ambient? "AmbientColor" : "DiffuseColor");
        }
      }
    }
  }
  else{
    //no Region selected
  }
}

void Widget3D::opacityChange(double value){
  disconnect(m_spin_opacity,SIGNAL(valueChanged(double)),this,SLOT(opacityChange(double)));
  if(representation){
    vtkSMProxy *proxy = representation->getProxy();
    vtkSMPropertyHelper(proxy, "Opacity").Set(value);
    proxy->UpdateVTKObjects();
  }
  connect(m_spin_opacity,SIGNAL(valueChanged(double)),this,SLOT(opacityChange(double)));
}

void Widget3D::update_solide_color_button_state(pqVariableType type, const QString &name){
  m_mesh_solid_color_set->setEnabled(name == "Solid Color");
}

////////////////////////////////////////////////////////////////////////////////

} // ParaViewTab
} // UI
} // CF
