// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_ParaView_Widget3D_hpp
#define CF_UI_ParaView_Widget3D_hpp

// Qt headers
#include <QVBoxLayout>
#include <QMainWindow>
#include <QPointer>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>

// ParaView header
#include "vtkSMSourceProxy.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqPluginManager.h"
#include "pqServer.h"
#include "pqStandardViewModules.h"
#include "pqRenderView.h"
#include "pqOutputPort.h"

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

private : //function

    /// Show the render of a file if the server connection is set and a the file exist
    void showRender();

    /// Create a view from server
    void createView();

    /// Add a filter
    void addFilter();


private slots: //slots

  /// Ask connection options and try to connect to a certain host & port
  void connectToServer();

  /// Ask connection options and try to connect to a certain host & port
  void disconnectFromServer();

  /// Create a reader for the defined PATH file on the server side ( .vtk or .ex2 )
  void openFile();

  /// Show a dialog window that ask for a path file
  void showLoadFileDialog();

  /// Show a dialog window that ask for an host and port
  void showConnectDialog();

  /// Change Style
  void changeStyle();

  /// Set the rotation center
  void set_rotation_center();

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
  QPointer<QVBoxLayout> m_layout_v;

  /// Layout of this widget
  QPointer<QHBoxLayout> m_layout_option;

  /// Button that show Server Connection dialog
  QPointer<QPushButton> m_connect_to_server_button;

  /// Button that show Server Load File dialog
  QPointer<QPushButton> m_load_file;

  /// Button that reset center of rotation
  QPointer<QPushButton> m_set_rotation_center;

  /// Button that show Server Load File dialog
  QPointer<QComboBox> m_style;

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

};


////////////////////////////////////////////////////////////////////////////////

} // ParaView
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////


#endif // CF_UI_ParaView_Widget3D_hpp
