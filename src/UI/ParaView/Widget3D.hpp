// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_ParaView_Widget3D_hpp
#define CF_UI_ParaView_Widget3D_hpp

// Qt headers
#include <QVBoxLayout>
#include <QPointer>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QGroupBox>

// ParaView header
#include "vtkSMSourceProxy.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqPluginManager.h"
#include "pqServer.h"
#include "pqStandardViewModules.h"
#include "pqRenderView.h"
#include "pqDisplayColorWidget.h"
#include "pqColorScaleEditor.h"
#include "pqPQLookupTableManager.h"
#include "pqDisplayRepresentationWidget.h"

// header
#include "UI/Graphics/LibGraphics.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ParaView {

////////////////////////////////////////////////////////////////////////////////

    /// @brief Show a mesh rendered on a server.
    /// @author Wertz Gil
class Graphics_API Widget3D : public QWidget
{
  Q_OBJECT

public: //function

  /// Constructor
  /// @param parent Parent QWdiget.
  Widget3D(QWidget *parent = 0);

public slots://slots

  /// Call openFile to reload last file path
  void reload();

  /// Connect client to paraview server and load a certain file
  /// @param host Ip or name of the paraview server.
  /// @param port Port used.
  /// @param path Path of the file in server side.
  void connectToServer(QString host,QString port, QString path);

private : //function

    /// Show the render of a file if the server connection is set and a the file exist
    void showRender();

    /// Create a view from server
    void createView();

    /// Add a filter
    void addFilter();

    /// Create a reader for the defined PATH file on the server side ( .vtk or .ex2 )
    /// @param file_path Path of the file in server side.
    void openFile(QString file_path);

    /// Connect client to paraview server.
    /// @param host Ip or name of the paraview server.
    /// @param port Port used.
    void connectToServer(QString host,QString port);

private slots: //slots

  /// Ask connection options to user and try to connect to server host with port.
  void connectToServer();

  /// Disconnect from current paraview server.
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

  /// View where the render is shown.
  QPointer<pqRenderView> m_RenderView;

  /// Server Port LineEdit.
  QPointer<QLineEdit> m_port_line;

  /// Server Host LineEdit.
  QPointer<QLineEdit> m_host_line;

  /// Server File Path LineEdit.
  QPointer<QLineEdit> m_Path_File_line;

  /// Data set selector.
  QPointer<pqDisplayColorWidget> m_dataSet_selector;

  /// Color scale selector.
  QPointer<pqColorScaleEditor> m_scaleEdit;

  /// Path of loaded file.
  QString m_file_path;

  /// Server group box options.
  QPointer<QGroupBox> m_server_options;

  /// Camera group box options.
  QPointer<QGroupBox> m_camera_options;

  /// Mesh group box options.
  QPointer<QGroupBox> m_mesh_options;

  /// Server group box options layout.
  QPointer<QVBoxLayout> m_layout_server_options;

  /// Camera group box options layout.
  QPointer<QVBoxLayout> m_layout_camera_options;

  /// Mesh group box options layout.
  QPointer<QHBoxLayout> m_layout_mesh_options;

};


////////////////////////////////////////////////////////////////////////////////

} // ParaView
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////


#endif // CF_UI_ParaView_Widget3D_hpp
