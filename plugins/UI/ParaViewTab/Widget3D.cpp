// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// Qt headers
//#include <QHBoxLayout>
#include <QAction>
#include <QLabel>
#include <QDialog>
#include <QIntValidator>
#include <QFileDialog>
#include <QColorDialog>
#include <QDebug>
#include <QScrollArea>
#include <QToolBar>
#include <QToolButton>

// ParaView header
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
#include "vtkProcessModuleConnectionManager.h"
#include "vtksys/SystemTools.hxx"
#include "vtkClientSocket.h"
#include "vtkTimerLog.h"
#include "pqServerManagerModel.h"
#include "vtkServerConnection.h"
#include "pqCameraDialog.h"
#include "pqObjectInspectorWidget.h"
#include "pqGlobalRenderViewOptions.h"
#include "pqProgressManager.h"
#include "pqStatusBar.h"

#include "vtkTimerLog.h"
#include "vtkClientSocket.h"


// header

#include "UI/Graphics/NRemoteOpen.hpp"
#include "UI/Core/NLog.hpp"

#include "UI/ParaViewTab/N3DView.hpp"

#include "UI/ParaViewTab/Widget3D.hpp"

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
//  m_layout_server_options = new QVBoxLayout();
//  m_layout_camera_options = new QVBoxLayout();
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
  m_list_selection->addItem("Single Selection",1);
  m_list_selection->addItem("Contiguous Selection",4);
  m_list_selection->addItem("Extended Selection",3);
  m_list_selection->addItem("Multi Selection",2);
  m_list_selection->setEditable(false);

  tool_bar->addWidget(m_preDefined_rotation);

  tool_bar->addAction(QIcon(), "Axes Visibility", this, SLOT(setCenterAxesVisibility()));
  //m_show_axes_button = new QPushButton("Hide Axes");        // show axes button

  tool_bar->addAction(QIcon(), "Camera settings", this, SLOT(show_camera_settings()));
  //m_show_camera_settings_button =  new QPushButton("Show Camera settings");  // Show camera settings dialog button

  // advanced paraview options (not used for now)
  m_disp_adv_opt_button = new QPushButton("Disp. Adv.");
  m_disp_adv_opt_button->setEnabled(false);
  m_gen_adv_opt_button = new QPushButton("Gen. Adv.");
  m_serv_adv_opt_button = new QPushButton("Serv. Adv.");

  //Mesh Options
  m_show_color_palette = new QPushButton(QIcon(":/paraview_icons/pqScalarBar24.png"),"Show Color Palette",this);
  m_show_color_palette->setEnabled(false);

  m_mesh_style = new pqDisplayRepresentationWidget(this); //Combo box that handle styles

  m_dataSet_selector = new pqDisplayColorWidget(this); // the scale color selector and legend

  m_mesh_solid_color_set = new QPushButton("Set Solid Color"); // Set Solide Color button
  m_mesh_solid_color_set->setEnabled(false);

  //Opacity spinner
  m_spin_opacity = new QDoubleSpinBox(this);
  m_spin_opacity->setMaximum(1);
  m_spin_opacity->setSingleStep(0.1);
  m_spin_opacity->setMinimum(0);
  m_spin_opacity->setEnabled(false);

  //Regions list
  m_actor_list = new QListWidget(this);
//  m_actor_list->setSelectionMode(QAbstractItemView::MultiSelection);

  //Create "force render" button and "Auto Render" checkbox
  m_checkbox_enable_rendering = new QCheckBox("Auto Render");

  m_action_force_rendering = tool_bar->addAction(QIcon(), "Render", this, SLOT(forceRendering()));
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
  m_regions_box = new QGroupBox("Regions",this);
  m_regions_box->setVisible(false);

  // representation initialisation
//  m_representation = 0;


  /** Disposition Phase **/
  //Set GroupBox layout
  m_mesh_options->setLayout(m_layout_mesh_options);
  m_regions_box->setLayout(m_layout_regions_box);

  //Mesh layouts
  m_layout_mesh_options->addWidget(this->m_mesh_style);
  m_layout_mesh_options->addWidget(this->m_dataSet_selector);
  m_layout_mesh_options->addWidget(this->m_show_color_palette);
  m_layout_mesh_options->addWidget(this->m_mesh_solid_color_set);
  m_layout_mesh_options->addWidget(this->m_spin_opacity);

  //Option layouts
  m_layout_option->addWidget(this->m_regions_box);
  //m_layout_option->addWidget(this->m_disp_adv_opt_button);
  //m_layout_option->addWidget(this->m_gen_adv_opt_button);
  //m_layout_option->addWidget(this->m_serv_adv_opt_button);

  //Horizotal layouts
  m_layout_h->addLayout(this->m_layout_option);

  m_layout_h->setStretchFactor(m_layout_option, 0);

  //Main layouts
  m_layout_v->addWidget(tool_bar);
  m_layout_v->addLayout(m_layout_h);
  m_layout_v->addWidget(this->m_mesh_options);
  //m_layout_v->addWidget(status_bar);

  m_layout_v->setStretchFactor(tool_bar, 0);
  m_layout_v->setStretchFactor(m_layout_h, 10);
  m_layout_v->setStretchFactor(m_mesh_options, 0);

  //Actor layouts
  m_layout_regions_box->addWidget(this->m_actor_list);
  m_layout_regions_box->addWidget(this->m_list_selection);


  /** Connection Phase **/
  connect(m_actor_list,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(show_hide_actor(QListWidgetItem*)));
  connect(m_actor_list,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(actor_changed(QListWidgetItem*)));
  connect(m_preDefined_rotation,SIGNAL(activated(int)),this,SLOT(set_rotation(int)));
  connect(m_show_color_palette, SIGNAL(released()), this, SLOT(show_color_editor()));
  connect(m_mesh_solid_color_set,SIGNAL(released()),this,SLOT(set_solid_color()));
  connect(m_dataSet_selector,SIGNAL(variableChanged(pqVariableType, const QString)),this,SLOT(enable_solide_color_button(pqVariableType, const QString)));
  //connect(m_show_axes_button,SIGNAL(released()),this,SLOT(setCenterAxesVisibility()));
  //connect(m_show_camera_settings_button,SIGNAL(released()),this,SLOT(show_camera_settings()));
  connect(m_disp_adv_opt_button,SIGNAL(released()),this,SLOT(show_disp_adv_settings()));
  connect(m_gen_adv_opt_button,SIGNAL(released()),this,SLOT(show_gen_adv_settings()));
  connect(m_serv_adv_opt_button,SIGNAL(released()),this,SLOT(show_serv_adv_settings()));
  connect(m_checkbox_enable_rendering,SIGNAL(toggled(bool)),this,SLOT(enableRendering(bool)));
  connect(m_list_selection,SIGNAL(activated(int)),this,SLOT(setActorListSelectionMode(int)));
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

/* Not used any more */
void Widget3D::showRender()
{
  if(m_source){

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
      m_layout_h->setStretchFactor(this->m_RenderView->getWidget(), 10);
      //Set a maximum size
      //        this->m_RenderView->getWidget()->setMaximumHeight(400);
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

  //hide mesh and region options
  m_mesh_options->setVisible(false);
  m_regions_box->setVisible(false);

  //change disconnect button to connect button
  m_action_connect->setEnabled(true);
  m_action_disconnect->setEnabled(false);

  //show user info
  NLog::globalLog()->addMessage("Disconnected from paraview server");

  //disconnected, cannot load or reload file while not connected
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

  /*
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
*/
  //or
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
  if(m_RenderView && m_input){
    //get the view representation
    pqDataRepresentation* repr = m_source_list.at(m_actor_list->currentRow())->getRepresentation(m_RenderView);

    if(repr && m_RenderView->getWidget() && m_server && m_server->isRemote() && m_source)
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

  if(m_RenderView && m_server && m_server->isRemote()){

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

    //show options
    m_mesh_options->setVisible(true);
    m_regions_box->setVisible(true);
  }else{
    if(!m_server->isRemote())
    {
      NLog::globalLog()->addError("You must be connected to a ParaView server.");
    }
  }
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
  bool visible = m_RenderView->getRepresentation(m_actor_list->row(itemDuble))->isVisible();

  for(int i=0;i<m_actor_list->selectedItems().size();++i)
  {

      QListWidgetItem * item = m_actor_list->selectedItems().at(i);

      int index = m_actor_list->row(item);

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
      m_RenderView->getRepresentation(index)->setVisible(!visible);

  }

  //be sure the mesh is updated
  m_RenderView->getWidget()->update();

}

void Widget3D::actor_changed(QListWidgetItem * item){
    if(m_actor_list->selectedItems().size() == 1){
      //set the color selector representation (after it will directly apply changes to this representation)
      this->m_dataSet_selector->setEnabled(true);
      this->m_dataSet_selector->setRepresentation(m_source_list.at(m_actor_list->row(m_actor_list->selectedItems().at(0)))->getRepresentation(m_RenderView));
      this->m_dataSet_selector->reloadGUI();

      //set the style selector representation (after it will directly apply changes to this representation)
      this->m_mesh_style->setEnabled(true);
      this->m_mesh_style->setRepresentation(m_source_list.at(m_actor_list->row(m_actor_list->selectedItems().at(0)))->getRepresentation(m_RenderView));

      //get the current actor/region representation
      QPointer<pqPipelineRepresentation> representation = qobject_cast<pqPipelineRepresentation*>
                         (m_source_list.at(m_actor_list->row(m_actor_list->selectedItems().at(0)))
                          ->getRepresentation(m_RenderView));

      //Set the opacity spin box
      disconnect(m_spin_opacity,SIGNAL(valueChanged(double)),this,SLOT(opacityChange(double)));
      m_spin_opacity->setEnabled(true);
      m_spin_opacity->setValue(representation->getOpacity());
      connect(m_spin_opacity,SIGNAL(valueChanged(double)),this,SLOT(opacityChange(double)));

      //enable advenced Region options button
      m_disp_adv_opt_button->setEnabled(m_actor_list->selectedItems().size());
//      m_show_color_palette->setEnabled(m_actor_list->selectedItems().size());
//      m_mesh_solid_color_set->setEnabled(m_actor_list->selectedItems().size());

      this->m_mesh_solid_color_set->setEnabled(this->m_dataSet_selector->getCurrentText() == "Solid Color");
      this->m_show_color_palette->setEnabled(this->m_dataSet_selector->getCurrentText() != "Solid Color");

//      m_mesh_solid_color_set->setEnabled(m_dataSet_selector->getCurrentText() == "Solid Color");
//      if(m_dataSet_selector->getCurrentText() != "Solid Color"){
//        this->m_dataSet_selector->setRepresentation(m_source_list.at(m_actor_list->row(m_actor_list->selectedItems().at(0)))->getRepresentation(m_RenderView));
//      }else{
//        this->m_dataSet_selector->setRepresentation(0);
//      }

    }else{
      this->m_dataSet_selector->setRepresentation(0);
      this->m_dataSet_selector->setEnabled(false);
      this->m_mesh_style->setRepresentation(0);
      this->m_mesh_style->setEnabled(false);
      qDebug() << m_dataSet_selector->getCurrentText();
      m_disp_adv_opt_button->setEnabled(false);
      m_show_color_palette->setEnabled(false);
      m_mesh_solid_color_set->setEnabled(false);
      m_spin_opacity->setEnabled(false);
      if(m_actor_list->selectedItems().size() > 1){
        //NLog::globalLog()->addError("One row selection maximum.");
      }
    }
  }

void Widget3D::set_solid_color(){

//  if(m_representation){ // if actor/region selected
    pqPipelineRepresentation* repr = qobject_cast<pqPipelineRepresentation*>
        (m_source_list.at(m_actor_list->row(m_actor_list->selectedItems().at(0)))
         ->getRepresentation(m_RenderView));
    if (!repr)
    {
      qCritical() << "No active representation.";
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
//  }
}

void Widget3D::opacityChange(double value){
  disconnect(m_spin_opacity,SIGNAL(valueChanged(double)),this,SLOT(opacityChange(double)));
  //if(m_representation){
  QPointer<pqPipelineRepresentation> representation = qobject_cast<pqPipelineRepresentation*>
                     (m_source_list.at(m_actor_list->row(m_actor_list->selectedItems().at(0)))
                      ->getRepresentation(m_RenderView));

    vtkSMProxy *proxy = representation->getProxy();
    vtkSMPropertyHelper(proxy, "Opacity").Set(value);
    proxy->UpdateVTKObjects();
  //}
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
  scr->setMaximumHeight(300);
  scr->setFrameShape(QFrame::NoFrame);
  scr->setWidget(obj_inspect);

  //set object inspector representation => set actor/region
  obj_inspect->setRepresentation(m_source_list.at(m_actor_list->currentRow())
                                 ->getRepresentation(m_RenderView));

  //Dialog widget
  QPointer<QDialog> display_adv_setting_Dialog = new QDialog(this);

  // popup layout
  QPointer<QVBoxLayout> vertical_popup_layout = new QVBoxLayout();
  display_adv_setting_Dialog->setLayout(vertical_popup_layout);

  //add the scroll area to the dialog
  vertical_popup_layout->addWidget(scr);

  //set popup visible and modal
  display_adv_setting_Dialog->resize(600,400);
  display_adv_setting_Dialog->setModal(true);
  display_adv_setting_Dialog->show();
}

void Widget3D::show_gen_adv_settings(){

  //create a General Render View Option widget
  pqGlobalRenderViewOptions * obj_inspect;
  obj_inspect = new pqGlobalRenderViewOptions();

  //create and set a scroll area for the widget
  QScrollArea* scr = new QScrollArea;
  scr->setWidgetResizable(true);
  scr->setMaximumHeight(300);
  scr->setFrameShape(QFrame::NoFrame);
  scr->setWidget(obj_inspect);

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
  vertical_popup_layout->addWidget(scr);
  vertical_popup_layout->addWidget(valid);

  //set popup visiblem modal
  OptionDialog->resize(600,400);
  OptionDialog->setModal(true);
  OptionDialog->show();
}

void Widget3D::show_serv_adv_settings(){

  //create a General Render View Option widget
  pqGlobalRenderViewOptions * obj_inspect;
  obj_inspect = new pqGlobalRenderViewOptions();

  //create and set a scroll area for the widget
  QScrollArea* scr = new QScrollArea;
  scr->setWidgetResizable(true);
  scr->setMaximumHeight(300);
  scr->setFrameShape(QFrame::NoFrame);
  scr->setWidget(obj_inspect);

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
  vertical_popup_layout->addWidget(scr);
  vertical_popup_layout->addWidget(valid);

  //set popup visiblem modal
  OptionDialog->resize(600,400);
  OptionDialog->setModal(true);
  OptionDialog->show();
}

void Widget3D::enableRendering(bool enable){

  //disable auto rendering
  m_action_force_rendering->setEnabled(!enable);
  //    m_force_rendering->setEnabled(!enable);

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
  m_RenderView->forceRender();
}

void Widget3D::setActorListSelectionMode(int mode){
  m_actor_list->setSelectionMode(QAbstractItemView::SelectionMode((m_list_selection->itemData(mode)).toInt()));
}

//settings->setValue("ImageReductionFactor", 1);
//settings->setValue("StillRenderImageReductionFactor", 1);

////////////////////////////////////////////////////////////////////////////////

} // ParaViewTab
} // UI
} // CF
