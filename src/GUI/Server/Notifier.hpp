// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Server_Notifier_hpp
#define CF_GUI_Server_Notifier_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QMap>

#include "Common/NotificationQueue.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Server {

  //////////////////////////////////////////////////////////////////////////////

  class Notifier : public QObject
  {
    Q_OBJECT

  public:

    Notifier(CF::Common::NotificationQueue * observedQueue, QObject * parent = 0);

    ~Notifier();

    void listenToEvent(const std::string & name, bool notifyOnce);

    void begin_notify();

    void newEvent(const std::string & name, const CF::Common::URI & raiserPath);

  signals:

    void eventOccured(const std::string & name, const CF::Common::URI & raiserPath);

  private:

    CF::Common::NotificationQueue * m_observedQueue;

    QMap<std::string, bool> m_onceNotifyingEvents;

  }; // class Notifier

  //////////////////////////////////////////////////////////////////////////////

} // Server
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Server_Notifier_hpp
