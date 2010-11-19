// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_FilteringModel_hpp
#define CF_GUI_Client_Core_FilteringModel_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QIcon>
#include <QMap>
#include <QSortFilterProxyModel>

#include "GUI/Client/UI/LibClientUI.hpp"

namespace CF {
namespace GUI {
namespace ClientUI {

  //////////////////////////////////////////////////////////////////////////////

  class ClientUI_API FilteringModel : public QSortFilterProxyModel
  {
    Q_OBJECT

  public:

    FilteringModel(QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role) const;

  protected:

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

  private:

    QMap<QString, QIcon> m_icons;

  }; // class FilteringModel

  //////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////


#endif // CF_GUI_Client_Core_FilteringModel_hpp
