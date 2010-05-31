#ifndef CF_GUI_Server_ServerRoot_hpp
#define CF_GUI_Server_ServerRoot_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QDomDocument>

#include "Common/CRoot.hpp"
#include "Common/NonInstantiable.hpp"

#include "GUI/Server/ServerRoot.hpp"

#include "GUI/Server/CCore.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Server {

  ///////////////////////////////////////////////////////////////////////////

  class ServerRoot :
      public boost::noncopyable,
      public CF::Common::NonInstantiable<ServerRoot>
  {
  public:

    static CF::Common::CRoot::Ptr & getRoot();

    static void processSignal(const QDomDocument & signal);

    static CCore::Ptr getCore();

  }; // class ServerRoot


} // namespace Server
} // namespace GUI
} // namespace CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Server_ServerRoot_hpp
