// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_ClientRoot_hpp
#define CF_GUI_Client_Core_ClientRoot_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QObject>

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

class ProcessingThread;

/// @brief Manages the client root node

/// This class ensures that the root node is available from anywhere, at
/// anytime.
/// @author Quentin Gasper.

class ClientCore_API ClientRoot : public QObject
{
  Q_OBJECT

public:

  static ClientRoot & instance();

  /// @brief Processes a signal from a string

  /// The receiver is called on the desired signal.
  /// @param signal Xml document with the signal description
  void processSignalString(const QString & signal);

  /// @brief Gives the root node.
  /// @return Returns the root node.
  inline NRoot::Ptr root() { return m_root; }

  /// @brief Gives the log node.
  /// @return Returns the log node.
  inline NLog::Ptr log() { return m_log; }

  /// @brief Gives the browser node.
  /// @return Returns the browser node
  inline NBrowser::Ptr browser() { return m_browser; }

  /// @brief Gives the tree node.
  /// @return Returns the tree node.
  inline NTree::Ptr tree() { return m_tree; }

  /// @brief Gives the core node.
  /// @return Returns the tree node.
  inline NCore::Ptr core() { return m_core; }

  /// @brief Gives the root UUID.
  /// @return Returns the root UUID.
  inline std::string getUUID() { return m_root->uuid(); }

  boost::shared_ptr<Common::XmlDoc> docFromPtr(const Common::XmlDoc * doc) const;

private slots:

  void processingFinished();

private:

  ClientRoot();

  NRoot::Ptr m_root;

  NLog::Ptr m_log;

  NBrowser::Ptr m_browser;

  NTree::Ptr m_tree;

  NCore::Ptr m_core;

  QMap<ProcessingThread*, boost::shared_ptr<Common::XmlDoc> > m_threads;

  QMap<const Common::XmlDoc *, boost::shared_ptr<Common::XmlDoc> > m_currentDocs;

}; // ClientRoot

////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_ClientRoot_hpp
