// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_ParaViewTab_Widget3D_hpp
#define CF_UI_ParaViewTab_Widget3D_hpp

// Qt headers
#include <QVBoxLayout>
#include <QPointer>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QList>
#include <QListWidgetItem>
#include <QDoubleSpinBox>

// ParaViewTab header
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

// header
#include "UI/ParaViewTab/LibParaViewTab.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ParaViewTab {

////////////////////////////////////////////////////////////////////////////////

/// @brief Show a mesh rendered on a ParaViewTab server.
/// @author Wertz Gil
class ParaViewTab_API Widget3D :
    public QWidget
{
  Q_OBJECT

public: //function

  /// Constructor
  /// @param parent Parent QWdiget.
  Widget3D(QWidget *parent = 0);

public slots://slots

  /// Call openFile to reload last file path
  //void reload();

  /// Connect client to ParaViewTab server.
  /// @param host Ip or name of the ParaViewTab server.
  /// @param port Port used.
  void connectToServer(QString host,QString port);

  void loadPaths(std::vector<QString> paths,std::vector<QString> names);

private : //function

  /// Show the render of a file if the server connection is set and a the file exist
  void showRender();

  /// Create a view from server
  void createView();

  /// Add a filter
  void addFilter();

  /// Create a reader for the defined PATH file on the server side ( .vtk or .ex2 )
  /// @param file_path Path of the file in server side.
  void openFile(QString file_path,QString file_name);

  void create_source(QString path);

private slots: //slots

  /// Ask connection options to user and try to connect to server host with port.
  void connectToServer();

  /// Disconnect from current ParaViewTab server.
  void disconnectFromServer();

  /// Call openFile with new entry path.
  void loadFile();

  /// Show a dialog window that ask for a path file.
  void showLoadFileDialog();

  /// Show a dialog window that ask for an host and port.
  void showConnectDialog();

  /// Set the rotation center, in center of the screen.
  void set_rotation_center();

  /// Set a selected rotation
  /// @param value The value of the rotation set.
  void set_rotation(int value);

  /// Take a screen shot of the current view
  void take_screen_shot();

  /// Reset camera
  void reset_camera();

  /// Show the color editor
  void show_color_editor();

  void show_hide_actor(QListWidgetItem * item);

  void actor_changed(QListWidgetItem * item);

  void setColor();

  void opacityChange(double value);

  void update_solide_color_button_state(pqVariableType type, const QString &name);

private: //data

  /// Initialising Application Core that manage all vtk and ParaViewTab Objects.
  QPointer<pqApplicationCore> m_core;

  /// Object Builder create Object on server side and a ghost on client side.
  QPointer<pqObjectBuilder> m_object_builder;

  /// Server connection
  QPointer<pqServer> m_server;

  /// Plugin manger that give acces to advenced object and view
  QPointer<pqPluginManager> m_plugin_manager;

  /// Main layout of this widget
  QPointer<QVBoxLayout> m_layout_v;
  QPointer<QVBoxLayout> m_layout_option;
  QPointer<QHBoxLayout> m_layout_h;

  /// Button that show Server Connection dialog
  QPointer<QPushButton> m_connect_to_server_button;

  /// Button that show Server Load File dialog
  QPointer<QPushButton> m_load_file;

  /// Button that reset center of rotation
  QPointer<QPushButton> m_set_rotation_center;

  /// Button take screen shot
  QPointer<QPushButton> m_screen_shot;

  /// Button reset rotation.
  QPointer<QPushButton> m_reset_camera;

  /// Button that show color selector.
  QPointer<QPushButton> m_show_color_palette;

  /// Button reload file.
  QPointer<QPushButton> m_reload;

  /// ComboBox of dataRepresentation style
  QPointer<pqDisplayRepresentationWidget> m_mesh_style;

  /// Combobox of predefined rotation set
  QPointer<QComboBox> m_preDefined_rotation;

  /// Source Pipeline is a flow from the source readed to show actor.
  QPointer<pqPipelineSource> m_source;

  /// render window input Pipeline.
  QPointer<pqPipelineSource> m_input;

  /// render window input Pipeline.
  QList<QPointer<pqPipelineSource> > m_source_list;

  /// render window input Pipeline.
  std::vector<QString> m_path_list;

  /// View where the render is shown.
  QPointer<pqRenderView> m_RenderView;

  /// Server Port LineEdit.
  QPointer<QLineEdit> m_port_line;

  /// Server Host LineEdit.
  QPointer<QLineEdit> m_host_line;

  /// Server File Path LineEdit.
  QPointer<QLineEdit> m_Path_File_line;

  /// Server File Name LineEdit.
  QPointer<QLineEdit> m_Name_line;

  /// Data set selector.
  QPointer<pqDisplayColorWidget> m_dataSet_selector;

  /// Color scale selector.
  QPointer<pqColorScaleEditor> m_scaleEdit;

  /// Path of loaded file.
  QString m_file_path;

  /// Name of loaded file.
  QString m_file_name;

  /// Server group box options.
  QPointer<QGroupBox> m_server_options;

  /// Camera group box options.
  QPointer<QGroupBox> m_camera_options;

  /// Mesh group box options.
  QPointer<QGroupBox> m_mesh_options;

  /// Mesh group box options.
  QPointer<QGroupBox> m_regions_box;

  /// Server group box options layout.
  QPointer<QVBoxLayout> m_layout_server_options;

  /// Camera group box options layout.
  QPointer<QVBoxLayout> m_layout_camera_options;

  /// Mesh group box options layout.
  QPointer<QHBoxLayout> m_layout_mesh_options;

  /// Regions group box layout.
  QPointer<QVBoxLayout> m_layout_regions_box;

  /// Regions list
  QPointer<QListWidget> m_actor_list;

  //QPointer<pqStandardColorButton> ColorActorColor;

  //QPointer<pqDisplayProxyEditor> dispProxyEdit;

  //QPointer<pqStandardColorLinkAdaptor> coloradapt;

  QPointer<pqPipelineRepresentation> representation;

  /// Button reload file.
  QPointer<QPushButton> m_mesh_solid_color_set;

  QPointer<QDoubleSpinBox> m_spin_opacity;

};


////////////////////////////////////////////////////////////////////////////////

} // ParaViewTab
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////


#endif // CF_UI_ParaViewTab_Widget3D_hpp
