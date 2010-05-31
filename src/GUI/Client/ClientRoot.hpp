#ifndef CF_GUI_Client_ClientRoot_hpp
#define CF_GUI_Client_ClientRoot_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QDomDocument>

#include "Common/CRoot.hpp"
#include "Common/NonInstantiable.hpp"

#include "GUI/Client/CBrowser.hpp"
#include "GUI/Client/CLog.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  ////////////////////////////////////////////////////////////////////////////

  class ClientRoot :
      public boost::noncopyable,
      public CF::Common::NonInstantiable<ClientRoot>
  {
    public:

      static CF::Common::CRoot::Ptr getRoot();

      static void processSignal(const QDomDocument & signal);

      static void processSignalString(const QString & signal);

      static CLog::Ptr getLog();

      static CBrowser::Ptr getBrowser();

  }; // class ClientRoot

  ////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_ClientRoot_hpp
