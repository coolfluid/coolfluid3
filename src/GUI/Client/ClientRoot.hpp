#ifndef CF_GUI_Client_ClientRoot_hpp
#define CF_GUI_Client_ClientRoot_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QDomDocument>

#include "Common/CRoot.hpp"
#include "Common/NonInstantiable.hpp"

#include "GUI/Network/ComponentNames.hpp"
#include "GUI/Client/CBrowser.hpp"
#include "GUI/Client/CLog.hpp"
#include "GUI/Client/CTree.hpp"

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
    static CF::Common::CRoot::Ptr getRoot();

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
    inline static CLog::Ptr getLog()
    {
      return getRoot()->access_component< CLog >(CLIENT_LOG_PATH);
    }

    /// @brief Gives the browser node.

    /// If the node does not exist yet, it is created.
    /// @return Returns the browser node
    inline static CBrowser::Ptr getBrowser()
    {
      return getRoot()->access_component< CBrowser >(CLIENT_BROWSERS_PATH);
    }

    /// @brief Gives the tree node.

    /// If the node does not exist yet, it is created.
    /// @return Returns the tree node.
    inline static CTree::Ptr getTree()
    {
      return getRoot()->access_component< CTree >(CLIENT_TREE_PATH);
    }

  }; // class ClientRoot

  ////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_ClientRoot_hpp
