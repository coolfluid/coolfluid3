// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Graphics_SignalManager_hpp
#define cf3_GUI_Graphics_SignalManager_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QMap>

#include "common/XML/SignalFrame.hpp"

#include "UI/Core/CNode.hpp"

class QAction;
class QMenu;
class QMainWindow;
class QPoint;

template<typename T> class QList;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace UI {
namespace Graphics {

  //////////////////////////////////////////////////////////////////////////////

  class SignalManager : public QObject
  {
    Q_OBJECT

  public:

    SignalManager(QMainWindow *parent = 0);

    ~SignalManager();

    void show_menu(const QPoint & pos, Core::CNode::Ptr node,
                  const QList<cf3::UI::Core::ActionInfo> & sigs);

  private slots:

    void action_triggered();

    void action_hovered();

    void signal_signature(common::SignalArgs & node);

    void dialog_finished(int result);

  private:

    QMenu * m_menu;

    Core::CNode::Ptr m_node;

    QMap<QAction *, Core::ActionInfo> m_signals;

    QMap<QAction *, bool> m_local_status;

    QAction * m_current_action;

    bool m_waiting_for_signature;

    common::XML::SignalFrame m_frame;

  }; // class SignalManager

  //////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Graphics_UI_SignalManager_hpp
