// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_TreeThread_hpp
#define CF_GUI_Client_Core_TreeThread_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QThread>

#include "GUI/Client/Core/NRoot.hpp"

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

class ClientCore_API TreeThread : public QThread
{
  Q_OBJECT

public:

  TreeThread(QObject * parent = nullptr);

  ~TreeThread();

  void run();

  /// @brief Gives the root UUID.
  /// @return Returns the root UUID.
  inline std::string getUUID() { return m_root->uuid(); }

  template<typename TYPE>
  typename TYPE::Ptr rootChild(const std::string & name) const
  {
    return m_root->root()->get_child_ptr(name)->as_ptr<TYPE>();
  }

  void setMutex(QMutex * mutex);

  NRoot::ConstPtr root() const { return m_root; }

  NRoot::Ptr root() { return m_root; }

  void newSignal(Common::XML::XmlDoc::Ptr doc);

private:

  NRoot::Ptr m_root;

  QMutex * m_mutex;

}; // TreeThread

////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_TreeThread_hpp
