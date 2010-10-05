// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_FilteringModel_hpp
#define CF_GUI_Client_FilteringModel_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QSortFilterProxyModel>

namespace CF {
namespace GUI {
namespace ClientCore {

  //////////////////////////////////////////////////////////////////////////////

  class FilteringModel : public QSortFilterProxyModel
  {
    Q_OBJECT

  public:

    FilteringModel(QObject *parent = 0);

  protected:

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

  }; // class FilteringModel

  //////////////////////////////////////////////////////////////////////////////

} // namespace ClientCore
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////


#endif // CF_GUI_Client_FilteringModel_hpp
