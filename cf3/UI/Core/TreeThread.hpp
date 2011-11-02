// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_TreeThread_hpp
#define cf3_ui_core_TreeThread_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QThread>

#include "UI/Core/NRoot.hpp"

#include "UI/Core/LibCore.hpp"


//////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common { namespace XML { class XmlDoc; } }

namespace ui {
namespace core {

////////////////////////////////////////////////////////////////////////////

/// @brief Manages the client root node

/// This class ensures that the root node is available from anywhere, at
/// anytime.
/// @author Quentin Gasper.

class Core_API TreeThread : public QThread
{
  Q_OBJECT

public:

  TreeThread(QObject * parent = nullptr);

  ~TreeThread();

  void run();

  /// @brief Gives the root UUID.
  /// @return Returns the root UUID.
  inline std::string get_uuid() { return m_root->uuid(); }

  template<typename TYPE>
  typename TYPE::Ptr root_child(const std::string & name) const
  {
    return m_root->get_child_ptr("UI")->get_child_ptr(name)->as_ptr<TYPE>();
  }

  void set_mutex(QMutex * mutex);

  NRoot::ConstPtr root() const { return m_root; }

  NRoot::Ptr root() { return m_root; }

  void new_signal( boost::shared_ptr<common::XML::XmlDoc> doc);

private:

  NRoot::Ptr m_root;

  QMutex * m_mutex;

}; // TreeThread

////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_TreeThread_hpp
