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

// ParaView header
#include "vtkSMPropertyHelper.h"
#include "pqDataRepresentation.h"
#include "vtkSmartPointer.h"
#include "vtkLookupTable.h"
#include "vtkUnsignedCharArray.h"
#include "vtkPolyData.h"
#include "vtkSMDocumentation.h"
#include "vtkPVDataInformation.h"
#include "pqProgressManager.h"
#include "pqTimeKeeper.h"


// header
#include "UI/ParaView/Widget3D.hpp"
#include "UI/Core/NLog.hpp"
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
        // automatically make a server connection
        m_core = pqApplicationCore::instance();
        m_object_builder = m_core->getObjectBuilder();

        // Register ParaView interfaces.
        m_plugin_manager = m_core->getPluginManager();

        // adds support for standard paraview views.
        m_plugin_manager->addInterface(new pqStandardViewModules(m_plugin_manager));

        //define timeout of connection process
//        pqProgressManager * m_progress_manager = m_core->getProgressManager();
//        m_progress_manager->setEnableProgress(false);


        //pqServerManagerModel* sm = m_core->getServerManagerModel();
        m_color = new pqDisplayColorWidget(this);


        m_layout_v = new QVBoxLayout();

        m_layout_option = new QHBoxLayout();

        m_connect_to_server_button = new QPushButton("Connect",this);

        m_load_file = new QPushButton("Load File",this);

        m_load_file->setEnabled(false);

        m_set_rotation_center = new QPushButton("Set Rotation Center",this);

        m_style = new QComboBox(this);
        m_style->addItem("Point",0);
        m_style->addItem("Wireframe",1);
        m_style->addItem("Surface",2);
        m_style->addItem("Outline",3);
        m_style->addItem("Volume",4);
        m_style->addItem("Surface with edges",5);
        m_style->setEditable(false);
        m_style->setCurrentIndex(2);
        m_style->setEnabled(false);

        this->setLayout(m_layout_v);

        m_server = m_object_builder->createServer(pqServerResource("builtin:"));

        if(m_server){
            createView();
        }else{
            NLog::globalLog()->addError("Error while creating widget3d");
        }


        //disposition
        this->m_connect_to_server_button->setMaximumWidth(100);
        this->m_load_file->setMaximumWidth(100);
        this->m_style->setMaximumWidth(150);
        this->m_set_rotation_center->setMaximumWidth(150);
        this->m_color->setMaximumWidth(250);

        m_layout_option->addWidget(this->m_connect_to_server_button);
        m_layout_option->addWidget(this->m_load_file);
        m_layout_option->addWidget(this->m_style);
        m_layout_option->addWidget(this->m_set_rotation_center);
        m_layout_option->addWidget(this->m_color);


        m_layout_v->addLayout(this->m_layout_option);

        //connect
        connect(m_connect_to_server_button,SIGNAL(released()),this,SLOT(showConnectDialog()));
        connect(m_load_file,SIGNAL(released()),this,SLOT(showLoadFileDialog()));
        connect(m_style,SIGNAL(currentIndexChanged (int)),this,SLOT(changeStyle()));
        connect(m_set_rotation_center,SIGNAL(released()),this,SLOT(set_rotation_center()));

    }

    void Widget3D::connectToServer()
    {

        QString given_host = m_host_line->text();
        QString port = m_port_line->text();

            QString host = "cs://";
            host += given_host;
            host += ":";
            host += port;

            if(m_server)
                m_object_builder->removeServer(m_server);

            m_server = m_object_builder->createServer(pqServerResource(host));

            if(m_server && m_server->isRemote()){
                NLog::globalLog()->addMessage("Connected to paraview server");
                this->m_connect_to_server_button->setText("Disconnect");
                m_load_file->setEnabled(true);
                disconnect(m_connect_to_server_button,SIGNAL(released()),this,SLOT(showConnectDialog()));
                connect(m_connect_to_server_button,SIGNAL(clicked()),this,SLOT(disconnectFromServer()));
                createView();
            }else{
                NLog::globalLog()->addError("Error while connecting to paraview dserver");
                m_server = m_object_builder->createServer(pqServerResource("builtin:"));
                createView();
            }

    }

    void Widget3D::openFile()
    {

        if(m_source)
            m_object_builder->destroy(m_source);

        QString given_file = m_Path_File_line->text();

        if(m_server && m_server->isRemote()){
        //detecting extention and launching correct reader

          QString extention = given_file.section('.',-1);

        //create reader depending on file
          if(!extention.compare("vtk")){
              m_source = m_object_builder->createReader("sources", "LegacyVTKFileReader",QStringList(given_file), m_server);
          }
          if(!extention.compare("ex2")){
              m_source = m_object_builder->createReader("sources", "ExodusIIReader",QStringList(given_file), m_server);
          }

          if(m_source){

              vtkSMSourceProxy::SafeDownCast(m_source->getProxy())->UpdatePipeline();

              if(m_source && m_server){
                  m_style->setEnabled(true);
                  showRender();
              }
          }else{
              m_style->setEnabled(false);
              NLog::globalLog()->addError("Source of this file path don't exist.");
          }
      }else{
          m_style->setEnabled(false);
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
            m_input = m_source;//m_filter2;

            m_object_builder->createDataRepresentation(m_input->getOutputPort(0), this->m_RenderView);

            changeStyle();



            pqDataRepresentation* repr = m_input->getRepresentation(m_RenderView);

            m_color->setRepresentation(repr);

            qDebug() << repr->getInputDataInformation()->GetNumberOfDataSets();
            qDebug() << repr->getInputDataInformation()->GetNumberOfPoints();
            qDebug() << repr->getInputDataInformation()->GetDataSetTypeAsString();
            qDebug() << repr->getInputDataInformation()->GetNumberOfCells();
            qDebug() << repr->getInputDataInformation()->GetNumberOfRows();
            qDebug() << repr->getInputDataInformation()->GetMemorySize();
            qDebug() << repr->getInputDataInformation()->GetPolygonCount();

            qDebug() << m_input->getNumberOfOutputPorts();
            qDebug() << repr->getLookupTable ();


            //LOOKUP_TABLE

            /////////////////////////////////////////////////////////////////////////
/*
            vtkPolyData* outputPolyData;

            double bounds[6];
            outputPolyData->GetBounds(bounds);

            // Find min and max z
            double minz = bounds[4];
            double maxz = bounds[5];

            // Create the color map
            vtkSmartPointer<vtkLookupTable> colorLookupTable =
              vtkSmartPointer<vtkLookupTable>::New();
            colorLookupTable->SetTableRange(minz, maxz);
            colorLookupTable->Build();

            // Generate the colors for each point based on the color map
            vtkSmartPointer<vtkUnsignedCharArray> colors =
              vtkSmartPointer<vtkUnsignedCharArray>::New();
            colors->SetNumberOfComponents(3);
            colors->SetName("Colors");

            for(int i = 0; i < outputPolyData->GetNumberOfPoints(); i++)
              {
              double p[3];
              outputPolyData->GetPoint(i,p);

              double dcolor[3];
              colorLookupTable->GetColor(p[2], dcolor);
              unsigned char color[3];
              for(unsigned int j = 0; j < 3; j++)
                {
                color[j] = static_cast<unsigned char>(255.0 * dcolor[j]);
                }
              colors->InsertNextTupleValue(color);
              }

            outputPolyData->GetPointData()->SetScalars(colors);
*/

            /////////////////////////////////////////////////////////////////////////

            //repr->setProperty("Representation",0);
/*
            vtkSMPropertyHelper(repr->getProxy(),"Representation").Set(0);

            vtkSMViewProxy::SafeDownCast(repr->getProxy())->UpdatePipeline();

            m_RenderView->getRepresentation(0)->setProperty("Representation",0);
*/

            /*
            // put the elevation in the window
            QString numPort = "Number of source port : ";
            numPort += QString::number(m_source->getNumberOfOutputPorts());
            ClientNLog::globalLog()->addMessage(numPort);

            m_object_builder->createDataRepresentation(m_source->getOutputPort(0), this->m_RenderView);
*/            // zoom to object
            this->m_RenderView->resetCamera();
            // make sure we update
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
          m_layout_v->insertWidget(0,this->m_RenderView->getWidget());
        }else{
            NLog::globalLog()->addError("Problem when creating a RenderView.");
        }
        }else{
            NLog::globalLog()->addError("Cannot create RenderView if no paraview server connection is set.");
        }

    }

    void Widget3D::disconnectFromServer(){

        if(m_source)
            m_object_builder->destroySources(m_server);

        if(m_server)
            m_object_builder->removeServer(m_server);

        if(m_RenderView)
            m_object_builder->destroy(m_RenderView);

        m_server = m_object_builder->createServer(pqServerResource("builtin:"));

        createView();

        m_style->setEnabled(false);

        this->m_connect_to_server_button->setText("Connect");

        NLog::globalLog()->addMessage("Deconnection from paraview server");

        m_load_file->setEnabled(false);
        disconnect(m_connect_to_server_button,SIGNAL(clicked()),this,SLOT(disconnectFromServer()));
        connect(m_connect_to_server_button,SIGNAL(released()),this,SLOT(showConnectDialog()));
    }

    void Widget3D::showLoadFileDialog(){
        //the popup dialog box
        QPointer<QDialog> loadFileDialog = new QDialog(this);

        //adding Exit and Connect button
        QPointer<QPushButton> btn_load = new QPushButton("Load", loadFileDialog);
        QPointer<QPushButton> btn_exit = new QPushButton("Exit", loadFileDialog);

        ///nobackup/st/wertz/frog/skeleton.vtk
        ///students/st_10_11/wertz/Downloads/mymodel.vtk
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

        //connect them to their actions
        connect(btn_exit, SIGNAL(released()),loadFileDialog,SLOT(close()));
        connect(btn_load, SIGNAL(released()),this,SLOT(openFile()));
        connect(btn_load, SIGNAL(released()),loadFileDialog,SLOT(close()));

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

        connectFileDialog->resize(100,80);
        connectFileDialog->setModal(true);
        connectFileDialog->show();
    }

    void Widget3D::addFilter(){

    }

    void Widget3D::changeStyle(){
        pqDataRepresentation* repr = m_input->getRepresentation(m_RenderView);
        if (repr)
          {
          vtkSMPropertyHelper(repr->getProxy(), "Representation").Set(m_style->currentIndex());
          repr->getProxy()->UpdateVTKObjects();
          m_RenderView->forceRender();
          }
    }

    void Widget3D::set_rotation_center(){
      m_RenderView->resetCenterOfRotation();
      m_RenderView->forceRender();
    }


////////////////////////////////////////////////////////////////////////////////

} // ParaView
} // UI
} // CF
