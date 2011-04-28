// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_ParaView_C3DVIEW_HPP
#define CF_UI_ParaView_C3DVIEW_HPP

// Qt header
#include <QObject>

// header
#include "UI/ParaView/LibParaView.hpp"

////////////////////////////////////////////////////////////////////////////////

// forward declaration to avoid incuding files
// Qt class
class QProcess;

//////////////////////////////////////////////////////////////////////////////



namespace CF {
namespace UI {
namespace ParaView {

//////////////////////////////////////////////////////////////////////////////

  /// @brief C3DView class.
  /// @author Wertz Gil
  class ParaView_API C3DView :
      public QObject,
      public Common::Component
{
    Q_OBJECT

public: // typedefs

  typedef boost::shared_ptr<C3DView> Ptr;
  typedef boost::shared_ptr<C3DView const> ConstPtr;

public:

  /// Constructor
  /// @param name Name of the node.
  C3DView(const std::string& name);

  /// Port setter.
  /// @param port The port on the paraview server must be launched.
  void setPort(QString port);

  /// Destructor
  virtual ~C3DView();

  /// Get the class name
  static std::string type_name () { return "C3DView"; }

  /// Launche a paraview server
  /// @param args
  void launch_pvserver( Common::SignalArgs & args );

  /// Dump a vtk or exodusII file.
  /// @param args
  void dump_file( Common::SignalArgs & args );

  /// Send paths and names of dumped file to the client
  /// @param args
  void send_server_info_to_client( Common::SignalArgs & args );

private slots:

    /// send Paraview server output messages to client
    void readyReadStandardOutput();

    /// send Paraview server output errors to client
    void readyReadStandardError();

private : //data
    /// Process of the paraview server.
    QProcess * m_pvserver;

    /// Port on the paraview server must be launched.
    QString m_port;
};

//////////////////////////////////////////////////////////////////////////////

} // ParaView
} // UI
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_UI_ParaView_C3DVIEW_HPP
