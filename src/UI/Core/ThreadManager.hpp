// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Core_ThreadManager_hpp
#define CF_GUI_Core_ThreadManager_hpp

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////////

class TreeThread;
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
  TreeThread & tree();

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

  TreeThread * m_treeThread;
};

////////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Core_ThreadManager_hpp
