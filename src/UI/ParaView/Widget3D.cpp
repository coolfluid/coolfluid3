// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// Qt headers
#include <QHBoxLayout>
#include <QLabel>
#include <QDialog>
#include <QIntValidator>
#include <QMap>
#include <QList>
#include <QDebug>
#include <QFileDialog>
#include <QStringList>

// ParaView header
#include "vtkSMPropertyHelper.h"
#include "pqDataRepresentation.h"
#include "vtkSmartPointer.h"
#include "vtkPolyData.h"
#include "vtkPNGWriter.h"
#include "vtkWindowToImageFilter.h"

// header
#include "UI/ParaView/Widget3D.hpp"
#include "UI/Core///NLog.hpp"
////////////////////////////////////////////////////////////////////////////////

using namespace CF::UI::Core;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ParaView {

////////////////////////////////////////////////////////////////////////////////

    Widget3D::Widget3D(QWidget *parent) :
            QWidget(parent)
    {
      //paraview instance and builder
        // automatically make a server connection
        m_core = pqApplicationCore::instance();
        m_object_builder = m_core->getObjectBuilder();

        // Register ParaView interfaces.
        m_plugin_manager = m_core->getPluginManager();

        // adds support for standard paraview views.
        m_plugin_manager->addInterface(new pqStandardViewModules(m_plugin_manager));

        // set the lookuptable to be able using legend.
        m_core->setLookupTableManager(new pqPQLookupTableManager(this));

        //define timeout of connection process
//        pqProgressManager * m_progress_manager = m_core->getProgressManager();
//        m_progress_manager->setEnableProgress(false);

        //main layout
        m_layout_v = new QVBoxLayout();
        this->setLayout(m_layout_v);

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

        m_reload = new QPushButton(QIcon(":/paraview_icons/ShowCenterButton.png"),"Reload Mesh",this);
        m_reload->setVisible(false);

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
        m_server->setHeartBeatTimeoutSetting(10);

        if(m_server){
            //create the builtin server view
            createView();
        }else{
            NLog::globalLog()->addError("Error while creating widget3d");
        }

        //disposition
        m_server_options = new QGroupBox("Server Options",this);
        m_server_options->setMaximumWidth(200);
        m_camera_options = new QGroupBox("Camera Options",this);
        m_camera_options->setMaximumWidth(200);
        m_mesh_options = new QGroupBox("Mesh Options",this);
        m_mesh_options->setVisible(false);

        m_layout_server_options = new QVBoxLayout();
        m_layout_camera_options = new QVBoxLayout();
        m_layout_mesh_options = new QHBoxLayout();

        m_server_options->setLayout(m_layout_server_options);
        m_camera_options->setLayout(m_layout_camera_options);
        m_mesh_options->setLayout(m_layout_mesh_options);

        m_layout_server_options->addWidget(this->m_connect_to_server_button);
        m_layout_server_options->addWidget(this->m_load_file);
        m_layout_mesh_options->addWidget(this->m_mesh_style);
        m_layout_camera_options->addWidget(this->m_set_rotation_center);
        m_layout_mesh_options->addWidget(this->m_dataSet_selector);
        m_layout_camera_options->addWidget(this->m_reset_camera);
        m_layout_camera_options->addWidget(this->m_screen_shot);
        m_layout_camera_options->addWidget(this->m_preDefined_rotation);
        m_layout_mesh_options->addWidget(this->m_show_color_palette);
        m_layout_server_options->addWidget(this->m_reload);

        m_layout_option->addWidget(this->m_server_options);
        m_layout_option->addWidget(this->m_camera_options);

        m_layout_h->addLayout(this->m_layout_option);

        m_layout_v->addLayout(m_layout_h);
        m_layout_v->addWidget(this->m_mesh_options);

        //connect
        connect(m_connect_to_server_button,SIGNAL(released()),this,SLOT(showConnectDialog()));
        connect(m_load_file,SIGNAL(released()),this,SLOT(showLoadFileDialog()));
        connect(m_set_rotation_center,SIGNAL(released()),this,SLOT(set_rotation_center()));
        connect(m_preDefined_rotation,SIGNAL(activated(int)),this,SLOT(set_rotation(int)));
        connect(m_screen_shot,SIGNAL(released()),this,SLOT(take_screen_shot()));
        connect(m_reset_camera,SIGNAL(released()),this,SLOT(reset_camera()));
        connect(m_show_color_palette,SIGNAL(released()),this,SLOT(show_color_editor()));
        connect(m_reload,SIGNAL(released()),this,SLOT(reload()));
    }

    void Widget3D::connectToServer(QString given_host,QString port)
    {
            //pqServerResource configuration
            QString host = "cs://";//cs:// =>client-server    rc:// => remote server connection (not implemented yet by paraview)
            host += given_host;
            host += ":";
            host += port;

            if(m_server)
                //if any remote or builtin server, delete them to free ressources
                m_object_builder->removeServer(m_server);

            //create new remote server
            m_server = m_object_builder->createServer(pqServerResource(host));

            //if the server is remote then :
            if(m_server && m_server->isRemote()){
                //show user info
                NLog::globalLog()->addMessage("Connected to paraview server");

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
                NLog::globalLog()->addError("Error while connecting to paraview server");
                m_server = m_object_builder->createServer(pqServerResource("builtin:"));
                createView();
            }

    }

    void Widget3D::openFile(QString file_path)
    {
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
              }
          }else{
            m_mesh_options->setVisible(false);
            NLog::globalLog()->addError("Source of this file path don't exist.");
          }
      }else{
        m_mesh_options->setVisible(false);
        NLog::globalLog()->addError("Cannot load a file if no paraview server connection is set.");
      }
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
        NLog::globalLog()->addError("Cannot create RenderView if no paraview server connection is set.");
      }
    }

    void Widget3D::disconnectFromServer(){

        //remove source if any
        if(m_source)
            m_object_builder->destroySources(m_server);

        //remove server if any (will remove all object on the server)
        if(m_server)
            m_object_builder->removeServer(m_server);

        //remove view if any
        if(m_RenderView)
            m_object_builder->destroy(m_RenderView);

        //create a builtin server to have axes shown
        m_server = m_object_builder->createServer(pqServerResource("builtin:"));

        if(m_server){
            //create the builtin server view
            createView();
        }

        //hide mesh options
        m_mesh_options->setVisible(false);

        //change disconnect button to connect button
        this->m_connect_to_server_button->setText("Connect");
        this->m_connect_to_server_button->setIcon(QIcon(":/paraview_icons/pqConnect24.png"));
        disconnect(m_connect_to_server_button,SIGNAL(clicked()),this,SLOT(disconnectFromServer()));
        connect(m_connect_to_server_button,SIGNAL(released()),this,SLOT(showConnectDialog()));

        //show user info
        NLog::globalLog()->addMessage("Disconnected from paraview server");

        //disconnected, cannot load or reload file while not connected
        m_load_file->setVisible(false);
        m_reload->setVisible(false);

    }

    void Widget3D::showLoadFileDialog(){
        //the popup dialog box
        QPointer<QDialog> loadFileDialog = new QDialog(this);

        //adding Exit and Connect button
        QPointer<QPushButton> btn_load = new QPushButton("Load", loadFileDialog);
        QPointer<QPushButton> btn_exit = new QPushButton("Exit", loadFileDialog);

        // line edit
        m_Path_File_line = new QLineEdit("/nobackup/st/wertz/frog/skeleton.vtk",this);

        // labels
        QPointer<QLabel> path_label = new QLabel("File Path");

        // popup layout
        QPointer<QVBoxLayout> vertical_popup_layout = new QVBoxLayout();
        QPointer<QHBoxLayout> horisontal_path_layout = new QHBoxLayout();
        QPointer<QHBoxLayout> horisontal_button_layout = new QHBoxLayout();

        horisontal_path_layout->addWidget(path_label);
        horisontal_path_layout->addWidget(m_Path_File_line);

        horisontal_button_layout->addWidget(btn_load);
        horisontal_button_layout->addWidget(btn_exit);

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
        }
    }

    void Widget3D::reset_camera(){
      //reset camera position
      m_RenderView->resetCamera();
    }

    void Widget3D::show_color_editor(){
      if(m_RenderView && m_input){
        //get the view representation
        pqDataRepresentation* repr = m_input->getRepresentation(m_RenderView);
        if(repr && m_RenderView->getWidget() && m_server && m_server->isRemote() && m_source){
          //create a color selector depending of the representation
          m_scaleEdit = new pqColorScaleEditor(m_RenderView->getWidget());
          m_scaleEdit->setRepresentation(repr);
          m_scaleEdit->show();
        }
      }
    }

    void Widget3D::reload(){
        NLog::globalLog()->addMessage("Reloading file");
        //reload the latest loaded file
        openFile(m_file_path);
        NLog::globalLog()->addMessage("File reloaded");
    }

    void Widget3D::loadFile(){
        NLog::globalLog()->addMessage("Loading file");
        //get file path from user input
        m_file_path = m_Path_File_line->text();
        //open this file
        openFile(m_file_path);
        NLog::globalLog()->addMessage("File loaded");
    }

    void Widget3D::connectToServer(){
      QString given_host = m_host_line->text();
      QString port = m_port_line->text();
      //connect to the chosen server
      connectToServer(given_host,port);
    }

    void Widget3D::connectToServer(QString host,QString port, QString path){
      //connect to the server "host" with port "port"
      connectToServer(host,port);
      //open the file from "path"
      openFile(path);
    }


////////////////////////////////////////////////////////////////////////////////

} // ParaView
} // UI
} // CF
