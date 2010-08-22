#ifndef CF_GUI_Server_ServerRoot_hpp
#define CF_GUI_Server_ServerRoot_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QDomDocument>

#include "Common/CRoot.hpp"
#include "Common/NonInstantiable.hpp"

#include "GUI/Server/ServerRoot.hpp"

#include "GUI/Server/CCore.hpp"

class QMutex;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Server {

  class ProcessingThread;

  ///////////////////////////////////////////////////////////////////////////

  class SignalCatcher : public QObject
  {
    Q_OBJECT

  public slots:

    void finished();
  };

  class ServerRoot :
      public boost::noncopyable,
      public CF::Common::NonInstantiable<ServerRoot>
  {
  public:

    static CF::Common::CRoot::Ptr & getRoot();

    static void processSignal(const std::string & target,
                              const CF::Common::CPath & receiver,
                              const std::string & clientid,
                              const std::string & frameid,
                              CF::Common::XmlNode & node,
                              boost::shared_ptr<CF::Common::XmlDoc> doc);

    static CCore::Ptr getCore();

  private:

    static boost::shared_ptr<CF::Common::XmlDoc> m_doc;

    static ProcessingThread * m_thread;

    static SignalCatcher * m_catcher;

    static QMutex m_mutex;

    friend void SignalCatcher::finished();

  }; // class ServerRoot


} // namespace Server
} // namespace GUI
} // namespace CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Server_ServerRoot_hpp
