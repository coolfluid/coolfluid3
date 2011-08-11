// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// Qt headers
#include <QAction>
#include <QLabel>
#include <QFileDialog>
#include <QColorDialog>
#include <QScrollArea>
#include <QToolBar>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDoubleSpinBox>
#include <QCheckBox>

// ParaView header
#include "pqDataRepresentation.h"
#include "pqSettings.h"
#include "pqStandardColorLinkAdaptor.h"
#include "vtkSMPVRepresentationProxy.h"
#include "pqSMAdaptor.h"
#include "pqStandardViewModules.h"
#include "pqPQLookupTableManager.h"
#include "vtkSMPropertyHelper.h"
#include "pqServerManagerModel.h"
#include "pqCameraDialog.h"
#include "pqGlobalRenderViewOptions.h"
#include "pqStatusBar.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqPluginManager.h"
#include "pqServer.h"
#include "pqRenderView.h"
#include "pqDisplayColorWidget.h"
#include "pqColorScaleEditor.h"
#include "pqDisplayRepresentationWidget.h"
#include "pqPipelineRepresentation.h"
#include "pqDisplayProxyEditorWidget.h"
#include "pqServerResource.h"
#include "vtkSMSourceProxy.h"
#include "pqProgressManager.h"

// header
#include "UI/Graphics/NRemoteOpen.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/ParaViewTab/N3DView.hpp"
#include "UI/ParaViewTab/Widget3D.hpp"
#include "UI/Core/NTree.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF::UI::Core;
using namespace CF::UI::Graphics;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ParaViewTab {

////////////////////////////////////////////////////////////////////////////////
Widget3D::Widget3D(QWidget *parent) :
  QWidget(parent)
{

  QToolBar * tool_bar = new QToolBar(this);
  tool_bar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

  /** Creation Phase **/

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

  //get the object builder
  m_object_builder = m_core->getObjectBuilder();

  // Register ParaView interfaces.
  m_plugin_manager = m_core->getPluginManager();

  // adds support for standard paraview views.
  m_plugin_manager->addInterface(new pqStandardViewModules(m_plugin_manager));

  // set the lookuptable to be able to show legend.
  m_core->setLookupTableManager(new pqPQLookupTableManager(this));

  progMgr = m_core->getProgressManager();


  /// Layout
  //main layout
  m_layout_v = new QVBoxLayout();
  this->setLayout(m_layout_v);

  //other layout
  //horisontal layout view + options
  m_layout_h = new QHBoxLayout();

  //vertical layout containing options
  m_layout_option = new QVBoxLayout();

  //GroupBox's layout
  m_layout_mesh_options = new QHBoxLayout();
  m_layout_regions_box = new QVBoxLayout();

  //Server advanced options
  //      pqGlobalRenderViewOptions * opt = new pqGlobalRenderViewOptions(this);
  //pqRenderViewOptions * opt = new pqRenderViewOptions(this);
  //pqApplicationOptionsDialog * opt = new pqApplicationOptionsDialog(this);

  //qDebug() << opt->getPageList();
  //      opt->setPage(opt->getPageList().at(3));
  //      m_layout_v->addWidget(opt);


  /// Options
  //Server Options
  m_action_connect = tool_bar->addAction(QIcon(":/paraview_icons/pqConnect24.png"),
                                         "Connect", this, SLOT(showConnectDialog()));
  m_action_disconnect = tool_bar->addAction(QIcon(":/paraview_icons/pqDisconnect24.png"),
                                            "Disconnect", this, SLOT(disconnectFromServer()));

  m_action_load_file = tool_bar->addAction(QIcon(":/paraview_icons/pqOpen24.png"),
                                           "Load File", this, SLOT(showLoadFileDialog()));
  m_action_disconnect->setEnabled( false );
  m_action_load_file->setEnabled( false );


  //View Options
  tool_bar->addAction(QIcon(":/paraview_icons/pqResetCenter24.png"), "Fix Center", this, SLOT(set_rotation_center()));
  tool_bar->addAction(QIcon(":/paraview_icons/pqCaptureScreenshot24.png"), "Screen Shot", this, SLOT(take_screen_shot()));
  tool_bar->addAction(QIcon(":/paraview_icons/pqResetCamera24.png"), "Reset Camera", this, SLOT(reset_camera()));


  //Combo box of predefined camera orientation
  m_preDefined_rotation = new QComboBox(this);
  m_preDefined_rotation->addItem(QIcon(":/paraview_icons/pqXPlus16.png"),"+X",0);
  m_preDefined_rotation->addItem(QIcon(":/paraview_icons/pqYPlus16.png"),"+Y",1);
  m_preDefined_rotation->addItem(QIcon(":/paraview_icons/pqZPlus16.png"),"+Z",2);
  m_preDefined_rotation->addItem(QIcon(":/paraview_icons/pqXMinus16.png"),"-X",3);
  m_preDefined_rotation->addItem(QIcon(":/paraview_icons/pqYMinus16.png"),"-Y",4);
  m_preDefined_rotation->addItem(QIcon(":/paraview_icons/pqZMinus16.png"),"-Z",5);
  m_preDefined_rotation->setMinimumWidth(100);
  m_preDefined_rotation->setEditable(false);

  //Combo box of predefined camera orientation
  m_list_selection = new QComboBox(this);
  m_list_selection->addItem(QIcon(":/paraview_icons/SimpleSeletcion.png"),"Single Selection",1);
  m_list_selection->addItem(QIcon(":/paraview_icons/multiSeletcions.png"),"Multi Selection",2);
  //  m_list_selection->addItem("Extended Selection",3);
  //  m_list_selection->addItem("Contiguous Selection",4);
  m_list_selection->setEditable(false);

  tool_bar->addWidget(m_preDefined_rotation);

  // show axes button
  tool_bar->addAction(QIcon(":/paraview_icons/pqShowOrientationAxes32.png"), "Axes Visibility", this, SLOT(setCenterAxesVisibility()));

  // Show camera settings dialog button
  tool_bar->addAction(QIcon(":/paraview_icons/pqProbeLocation24.png"), "Camera settings", this, SLOT(show_camera_settings()));

  // advanced paraview options (not used for now)
  this->m_disp_adv_opt_button = new QPushButton("Region Adv. Settings");
  this->m_disp_adv_opt_button->setEnabled(false);
  this->m_gen_adv_opt_button = new QPushButton("General Adv. Settings");
  this->m_serv_adv_opt_button = new QPushButton("Server Adv. Settings");

  showAdvOptions(NTree::globalTree().get()->isAdvancedMode());

  //Mesh Options
  m_show_color_palette = new QPushButton(QIcon(":/paraview_icons/pqScalarBar24.png"),"Show Color Palette",this);
  m_show_color_palette->setEnabled(false);

  m_mesh_style = new pqDisplayRepresentationWidget(this); //Combo box that handle styles

  m_dataSet_selector = new pqDisplayColorWidget(this); // the scale color selector and legend

  m_mesh_solid_color_set = new QPushButton(QIcon(":/paraview_icons/color.png"),"Set Solid Color"); // Set Solide Color button
  m_mesh_solid_color_set->setEnabled(false);

  //Opacity spinner
  QLabel * opacity_label = new QLabel("Opacity:");
  opacity_label->setAlignment(Qt::AlignHCenter);
  m_spin_opacity = new QDoubleSpinBox(this);
  m_spin_opacity->setMaximum(1);
  m_spin_opacity->setSingleStep(0.1);
  m_spin_opacity->setMinimum(0);
  m_spin_opacity->setAlignment(Qt::AlignHCenter);
  QVBoxLayout * opacityLayout = new QVBoxLayout();
  opacityLayout->addWidget(opacity_label);
  opacityLayout->addWidget(m_spin_opacity);

  //Regions list
  m_actor_list = new QListWidget(this);

  //Create "force render" button and "Auto Render" checkbox
  m_action_force_rendering = tool_bar->addAction(QIcon(":/paraview_icons/render_region.png"), "Render", this, SLOT(forceRendering()));

  m_checkbox_enable_rendering = new QCheckBox("Auto Render"); //QIcon(":/paraview_icons/pqVcrLoop24.png")
  tool_bar->addWidget(m_checkbox_enable_rendering);

  /// Progress Bar
  // The progress bar (not used for now)
  pqStatusBar * status_bar = new pqStatusBar();


  /// Server
  //create a builtin server to have axes shown
  m_server = m_object_builder->createServer(pqServerResource("builtin:"));

  if(m_server){
    //create the builtin server view
    createView();
    //disable instant rendering
    enableRendering(false);
  }else{
    NLog::globalLog()->addError("Error while creating 'builtin server'");
  }

  //Group Box creation
  m_mesh_options = new QGroupBox("Mesh Options",this);
  m_mesh_options->setVisible(false);
  m_mesh_options->setEnabled(false);
  m_regions_box = new QGroupBox("Regions",this);
  m_regions_box->setVisible(false);

  /** Disposition Phase **/
  //Set GroupBox layout
  m_mesh_options->setLayout(m_layout_mesh_options);
  m_regions_box->setLayout(m_layout_regions_box);

  //Mesh layouts
  m_layout_mesh_options->addWidget(this->m_mesh_style);
  m_layout_mesh_options->addWidget(this->m_dataSet_selector);
  m_layout_mesh_options->addWidget(this->m_show_color_palette);
  m_layout_mesh_options->addWidget(this->m_mesh_solid_color_set);
  m_layout_mesh_options->addLayout(opacityLayout);

  //Option layouts
  m_layout_option->addWidget(this->m_regions_box);
  m_layout_option->addWidget(this->m_disp_adv_opt_button);
  m_layout_option->addWidget(this->m_gen_adv_opt_button);
  m_layout_option->addWidget(this->m_serv_adv_opt_button);

  //Horizotal layouts
  m_layout_h->addLayout(this->m_layout_option);

  m_layout_h->setStretchFactor(m_layout_option, 0);

  //Main layouts
  m_layout_v->addWidget(tool_bar);
  m_layout_v->addLayout(m_layout_h);
  m_layout_v->addWidget(this->m_mesh_options);
//  m_layout_v->addWidget(status_bar);

  m_layout_v->setStretchFactor(tool_bar, 0);
  m_layout_v->setStretchFactor(m_layout_h, 10);
  m_layout_v->setStretchFactor(m_mesh_options, 0);

  //Actor layouts
  m_layout_regions_box->addWidget(this->m_actor_list);
  m_layout_regions_box->addWidget(this->m_list_selection);


  /** Connection Phase **/
  connect(m_actor_list,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(show_hide_actor(QListWidgetItem*)));
  connect(m_actor_list,SIGNAL(itemSelectionChanged()),this,SLOT(actor_changed()));
  connect(m_preDefined_rotation,SIGNAL(activated(int)),this,SLOT(set_rotation(int)));
  connect(m_show_color_palette, SIGNAL(released()), this, SLOT(show_color_editor()));
  connect(m_mesh_solid_color_set,SIGNAL(released()),this,SLOT(set_solid_color()));
  connect(m_dataSet_selector,SIGNAL(variableChanged(pqVariableType, const QString)),this,SLOT(enable_solide_color_button(pqVariableType, const QString)));
  connect(m_disp_adv_opt_button,SIGNAL(released()),this,SLOT(show_disp_adv_settings()));
  connect(m_gen_adv_opt_button,SIGNAL(released()),this,SLOT(show_gen_adv_settings()));
  connect(m_serv_adv_opt_button,SIGNAL(released()),this,SLOT(show_serv_adv_settings()));
  connect(m_checkbox_enable_rendering,SIGNAL(toggled(bool)),this,SLOT(enableRendering(bool)));
  connect(m_list_selection,SIGNAL(activated(int)),this,SLOT(setActorListSelectionMode(int)));
  connect(NTree::globalTree().get(),SIGNAL(advancedModeChanged(bool)),this,SLOT(showAdvOptions(bool)));

//  qDebug() << "widget built";

}

void Widget3D::connectToServer(QString given_host,QString port)
{

  //Be sure we are not connected yet and all widget are in a correct state
  disconnectFromServer();

  //pqServerResource configuration
  QString host = "cs://"; //cs:// =>client-server    rc:// => remote server connection (not implemented yet by paraview)
  host += given_host;
  host += ":";
  host += port;

  //create new remote server
  m_server = m_object_builder->createServer(pqServerResource(host));

  //if the server is remote then :
  if(m_server && m_server->isRemote()){
    //show user info
    NLog::globalLog()->addMessage("Connected to paraview server");

    //show the "load file" button
    m_action_load_file->setEnabled(true);

    //change connect button to disconnect button
    m_action_connect->setEnabled(false);
    m_action_disconnect->setEnabled(true);

    //create server view
    createView();
  }else{
    NLog::globalLog()->addError("Error while connecting to paraview server");
    //Set Server to a stable state
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
}

void Widget3D::createView(){
  if(m_server){
    // create a graphics window and put it in our main window
    this->m_RenderView = qobject_cast<pqRenderView*>(
          m_object_builder->createView(pqRenderView::renderViewType(), m_server));


    if(m_RenderView){
      //put the view in the 0 index so it is the first widget of the layout (avoid bugs)
      m_layout_h->insertWidget(0,this->m_RenderView->getWidget());
      m_layout_h->setStretchFactor(this->m_RenderView->getWidget(), 10);
    }else{
      NLog::globalLog()->addError("Problem when creating a RenderView.");
    }
  }else{
    NLog::globalLog()->addError("Cannot create RenderView if no paraview server connection is set.");
  }
}

void Widget3D::disconnectFromServer(){

  //remove server if any (will remove all object on the server)
  if(m_server)
    //show user info
    if(m_server->isRemote()){
      NLog::globalLog()->addMessage("Disconnected from paraview server");
    }
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

  //disable mesh options
  m_mesh_options->setEnabled(false);
  m_disp_adv_opt_button->setEnabled(false);
  this->m_dataSet_selector->setRepresentation(0);
  this->m_mesh_style->setRepresentation(0);

  if(m_server){
    //create the builtin server view
    createView();
  }

  //hide mesh and region options
  m_mesh_options->setVisible(false);
  m_regions_box->setVisible(false);

  //change disconnect button to connect button
  m_action_connect->setEnabled(true);
  m_action_disconnect->setEnabled(false);

  //disconnected, cannot load file while not connected
  m_action_load_file->setEnabled(false);

}

void Widget3D::showLoadFileDialog(){
  // Create a Server file browser Dialog
  NRemoteOpen::Ptr loadFileDialog = NRemoteOpen::create();

  QStringList path_list = loadFileDialog->showMultipleSelect();

  for(int i=0;i<path_list.size();++i)
  {
    // Show the file Dialog and load a file
    QFileInfo * fileinfo = new QFileInfo(path_list.at(i));

    if(!fileinfo->filePath().isEmpty())
    {
      NLog::globalLog()->addMessage("Loading file");
      //open this file
      openFile(fileinfo->filePath(),fileinfo->fileName().section('.',0,0));
    }
    else
    {
      //error
      NLog::globalLog()->addError("Cannot Load file !");
    }
  }
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
  m_host_line = new QLineEdit("localhost",this);

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

void Widget3D::set_rotation_center(){
  //set the rotation center in center of the mesh
  m_RenderView->resetCenterOfRotation();
  //be sure the mesh updated
  m_RenderView->getWidget()->update();
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

  //Get File name
  file_name = QFileDialog::getSaveFileName(
        this, "Export File Name", file_name,
        "png Images (*.png)");
  //Save the file
  m_RenderView->saveImage(0,0,file_name);

  //}
}

void Widget3D::reset_camera(){
  //reset camera position
  m_RenderView->resetCamera();
}

void Widget3D::show_color_editor(){
  if(m_RenderView){
    //get the view representation
    pqDataRepresentation* repr = m_source_list.at(m_actor_list->currentRow())->getRepresentation(m_RenderView);

    if(repr && m_RenderView->getWidget() && m_server && m_server->isRemote())
    {
      //create a scale color selector depending of the representation
      m_scaleEdit = new pqColorScaleEditor(m_RenderView->getWidget());
      m_scaleEdit->setRepresentation(repr);
      m_scaleEdit->show();

    }
  }
}

void Widget3D::connectToServer(){
  QString given_host = m_host_line->text();
  QString port = m_port_line->text();
  //connect to the chosen server
  connectToServer(given_host,port);
}

void Widget3D::loadPaths(std::vector<QString> paths,std::vector<QString> names){


  m_actor_list->selectionModel()->clearSelection();
  m_actor_list->setSelectionMode(QAbstractItemView::NoSelection);

  m_mesh_options->setVisible(false);
  m_regions_box->setVisible(false);

  m_actor_list->setItemSelected(0,false);

  if(m_RenderView && m_server && m_server->isRemote()){

    //create source for eatch path
    for(int i=0;i< paths.size();++i){

      bool mesh_visible = true;

      if(m_paths_list.contains(paths.at(i))){

        mesh_visible = m_source_list.at(m_paths_list.indexOf(paths.at(i)))->getRepresentation(this->m_RenderView)->isVisible();

        //delete previous sources
        m_object_builder->destroy(m_source_list.at(m_paths_list.indexOf(paths.at(i))));
        QListWidgetItem * newItem = new QListWidgetItem(names.at(i));
        newItem->setIcon(QIcon(":/paraview_icons/pqEyeball16.png"));

        m_actor_list->removeItemWidget(m_actor_list->item(m_paths_list.indexOf(paths.at(i))));

        create_source(paths.at(i));

        m_source_list.replace(m_paths_list.indexOf(paths.at(i)),m_source);

      }else{
        m_paths_list.push_back(paths.at(i));
        create_source(paths.at(i));
        m_source_list.push_back(m_source);

        QListWidgetItem * newItem = new QListWidgetItem(names.at(i));
        newItem->setIcon(QIcon(":/paraview_icons/pqEyeball16.png"));
        m_actor_list->addItem(newItem);
      }

      //create a data representation in server side, for the render window with the input
      m_object_builder->createDataRepresentation(m_source->getOutputPort(0), this->m_RenderView);
      vtkSMSourceProxy::SafeDownCast(m_source->getProxy())->UpdatePipeline();

      m_source->getRepresentation(this->m_RenderView)->setVisible(mesh_visible);

    }

    //zoom to object
    reset_camera();

    //refresh widget
    m_RenderView->getWidget()->update();

    //make sure we update the view
//    this->m_RenderView->render();


    //show options
    m_mesh_options->setVisible(true);
    m_regions_box->setVisible(true);

  }else{
    if(!m_server->isRemote())
    {
      NLog::globalLog()->addError("You must be connected to a ParaView server.");
    }
  }

   m_actor_list->setSelectionMode(QAbstractItemView::SingleSelection);

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
    NLog::globalLog()->addError("Cannot load a file if no paraview server connection is set.");
  }
}

void Widget3D::show_hide_actor(QListWidgetItem * itemDuble){

  itemDuble->setSelected(true);

  bool visible = m_source_list.at(m_actor_list->row(itemDuble))->getRepresentation(this->m_RenderView)->isVisible();

  for(int i=0;i<m_actor_list->selectedItems().size();++i)
  {
      QListWidgetItem * item = m_actor_list->selectedItems().at(i);

      //if actor is visible
      if(visible){
        //Hide actor/region : Change icon and text color
        item->setTextColor(Qt::gray);
        item->setIcon(QIcon(":/paraview_icons/pqEyeballd16.png"));
      }else{
        //Show actor/region : Change icon and text color
        item->setTextColor(Qt::black);
        item->setIcon(QIcon(":/paraview_icons/pqEyeball16.png"));
      }

      //Set actor/region visibility
      m_source_list.at(m_actor_list->selectionModel()->selectedRows().at(i).row())
          ->getRepresentation(this->m_RenderView)->setVisible(!visible);

  }

  //be sure the mesh is updated
  m_RenderView->getWidget()->update();
  reset_camera();

}

void Widget3D::actor_changed(){
    if(m_actor_list->selectedItems().size() == 1){
      m_mesh_options->setEnabled(true);
      m_disp_adv_opt_button->setEnabled(true);

      //set the color selector representation (after it will directly apply changes to this representation)
      this->m_dataSet_selector->setRepresentation(m_source_list.at(m_actor_list->row(m_actor_list->selectedItems().at(0)))->getRepresentation(m_RenderView));
      this->m_dataSet_selector->reloadGUI();

      //set the style selector representation (after it will directly apply changes to this representation)
      this->m_mesh_style->setRepresentation(m_source_list.at(m_actor_list->row(m_actor_list->selectedItems().at(0)))->getRepresentation(m_RenderView));

      //get the current actor/region representation
      QPointer<pqPipelineRepresentation> representation = qobject_cast<pqPipelineRepresentation*>
                         (m_source_list.at(m_actor_list->row(m_actor_list->selectedItems().at(0)))
                          ->getRepresentation(m_RenderView));

      //Set the opacity spin box
      disconnect(m_spin_opacity,SIGNAL(valueChanged(double)),this,SLOT(opacityChange(double)));
      m_spin_opacity->setValue(representation->getOpacity());
      connect(m_spin_opacity,SIGNAL(valueChanged(double)),this,SLOT(opacityChange(double)));

      this->m_mesh_solid_color_set->setEnabled(this->m_dataSet_selector->getCurrentText() == "Solid Color");
      this->m_show_color_palette->setEnabled(this->m_dataSet_selector->getCurrentText() != "Solid Color");

    }else{
      m_mesh_options->setEnabled(false);
      m_disp_adv_opt_button->setEnabled(false);
      this->m_dataSet_selector->setRepresentation(0);
      this->m_mesh_style->setRepresentation(0);
      if(m_actor_list->selectedItems().size() > 1){
        //NLog::globalLog()->addError("One row selection maximum.");
      }
    }
  }

void Widget3D::set_solid_color(){

    pqPipelineRepresentation* repr = qobject_cast<pqPipelineRepresentation*>
        (m_source_list.at(m_actor_list->row(m_actor_list->selectedItems().at(0)))
         ->getRepresentation(m_RenderView));
    if (!repr)
    {
//      qCritical() << "No active representation.";
      return;
    }

    if (repr->getColorField() == pqPipelineRepresentation::solidColor())
    {
      // Get the color property.
      vtkSMProxy * proxy = repr->getProxy();
      vtkSMProperty * diffuse = proxy->GetProperty("DiffuseColor");
      vtkSMProperty * ambient = proxy->GetProperty("AmbientColor");
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

void Widget3D::opacityChange(double value){
  disconnect(m_spin_opacity,SIGNAL(valueChanged(double)),this,SLOT(opacityChange(double)));
  QPointer<pqPipelineRepresentation> representation = qobject_cast<pqPipelineRepresentation*>
                     (m_source_list.at(m_actor_list->row(m_actor_list->selectedItems().at(0)))
                      ->getRepresentation(m_RenderView));

    vtkSMProxy *proxy = representation->getProxy();
    vtkSMPropertyHelper(proxy, "Opacity").Set(value);
    proxy->UpdateVTKObjects();
  connect(m_spin_opacity,SIGNAL(valueChanged(double)),this,SLOT(opacityChange(double)));
  m_RenderView->getWidget()->update();
}

void Widget3D::enable_solide_color_button(pqVariableType type, const QString &name){
  m_mesh_solid_color_set->setEnabled(name == "Solid Color");
  this->m_show_color_palette->setEnabled(name != "Solid Color");
}

void Widget3D::show_camera_settings(){
  pqCameraDialog * dia = new pqCameraDialog();
  dia->setRenderModule(m_RenderView);
  dia->show();
}

void Widget3D::setCenterAxesVisibility(){

  //Set the center axe visibility
  m_RenderView->setCenterAxesVisibility(!m_RenderView->getCenterAxesVisibility());

  //make sure the view is updated
  m_RenderView->getWidget()->update();
}

void Widget3D::show_disp_adv_settings(){

  //create an object inspector
  pqDisplayProxyEditorWidget * obj_inspect;
  obj_inspect = new pqDisplayProxyEditorWidget();

  //create and set a scroll area for the widget
  QScrollArea* scr = new QScrollArea;
  scr->setWidgetResizable(true); //if not set, nothing appear
  scr->setMaximumHeight(600);
  scr->setMaximumWidth(420);
  scr->setFrameShape(QFrame::NoFrame);
  scr->setWidget(obj_inspect);

  //set object inspector representation => set actor/region
  obj_inspect->setRepresentation(m_source_list.at(m_actor_list->currentRow())
                                 ->getRepresentation(m_RenderView));

  //Dialog widget
  QPointer<QDialog> display_adv_setting_Dialog = new QDialog(this);

  //Apply change button
  QPushButton * valid = new QPushButton("Apply");
  connect(valid,SIGNAL(released()),obj_inspect,SLOT(update()));
  connect(valid,SIGNAL(released()),display_adv_setting_Dialog,SLOT(close()));

  // popup layout
  QPointer<QVBoxLayout> vertical_popup_layout = new QVBoxLayout();
  display_adv_setting_Dialog->setLayout(vertical_popup_layout);

  //add the scroll area to the dialog
  vertical_popup_layout->addWidget(scr);
  vertical_popup_layout->addWidget(valid);

  //set popup visible and modal
  display_adv_setting_Dialog->setModal(true);
  display_adv_setting_Dialog->show();
}

void Widget3D::show_gen_adv_settings(){

  //create a General Render View Option widget
  pqGlobalRenderViewOptions * obj_inspect;
  obj_inspect = new pqGlobalRenderViewOptions();
  obj_inspect->setMaximumHeight(400);

  //Set one of the options page to general settings
  obj_inspect->setPage(obj_inspect->getPageList().at(3));

  //Dialog widget
  QPointer<QDialog> OptionDialog = new QDialog(this);

  //Apply change button
  QPushButton * valid = new QPushButton("Apply");
  connect(valid,SIGNAL(released()),obj_inspect,SLOT(applyChanges()));
  connect(valid,SIGNAL(released()),OptionDialog,SLOT(close()));

  // popup layout
  QPointer<QVBoxLayout> vertical_popup_layout = new QVBoxLayout();
  OptionDialog->setLayout(vertical_popup_layout);

  //add the scroll area and the apply button to the dialog
  vertical_popup_layout->addWidget(obj_inspect);
  vertical_popup_layout->addWidget(valid);

  //set popup visiblem modal
  OptionDialog->setModal(true);
  OptionDialog->show();
}

void Widget3D::show_serv_adv_settings(){

  //create a General Render View Option widget
  pqGlobalRenderViewOptions * obj_inspect;
  obj_inspect = new pqGlobalRenderViewOptions();
  obj_inspect->setMaximumHeight(550);

  //Set one of the options page to server settings
  obj_inspect->setPage(obj_inspect->getPageList().at(1));

  //Dialog widget
  QPointer<QDialog> OptionDialog = new QDialog(this);

  //Apply change button
  QPushButton * valid = new QPushButton("Apply");
  connect(valid,SIGNAL(released()),obj_inspect,SLOT(applyChanges()));
  connect(valid,SIGNAL(released()),OptionDialog,SLOT(close()));

  // popup layout
  QPointer<QVBoxLayout> vertical_popup_layout = new QVBoxLayout();
  OptionDialog->setLayout(vertical_popup_layout);

  //add the scroll area and the apply button to the dialog
  vertical_popup_layout->addWidget(obj_inspect);
  vertical_popup_layout->addWidget(valid);

  //set popup visiblem modal
  OptionDialog->setModal(true);
  OptionDialog->show();
}

void Widget3D::enableRendering(bool enable){

  //disable auto rendering
  m_action_force_rendering->setEnabled(!enable);

  //get pqApplicationCore settings
  pqSettings * settings = pqApplicationCore::instance()->settings();

  //begin a setting group
  settings->beginGroup("renderModule");
  if(enable)
  {
    //Set the waiting delay before rendering to 0
    settings->setValue("NonInteractiveRenderDelay",0);
    //      settings->setValue("ImageReductionFactor", 10000);
    //      settings->setValue("StillRenderImageReductionFactor", 10000);
  }
  else
  {
    //Set the waiting delay before rendering to 1 houre
    settings->setValue("NonInteractiveRenderDelay",3600);
    //      settings->setValue("ImageReductionFactor", 10000);
    //      settings->setValue("StillRenderImageReductionFactor", 10000);
  }
  //end the setting group
  settings->endGroup();

  // loop through render views and apply new settings
  QList<pqRenderViewBase*> views =
      pqApplicationCore::instance()->getServerManagerModel()->
      findItems<pqRenderViewBase*>();

  foreach(pqRenderViewBase* view, views)
  {
    view->restoreSettings(true);
  }

  if(enable)
    //Force the mesh to render
    m_RenderView->forceRender();

}

void Widget3D::forceRendering(){
//  m_RenderView->forceRender();
  NLog::globalLog()->addMessage("Rendering in progress");
  connect(m_RenderView,SIGNAL(endRender()),this,SLOT(renderingProgress()));
  m_RenderView->render();
}

void Widget3D::setActorListSelectionMode(int mode){
  m_actor_list->setSelectionMode(QAbstractItemView::SelectionMode((m_list_selection->itemData(mode)).toInt()));
}

void Widget3D::showAdvOptions(bool showAdv){
  this->m_disp_adv_opt_button->setVisible(showAdv);
  this->m_gen_adv_opt_button->setVisible(showAdv);
  this->m_serv_adv_opt_button->setVisible(showAdv);
}

void Widget3D::renderingProgress(){
  NLog::globalLog()->addMessage("Rendering finished");
  disconnect(m_RenderView,SIGNAL(endRender()),this,SLOT(renderingProgress()));
}

////////////////////////////////////////////////////////////////////////////////

} // ParaViewTab
} // UI
} // CF
