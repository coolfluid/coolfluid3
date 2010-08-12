#ifndef CF_GUI_Client_ClientRoot_hpp
#define CF_GUI_Client_ClientRoot_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QDomDocument>

#include "Common/NonInstantiable.hpp"

#include "GUI/Network/ComponentNames.hpp"
#include "GUI/Client/NBrowser.hpp"
#include "GUI/Client/NLog.hpp"
#include "GUI/Client/NRoot.hpp"
#include "GUI/Client/NTree.hpp"
#include "GUI/Client/NCore.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace GUI {
namespace Client {

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Manages the client root node

  /// This class ensures that the root node is available from anywhere, at
  /// anytime.
  class ClientRoot :
      public CF::Common::NonInstantiable<ClientRoot>
  {
  public:

    /// @brief Gives the root node.

    /// @return Returns the root node.
    static NRoot::Ptr getRoot();

    /// @brief Processes a signal from an Xml document

    /// The receiver is called on the desired signal.
    /// @param signal Xml document with the signal description
    static void processSignal(const QDomDocument & signal);

    /// @brief Processes a signal from a string

    /// The receiver is called on the desired signal.
    /// @param signal Xml document with the signal description
    static void processSignalString(const QString & signal);

    /// @brief Gives the log node.

    /// If the node does not exist yet, it is created.
    /// @return Returns the log node.
    inline static NLog::Ptr getLog()
    {
      return getRoot()->root()->access_component< NLog >(CLIENT_LOG_PATH);
    }

    /// @brief Gives the browser node.

    /// If the node does not exist yet, it is created.
    /// @return Returns the browser node
    inline static NBrowser::Ptr getBrowser()
    {
      return getRoot()->root()->access_component< NBrowser >(CLIENT_BROWSERS_PATH);
    }

    /// @brief Gives the tree node.

    /// If the node does not exist yet, it is created.
    /// @return Returns the tree node.
    inline static NTree::Ptr getTree()
    {
      return getRoot()->root()->access_component< NTree >(CLIENT_TREE_PATH);
    }

    /// @brief Gives the core node.

    /// If the node does not exist yet, it is created.
    /// @return Returns the tree node.
    inline static NCore::Ptr getCore()
    {
      return getRoot()->root()->access_component< NCore >(CLIENT_CORE_PATH);
    }

    /// @brief Gives the root UUID.
    /// @return Returns the root UUID.
    inline static std::string getUUID()
    {
      return getRoot()->getUUID();
    }

  }; // class ClientRoot

  ////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_ClientRoot_hpp
