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

  /// Light Constructor
  /// @param parent Parent QWdiget.
  Widget3D(QWidget *parent = 0);

public slots://slots

  /// Call openFile to reload existing path
  void reload();

  /// Ask the client to connect to paraview server and qsk to load a certain file
  void connectToServer(QString host,QString port, QString path);


private : //function

    /// Show the render of a file if the server connection is set and a the file exist
    void showRender();

    /// Create a view from server
    void createView();

    /// Add a filter
    void addFilter();

    /// Create a reader for the defined PATH file on the server side ( .vtk or .ex2 )
    void openFile(QString file_path);

    /// Ask connection options and try to connect to a certain host & port
    void connectToServer(QString host,QString port);


private slots: //slots

  /// Ask connection options and try to connect to a certain host & port
  void connectToServer();

  /// Ask connection options and try to connect to a certain host & port
  void disconnectFromServer();

  /// Call openFile with new entry path
  void loadFile();

  /// Show a dialog window that ask for a path file
  void showLoadFileDialog();

  /// Show a dialog window that ask for an host and port
  void showConnectDialog();

  /// Set the rotation center
  void set_rotation_center();

  /// Set the current rotation
  void set_rotation(int value);

  /// Take a screen shot of the current view
  void take_screen_shot();

  /// reset the camera
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

  /// Layout of this widget
  QPointer<QHBoxLayout> m_layout_h;

  /// Layout of this widget
  QPointer<QVBoxLayout> m_layout_v;

  /// Layout of this widget
  QPointer<QVBoxLayout> m_layout_option;

  /// Button that show Server Connection dialog
  QPointer<QPushButton> m_connect_to_server_button;

  /// Button that show Server Load File dialog
  QPointer<QPushButton> m_load_file;

  /// Button that reset center of rotation
  QPointer<QPushButton> m_set_rotation_center;

  /// Button make a screen shot
  QPointer<QPushButton> m_screen_shot;

  /// Button make a screen shot
  QPointer<QPushButton> m_reset_camera;

  /// Button make a screen shot
  QPointer<QPushButton> m_show_color_palet;

  /// Button make a screen shot
  QPointer<QPushButton> m_reload;

  /// ComboBox of dataRepresentation style
  //QPointer<QComboBox> m_style;
  QPointer<pqDisplayRepresentationWidget> m_style;

  /// Combobox of predefined rotation set
  QPointer<QComboBox> m_preDefined_rotation;

  /// Source Pipeline is a flow from the source readed to show actor.
  QPointer<pqPipelineSource> m_source;

  /// render window input Pipeline.
  QPointer<pqPipelineSource> m_input;

  /// View where the render is shown
  QPointer<pqRenderView> m_RenderView;

  /// Server Port LineEdit
  QPointer<QLineEdit> m_port_line;

  /// Server Host LineEdit
  QPointer<QLineEdit> m_host_line;

  /// Server File Path LineEdit
  QPointer<QLineEdit> m_Path_File_line;

  pqDisplayColorWidget * m_color;

  //pqDisplayProxyEditor * m_disp_pan;

  //pqCameraDialog * camDia;

  pqColorScaleEditor * scaleEdit;

  pqLookupTableManager* lutManager;

  QString m_file_path;

  QGroupBox * server_options;
  QGroupBox * camera_options;
  QGroupBox * mesh_options;

  QPointer<QVBoxLayout> m_layout_server_options;
  QPointer<QVBoxLayout> m_layout_camera_options;
  QPointer<QHBoxLayout> m_layout_mesh_options;

};


////////////////////////////////////////////////////////////////////////////////

} // ParaView
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////


#endif // CF_UI_ParaView_Widget3D_hpp
