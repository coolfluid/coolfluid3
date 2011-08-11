// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_ParaViewTab_Widget3D_hpp
#define CF_UI_ParaViewTab_Widget3D_hpp

// Qt headers
#include <QWidget>
#include <QPointer>

// ParaView header
#include "pqVariableType.h"

// header
#include "UI/ParaViewTab/LibParaViewTab.hpp"

////////////////////////////////////////////////////////////////////////////////

// forward declaration to avoid incuding files
// Qt class
class QAction;
class QVBoxLayout;
class QHBoxLayout;
class QLineEdit;
class QGroupBox;
class QListWidget;
class QListWidgetItem;
class QDoubleSpinBox;
class QCheckBox;
class QPushButton;
class QComboBox;
class QString;

// ParaView class
class pqRenderView;
class pqDisplayColorWidget;
class pqColorScaleEditor;
class pqDisplayRepresentationWidget;
class pqApplicationCore;
class pqObjectBuilder;
class pqServer;
class pqPluginManager;
class pqPipelineSource;
class pqPipelineRepresentation;
class pqProgressManager;


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
  void actor_changed();

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

  /// Show advanced options on advanced mode.
  /// @param showAdv Option visibility.
  void showAdvOptions(bool showAdv);

  void renderingProgress();

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

  /// ComboBox of dataRepresentation style
  QPointer<pqDisplayRepresentationWidget> m_mesh_style;

  /// Combobox of predefined rotation set
  QPointer<QComboBox> m_preDefined_rotation;

  /// Combobox of region selection type
  QPointer<QComboBox> m_list_selection;

  /// Source Pipeline is a flow from the source readed to show actor.
  QPointer<pqPipelineSource> m_source;

  /// render window input Pipeline.
  QList<QPointer<pqPipelineSource> > m_source_list;

  /// View where the render is shown.
  QPointer<pqRenderView> m_RenderView;

  /// Server Port LineEdit.
  QPointer<QLineEdit> m_port_line;

  /// Server Host LineEdit.
  QPointer<QLineEdit> m_host_line;

  /// Data set selector.
  QPointer<pqDisplayColorWidget> m_dataSet_selector;

  /// Color scale selector.
  QPointer<pqColorScaleEditor> m_scaleEdit;

  /// Mesh group box options.
  QPointer<QGroupBox> m_mesh_options;

  /// Mesh group box options.
  QPointer<QGroupBox> m_regions_box;

  /// Mesh group box options layout.
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

  /// Mesh path list
  QList<QString> m_paths_list;
};


////////////////////////////////////////////////////////////////////////////////

} // ParaViewTab
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////


#endif // CF_UI_ParaViewTab_Widget3D_hpp
