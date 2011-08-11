// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Server_ServerRoot_hpp
#define CF_GUI_Server_ServerRoot_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QMutex>

#include "Common/CRoot.hpp"
#include "Common/CJournal.hpp"

#include "UI/Server/ServerRoot.hpp"

#include "UI/Server/CCore.hpp"

class QMutex;
template<typename T> class QList;

//////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Common { namespace mpi { class CPEManager; } }
namespace Solver { class CPlotter; }

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

    Common::CRoot::Ptr root() { return m_root; }

    Common::CRoot::ConstPtr root() const { return m_root; }

    CCore::Ptr core() { return m_core; }

    CCore::ConstPtr core() const { return m_core; }

    Common::CJournal::Ptr journal() { return m_journal; }

    Common::CJournal::ConstPtr journal() const { return m_journal; }

    boost::shared_ptr<Common::mpi::CPEManager> manager() { return m_manager; }

    boost::shared_ptr<Common::mpi::CPEManager const> manager() const { return m_manager; }

    void process_signal(const std::string & target,
                       const Common::URI & receiver,
                       const std::string & clientid,
                       const std::string & frameid,
                       Common::SignalArgs & node);



    void listen_to_events();

    void signal_to_forward( Common::SignalArgs & args );

  public slots:

    void finished();

  private:

    ServerRoot();

    ~ServerRoot();

  private: // data

    boost::shared_ptr<Common::XML::XmlDoc> m_doc;

    ProcessingThread * m_thread;

    Common::CRoot::Ptr m_root;

    CCore::Ptr m_core;

    Common::CJournal::Ptr m_journal;

    QMutex m_mutex;

    Common::NotificationQueue * m_queue;

    Notifier * m_notifier;

    std::string m_current_client_id;

    std::string m_current_frame_id;

    QList< Common::URI > m_local_components;

    boost::shared_ptr<Common::mpi::CPEManager> m_manager;

    boost::shared_ptr<Solver::CPlotter> m_plotter;

  }; // class ServerRoot


} // Server
} // UI
} // CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Server_ServerRoot_hpp
