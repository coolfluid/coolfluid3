// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_TreeThread_hpp
#define cf3_ui_core_TreeThread_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QThread>

#include "ui/core/NRoot.hpp"

#include "ui/core/LibCore.hpp"
#include <qmutex.h>


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

  /// @brief Gives the root UuiD.
  /// @return Returns the root UuiD.
  inline const common::UUCount& get_uuid() { return m_root->uuid(); }

  template<typename TYPE>
  Handle< TYPE > root_child(const std::string & name) const
  {
    return Handle<TYPE>(root()->get_child("UI")->get_child(name));
  }

  Handle< NRoot > root() const
  {
    QMutexLocker locker(m_mutex);
    cf3_assert(isRunning());
    cf3_assert(is_not_null(m_root));
    return Handle<NRoot>(m_root);
  }

  Handle< NRoot > root()
  {
    QMutexLocker locker(m_mutex);
    cf3_assert(isRunning());
    cf3_assert(is_not_null(m_root));
    return Handle<NRoot>(m_root);
  }

  void new_signal( boost::shared_ptr<common::XML::XmlDoc> doc);

private:

  boost::shared_ptr< NRoot > m_root;

  QMutex * m_mutex;

}; // TreeThread

////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_TreeThread_hpp
