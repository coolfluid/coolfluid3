// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_FilteringModel_hpp
#define cf3_ui_core_FilteringModel_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QIcon>
#include <QMap>
#include <QSortFilterProxyModel>

#include "UI/Graphics/LibGraphics.hpp"

namespace cf3 {
namespace ui {
namespace graphics {

  //////////////////////////////////////////////////////////////////////////////

  class Graphics_API FilteringModel : public QSortFilterProxyModel
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

} // Core
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////////


#endif // cf3_ui_core_FilteringModel_hpp
