// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_StatusPanel_hpp
#define CF_GUI_Client_StatusPanel_hpp

/////////////////////////////////////////////////////////////////////////////

#include <QTreeView>

#include "Common/CF.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

/////////////////////////////////////////////////////////////////////////////

  class StatusModel;

/////////////////////////////////////////////////////////////////////////////

  class StatusPanel : public QTreeView
  {
    Q_OBJECT

    public:

    StatusPanel(StatusModel * model, QWidget * parent = CFNULL);

    ~StatusPanel();

    private slots:

    void subSystemAdded(const QModelIndex & index);

    private:

    StatusModel * m_model;

  }; // class StatusPanel

/////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_StatusPanel_h
