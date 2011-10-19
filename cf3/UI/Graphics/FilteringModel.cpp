// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFileIconProvider>
#include <QModelIndex>

#include "UI/Core/NTree.hpp"

#include "UI/Graphics/FilteringModel.hpp"

using namespace cf3::UI::Core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

FilteringModel::FilteringModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
  QFileIconProvider prov;
  m_icons["CRoot"] = prov.icon(QFileIconProvider::Computer);
  m_icons["CF.Common.CGroup"] = prov.icon(QFileIconProvider::Folder);
  m_icons["CF.Mesh.CMesh"] = prov.icon(QFileIconProvider::Drive);
  m_icons["CF.Mesh.CMeshReader"] = prov.icon(QFileIconProvider::Trashcan);
  m_icons["CF.Common.CLink"] = prov.icon(QFileIconProvider::Network);
  m_icons["NLog"] = prov.icon(QFileIconProvider::Folder);
  m_icons["NBrowser"] = prov.icon(QFileIconProvider::Folder);
  m_icons["NTree"] = prov.icon(QFileIconProvider::Folder);
  m_icons["NPlugins"] = prov.icon(QFileIconProvider::Folder);
  m_icons["NPlugin"] = prov.icon(QFileIconProvider::Trashcan);
  m_icons["NetworkQueue"] = prov.icon(QFileIconProvider::Folder);
}

//////////////////////////////////////////////////////////////////////////////////

QVariant FilteringModel::data(const QModelIndex &index, int role) const
{
  if(role == Qt::DecorationRole && index.column() == 0)
  {
    QModelIndex typeIndex = this->index(index.row(), 1, index.parent());
    QString type = QSortFilterProxyModel::data(typeIndex, Qt::DisplayRole).toString();
    return m_icons.value(type, QFileIconProvider().icon(QFileIconProvider::File));
  }
  else
    return QSortFilterProxyModel::data(index, role);
}

//////////////////////////////////////////////////////////////////////////////////

bool FilteringModel::filterAcceptsRow(int row, const QModelIndex & parent) const
{
  NTree * tree = static_cast<NTree*>(sourceModel());
  QModelIndex index = sourceModel()->index(row, 0, parent);

  if(tree != nullptr)
    return sourceModel()->data(index).isValid() && tree->nodeMatches(index, filterRegExp());
  else
    return QSortFilterProxyModel::filterAcceptsRow(row, parent);

  return true;
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
