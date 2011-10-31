// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Server_ServerRoot_hpp
#define cf3_GUI_Server_ServerRoot_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QMutex>

#include "common/Journal.hpp"
#include "common/NotificationQueue.hpp"

#include "UI/Server/CCore.hpp"

class QMutex;
template<typename T> class QList;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common { namespace PE { class Manager; } }
namespace solver { class CPlotter; }

namespace UI {
namespace Server {

  class ProcessingThread;
  class Notifier;

  ///////////////////////////////////////////////////////////////////////////

  class ServerRoot :
      public QObject
  {
    Q_OBJECT

  public:

    static ServerRoot & instance();

    common::Component::Ptr root() { return m_root; }

    common::Component::ConstPtr root() const { return m_root; }

    CCore::Ptr core() { return m_core; }

    CCore::ConstPtr core() const { return m_core; }

    common::Journal::Ptr journal() { return m_journal; }

    common::Journal::ConstPtr journal() const { return m_journal; }

    boost::shared_ptr<common::PE::Manager> manager() { return m_manager; }

    boost::shared_ptr<common::PE::Manager const> manager() const { return m_manager; }

    void process_signal(const std::string & target,
                       const common::URI & receiver,
                       const std::string & clientid,
                       const std::string & frameid,
                       common::SignalArgs & node);



    void listen_to_events();

    void signal_to_forward( common::SignalArgs & args );

  public slots:

    void finished();

  private:

    ServerRoot();

    ~ServerRoot();

  private: // data

    boost::shared_ptr<common::XML::XmlDoc> m_doc;

    ProcessingThread * m_thread;

    common::Component::Ptr m_root;

    CCore::Ptr m_core;

    common::Journal::Ptr m_journal;

    QMutex m_mutex;

    common::NotificationQueue * m_queue;

    Notifier * m_notifier;

    std::string m_current_client_id;

    std::string m_current_frame_id;

    QList< common::URI > m_local_components;

    boost::shared_ptr<common::PE::Manager> m_manager;

    boost::shared_ptr<solver::CPlotter> m_plotter;

  }; // class ServerRoot


} // Server
} // UI
} // cf3

/////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Server_ServerRoot_hpp
