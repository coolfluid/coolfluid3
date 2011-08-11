// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_Core_RemoteDispatcher_hpp
#define CF_UI_Core_RemoteDispatcher_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "Common/XML/SignalFrame.hpp"

#include "Common/SignalDispatcher.hpp"

template<typename T> class QList;

//////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Common { class ConnectionManager; }

namespace UI {
namespace Core {

class NRoot;

//////////////////////////////////////////////////////////////////////////////

class RemoteDispatcher :
    public QObject,
    public Common::SignalDispatcher
{
  Q_OBJECT

public:

  RemoteDispatcher ( NRoot & root );

  void run ();

  virtual void dispatch_signal ( const std::string &target,
                                 const Common::URI &receiver,
                                 Common::SignalArgs &args );

  /// @name SIGNALS
  // @{

  void send_next_signal( Common::SignalArgs & args );

  // @}

signals:

  void finished();

private:

  QList< Common::SignalArgs > m_pendingSignals;

  std::string m_currentFrameId;

  Uint m_nextIndex;

  bool m_running;

  Common::ConnectionManager * m_connectionManager;

  void send_signal( Uint index );

};

//////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_UI_Core_RemoteDispatcher_hpp
