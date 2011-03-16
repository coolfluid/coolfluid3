// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_ThreadManager_hpp
#define CF_GUI_Client_Core_ThreadManager_hpp

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////////

class ClientRoot;
class NetworkThread;

/// Class the manages the client threads.
/// This class is a singleton and threads it provides should never be deleted
/// manually.
/// @note Since this class is located in the Core part of the client application,
/// it does not provide access to the GUI thread. This thread can be accessed
/// by calling @c qApp->thread() after the @c QApplication object has been
/// initialized.
/// @author Quentin Gasper.
class ThreadManager
{
public:

  /// @return Returns the unique instance of this class.
  static ThreadManager & instance();

  /// @return Returns a reference to the network thread.
  NetworkThread & network();

  /// @return Returns a reference to the tree thread.
  ClientRoot & tree();

private: // functions

  /// Constructor.
  /// Builds all manages threads.
  ThreadManager();

  /// Destructor.
  /// All running threads are exited before they are destroyed.
  ~ThreadManager();

private: // data

  /// The network thread.
  NetworkThread * m_networkThread;

  ClientRoot * m_treeThread;
};

////////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_ThreadManager_hpp
