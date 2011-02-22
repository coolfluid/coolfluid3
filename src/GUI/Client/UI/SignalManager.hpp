// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_SignalManager_hpp
#define CF_GUI_Client_UI_SignalManager_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "GUI/Client/Core/CNode.hpp"

class QMenu;
class QMainWindow;

template<typename T> class QList;
template<typename T, typename V> class QMap;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

  //////////////////////////////////////////////////////////////////////////////

  class SignalManager : public QObject
  {
    Q_OBJECT

  public:

    SignalManager(QMainWindow *parent = 0);

    ~SignalManager();

    void showMenu(const QPoint & pos, ClientCore::CNode::Ptr node,
                  const QList<CF::GUI::ClientCore::ActionInfo> & sigs);

  private slots:

    void actionTriggered();

    void actionHovered();

    void signalSignature(Common::Signal::arg_t & node);

  private:

    QMenu * m_menu;

    ClientCore::CNode::Ptr m_node;

    QMap<QAction *, ClientCore::ActionInfo> m_signals;

    QMap<QAction *, bool> m_localStatus;

    QAction * m_currentAction;

    bool m_waitingForSignature;

  }; // class SignalManager

  //////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_UI_SignalManager_hpp
