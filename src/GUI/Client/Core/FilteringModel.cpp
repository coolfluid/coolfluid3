// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QModelIndex>
#include <QDebug>

#include "GUI/Client/Core/NTree.hpp"
#include "GUI/Client/Core/FilteringModel.hpp"

using namespace CF::GUI::ClientCore;

FilteringModel::FilteringModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

//////////////////////////////////////////////////////////////////////////////////

bool FilteringModel::filterAcceptsRow(int row, const QModelIndex & parent) const
{
  NTree * tree = static_cast<NTree*>(sourceModel());
  QModelIndex index = sourceModel()->index(row, 0, parent);

  if(tree != CFNULL)
    return sourceModel()->data(index).isValid() && tree->nodeMatches(index, filterRegExp());
  else
    return QSortFilterProxyModel::filterAcceptsRow(row, parent);
}
