// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_StatusPanel_hpp
#define CF_GUI_Client_UI_StatusPanel_hpp

/////////////////////////////////////////////////////////////////////////////

#include <QTreeView>

#include "Common/CF.hpp"

#include "GUI/Client/UI/LibClientUI.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {

//namespace ClientCore { class StatusModel; }

namespace ClientUI {

/////////////////////////////////////////////////////////////////////////////

  class ClientUI_API StatusPanel : public QTreeView
  {
    Q_OBJECT

  public:

    StatusPanel(/*ClientCore::StatusModel*/ QAbstractItemModel * model, QWidget * parent = nullptr);

    ~StatusPanel();

  private slots:

    void subSystemAdded(const QModelIndex & index);

  private:

    /*ClientCore::StatusModel*/ QAbstractItemModel * m_model;

  }; // class StatusPanel

/////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_StatusPanel_h
