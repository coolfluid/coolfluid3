// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_SignalManager_hpp
#define CF_GUI_Client_UI_SignalManager_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "Common/CPath.hpp"

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

    void showMenu(const QPoint & pos, const CF::Common::CPath & path,
                  const QList<CF::GUI::ClientCore::ActionInfo> & sigs);

  private slots:

    void actionTriggered();

    void actionHovered();

  private:

    QMenu * m_menu;

    CF::Common::CPath m_path;

    QMap<QAction *, CF::GUI::ClientCore::ActionInfo> m_signals;

    QMap<QAction *, bool> m_localStatus;

  }; // class SignalManager

  //////////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_UI_SignalManager_hpp
