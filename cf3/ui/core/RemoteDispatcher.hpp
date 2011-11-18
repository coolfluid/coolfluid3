// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_RemoteDispatcher_hpp
#define cf3_ui_core_RemoteDispatcher_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "common/XML/SignalFrame.hpp"

#include "common/SignalDispatcher.hpp"

template<typename T> class QList;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common { class ConnectionManager; }

namespace ui {
namespace core {

class NRoot;

//////////////////////////////////////////////////////////////////////////////

class RemoteDispatcher :
    public QObject,
    public common::SignalDispatcher
{
  Q_OBJECT

public:

  RemoteDispatcher ( NRoot & root );

  void run ();

  virtual void dispatch_signal ( const std::string &target,
                                 const common::URI &receiver,
                                 common::SignalArgs &args );

  /// @name SIGNALS
  // @{

  void send_next_signal( common::SignalArgs & args );

  // @}

signals:

  void finished();

private:

  QList< common::SignalArgs > m_pendingSignals;

  std::string m_currentFrameId;

  Uint m_nextIndex;

  bool m_running;

  common::ConnectionManager * m_connectionManager;

  void send_signal( Uint index );

};

//////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_RemoteDispatcher_hpp
