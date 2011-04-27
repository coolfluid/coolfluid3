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
#include <QThread>
#include <QCheckBox>

// ParaView header
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
#include "vtkClientSocket.h"
#include "pqDisplayProxyEditorWidget.h"
#include "pqProgressManager.h"

// header
#include "UI/ParaViewTab/LibParaViewTab.hpp"

class QAction;

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

  /// Connect client to paraview server.
  /// @param host Ip or name of the paraview server.
  /// @param port Port used.
  void connectToServer(QString host,QString port);

  /// Load Actor and set their names into Actor List.
  /// @param paths Vector of server path.
  /// @param names Vector of names corresponding to paths.
  void loadPaths(std::vector<QString> paths,std::vector<QString> names);


private : //function

    /// Show the render of a file if the server connection is set and a the file exist
    void showRender();

    /// Create a view from server
    void createView();

    /// Create a reader for the defined PATH file on the server side ( .vtk or .ex2 )
    /// @param file_path Path of files in server side.
    /// @param file_name Name of file in server side.
    void openFile(QString file_path,QString file_name);

    /// Create a pqPipelineSource with the path.
    /// @param path Path of files in server side.
    void create_source(QString path);

private slots: //slots

  /// Ask connection options to user and try to connect to server host with port.
  void connectToServer();

  /// Disconnect from current paraview server.
  void disconnectFromServer();

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

  /// Show or Hide the current Region/Actor
  /// @param item Item corresponding to the Region/Actor.
  void show_hide_actor(QListWidgetItem * item);

  /// Called when the current Region/Actor change.
  /// @param item Item corresponding to the Region/Actor.
  void actor_changed(QListWidgetItem * item);

  /// Color Picker that change current Region/Actor color.
  void set_solid_color();

  /// Change the current Region/Actor opacity.
  /// @param value Value of opacity from 0 to 1.
  void opacityChange(double value);

  /// Change the current Region/Actor opacity.
  /// @param type
  /// @param name The current mash style name.
  void enable_solide_color_button(pqVariableType type, const QString &name);

  /// Display View Camera settings.
  void show_camera_settings();

  /// Show Hide center axes.
  void setCenterAxesVisibility();

  /// Show advanced settings for the current display.
  void show_disp_adv_settings();

  /// Show general settings.
  void show_gen_adv_settings();

  /// Show server settings.
  void show_serv_adv_settings();

  /// Force the mesh to render.
  void forceRendering();

  /// Set auto render
  /// @param enable Auto Render state.
  void enableRendering(bool enable);

  /// Set the Acor list selection mode.
  /// @param mode The chosen mode.
  void setActorListSelectionMode(int mode);

private: //data

  /// Initialising Application Core that manage all vtk and paraview Objects.
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

  /// Tool bar for server options


  /// Action to connect to the server
  QAction * m_action_connect;

  /// Action to disconnect from the server
  QAction * m_action_disconnect;

  /// Action to load a file
  QAction * m_action_load_file;

  /// Force Rendering button.
  QAction * m_action_force_rendering;

  /// Button that show color selector.
  QPointer<QPushButton> m_show_color_palette;

//  /// Button reload file.
//  QPointer<QPushButton> m_reload;

  /// ComboBox of dataRepresentation style
  QPointer<pqDisplayRepresentationWidget> m_mesh_style;

  /// Combobox of predefined rotation set
  QPointer<QComboBox> m_preDefined_rotation;

  /// Combobox of region selection type
  QPointer<QComboBox> m_list_selection;

  /// Source Pipeline is a flow from the source readed to show actor.
  QPointer<pqPipelineSource> m_source;

  /// render window input Pipeline.
  QPointer<pqPipelineSource> m_input;

  /// render window input Pipeline.
  QList<QPointer<pqPipelineSource> > m_source_list;

//  /// render window input Pipeline.
//  std::vector<QString> m_path_list;

  /// View where the render is shown.
  QPointer<pqRenderView> m_RenderView;

  /// Server Port LineEdit.
  QPointer<QLineEdit> m_port_line;

  /// Server Host LineEdit.
  QPointer<QLineEdit> m_host_line;

//  /// Server File Path LineEdit.
//  QPointer<QLineEdit> m_Path_File_line;

//  /// Server File Name LineEdit.
//  QPointer<QLineEdit> m_Name_line;

  /// Data set selector.
  QPointer<pqDisplayColorWidget> m_dataSet_selector;

  /// Color scale selector.
  QPointer<pqColorScaleEditor> m_scaleEdit;

//  /// Path of loaded file.
//  QString m_file_path;

//  /// Name of loaded file.
//  QString m_file_name;

//  /// Mesh group box options.
  QPointer<QGroupBox> m_mesh_options;

  /// Mesh group box options.
  QPointer<QGroupBox> m_regions_box;

//  /// Server group box options layout.
//  QPointer<QVBoxLayout> m_layout_server_options;

//  /// Camera group box options layout.
//  QPointer<QVBoxLayout> m_layout_camera_options;

//  /// Mesh group box options layout.
  QPointer<QHBoxLayout> m_layout_mesh_options;

  /// Regions group box layout.
  QPointer<QVBoxLayout> m_layout_regions_box;

  /// Regions list
  QPointer<QListWidget> m_actor_list;

  /// Temporary Representation
  QPointer<pqPipelineRepresentation> m_representation;

  /// Solid Color Button.
  QPointer<QPushButton> m_mesh_solid_color_set;

  /// Current Region/Actor opacity spin box.
  QPointer<QDoubleSpinBox> m_spin_opacity;

//  /// Show center axes button.
//  QPointer<QPushButton> m_show_axes_button;

//  /// Show camera dialog button.
//  QPointer<QPushButton> m_show_camera_settings_button;

  /// Display Advanced Option. (do not remove!)
  QPointer<QPushButton> m_disp_adv_opt_button;

  /// General Advanced Option. (do not remove!)
  QPointer<QPushButton> m_gen_adv_opt_button;

  /// Server Advanced Option. (do not remove!)
  QPointer<QPushButton> m_serv_adv_opt_button;

  /// Auto Render checkbox
  QPointer<QCheckBox> m_checkbox_enable_rendering;

  /// Progress Manager (not used)
  QPointer<pqProgressManager> progMgr;
};


////////////////////////////////////////////////////////////////////////////////

} // ParaViewTab
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////


#endif // CF_UI_ParaViewTab_Widget3D_hpp
