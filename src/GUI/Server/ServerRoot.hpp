// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Server_ServerRoot_hpp
#define CF_GUI_Server_ServerRoot_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QDomDocument>

#include "Common/CRoot.hpp"
#include "Common/CJournal.hpp"

#include "GUI/Server/ServerRoot.hpp"

#include "GUI/Server/CCore.hpp"

class QMutex;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Server {

  class ProcessingThread;
  class Notifier;

  ///////////////////////////////////////////////////////////////////////////

  class SignalCatcher : public QObject
  {
    Q_OBJECT

  public slots:

    void finished();
  };

  class ServerRoot :
      public boost::noncopyable,
      public CF::NonInstantiable<ServerRoot>
  {
  public:

    static CF::Common::CRoot::Ptr root();

    static void processSignal(const std::string & target,
                              const CF::Common::URI & receiver,
                              const std::string & clientid,
                              const std::string & frameid,
                              CF::Common::XmlNode & node,
                              boost::shared_ptr<CF::Common::XmlDoc> doc);

    static CCore::Ptr core();

    static CF::Common::CJournal::Ptr journal();

    static void listenToEvents();

  private:

    static boost::shared_ptr<CF::Common::XmlDoc> m_doc;

    static ProcessingThread * m_thread;

    static SignalCatcher * m_catcher;

    static QMutex m_mutex;

    static CF::Common::NotificationQueue * m_queue;

    static Notifier * m_notifier;

    friend void SignalCatcher::finished();

  }; // class ServerRoot


} // Server
} // GUI
} // CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Server_ServerRoot_hpp
