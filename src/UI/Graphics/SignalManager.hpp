// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Graphics_SignalManager_hpp
#define CF_GUI_Graphics_SignalManager_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QMap>

#include "Common/XML/SignalFrame.hpp"

#include "UI/Core/CNode.hpp"

class QAction;
class QMenu;
class QMainWindow;
class QPoint;

template<typename T> class QList;

////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace UI {
namespace Graphics {

  //////////////////////////////////////////////////////////////////////////////

  class SignalManager : public QObject
  {
    Q_OBJECT

  public:

    SignalManager(QMainWindow *parent = 0);

    ~SignalManager();

    void showMenu(const QPoint & pos, Core::CNode::Ptr node,
                  const QList<CF::UI::Core::ActionInfo> & sigs);

  private slots:

    void actionTriggered();

    void actionHovered();

    void signalSignature(Common::SignalArgs & node);

    void dialogFinished(int result);

  private:

    QMenu * m_menu;

    Core::CNode::Ptr m_node;

    QMap<QAction *, Core::ActionInfo> m_signals;

    QMap<QAction *, bool> m_localStatus;

    QAction * m_currentAction;

    bool m_waitingForSignature;

    Common::XML::SignalFrame m_frame;

  }; // class SignalManager

  //////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Graphics_UI_SignalManager_hpp
