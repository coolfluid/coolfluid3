// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Server_Notifier_hpp
#define cf3_ui_Server_Notifier_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QMap>

#include "common/NotificationQueue.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace Server {

  //////////////////////////////////////////////////////////////////////////////

  class Notifier : public QObject
  {
    Q_OBJECT

  public:

    Notifier(cf3::common::NotificationQueue * observedQueue, QObject * parent = 0);

    ~Notifier();

    void listenToEvent(const std::string & name, bool notifyOnce);

    void begin_notify();

    void newEvent(const std::string & name, const cf3::common::URI & raiserPath);

  signals:

    void eventOccured(const std::string & name, const cf3::common::URI & raiserPath);

  private:

    cf3::common::NotificationQueue * m_observedQueue;

    QMap<std::string, bool> m_onceNotifyingEvents;

  }; // class Notifier

  //////////////////////////////////////////////////////////////////////////////

} // Server
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Server_Notifier_hpp
