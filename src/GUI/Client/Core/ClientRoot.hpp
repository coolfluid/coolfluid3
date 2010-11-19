// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_ClientRoot_hpp
#define CF_GUI_Client_Core_ClientRoot_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QDomDocument>

#include "Common/NonInstantiable.hpp"

#include "GUI/Network/ComponentNames.hpp"
#include "GUI/Client/Core/NBrowser.hpp"
#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/Core/NRoot.hpp"
#include "GUI/Client/Core/NTree.hpp"
#include "GUI/Client/Core/NCore.hpp"

#include "GUI/Client/Core/LibClientCore.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace GUI {
namespace ClientCore {

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Manages the client root node

  /// This class ensures that the root node is available from anywhere, at
  /// anytime.
  /// @author Quentin Gasper.

  class ClientCore_API ClientRoot :
      public CF::Common::NonInstantiable<ClientRoot>
  {
  public:

    /// @brief Gives the root node.

    /// @return Returns the root node.
    static NRoot::Ptr root();

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
    inline static NLog::Ptr log()
    {
      return root()->root()->access_component< NLog >(CLIENT_LOG_PATH);
    }

    /// @brief Gives the browser node.

    /// If the node does not exist yet, it is created.
    /// @return Returns the browser node
    inline static NBrowser::Ptr browser()
    {
      return root()->root()->access_component< NBrowser >(CLIENT_BROWSERS_PATH);
    }

    /// @brief Gives the tree node.

    /// If the node does not exist yet, it is created.
    /// @return Returns the tree node.
    inline static NTree::Ptr tree()
    {
      return root()->root()->access_component< NTree >(CLIENT_TREE_PATH);
    }

    /// @brief Gives the core node.

    /// If the node does not exist yet, it is created.
    /// @return Returns the tree node.
    inline static NCore::Ptr core()
    {
      return root()->root()->access_component< NCore >(CLIENT_CORE_PATH);
    }

    /// @brief Gives the root UUID.
    /// @return Returns the root UUID.
    inline static std::string getUUID()
    {
      return root()->uuid();
    }

  }; // class ClientRoot

  ////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_ClientRoot_hpp
