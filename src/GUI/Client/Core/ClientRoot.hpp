// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_ClientRoot_hpp
#define CF_GUI_Client_Core_ClientRoot_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QMap>

#include "GUI/Client/Core/NRoot.hpp"
#include "GUI/Network/ComponentNames.hpp"

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

  /// @brief Gives the root UUID.
  /// @return Returns the root UUID.
  inline std::string getUUID() { return m_root->uuid(); }

  Common::XML::XmlDoc::Ptr docFromPtr(const Common::XML::XmlDoc * doc) const;

  template<typename TYPE>
  typename TYPE::Ptr rootChild(const std::string & name) const
  {
    return m_root->root()->get_child<TYPE>(name);
  }

  NRoot::ConstPtr root() const { return m_root; }

  NRoot::Ptr root() { return m_root; }

private slots:

  void processingFinished();

private:

  NRoot::Ptr m_root;

  ClientRoot();

  QMap<ProcessingThread*, Common::XML::XmlDoc::Ptr > m_threads;

  QMap<const rapidxml::xml_node<char> *, Common::XML::XmlDoc::Ptr > m_currentDocs;

}; // ClientRoot

////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_ClientRoot_hpp
