// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "GUI/Core/NTree.hpp"

#include "GUI/Core/PropertyModel.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

PropertyModel::PropertyModel()
  : QAbstractItemModel()
{
  currentIndexChanged(NTree::globalTree()->currentIndex(), QModelIndex());

  m_columns << "Name" << "Value";

  connect(NTree::globalTree().get(), SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)),
          this, SLOT(currentIndexChanged(QModelIndex,QModelIndex)));
}

////////////////////////////////////////////////////////////////////////////

//PropertyModel::~PropertyModel()
//{
//  this->emptyList();
//}

////////////////////////////////////////////////////////////////////////////

QVariant  PropertyModel::data(const QModelIndex & index, int role) const
{
  QVariant data;

  if(role == Qt::DisplayRole && index.isValid())
  {
    PropertyItem * item = m_data.at(index.row());

    switch(index.column())
    {
    case 0:
      data = item->m_name;
      break;
    case 1:
      data = item->m_value;
      break;
    }
  }

  return data;
}

////////////////////////////////////////////////////////////////////////////

QModelIndex PropertyModel::index(int row, int column, const QModelIndex & parent) const
{
  PropertyItem * item;
  QModelIndex index;


  if(this->hasIndex(row, column, parent))
  {
    item = m_data.at(row);

    if(!parent.isValid())
      index = createIndex(row, column, item);
  }

  return index;
}

////////////////////////////////////////////////////////////////////////////

QModelIndex PropertyModel::parent(const QModelIndex &child) const
{
  return QModelIndex();
}

////////////////////////////////////////////////////////////////////////////

int PropertyModel::rowCount(const QModelIndex & parent) const
{
  if (!parent.isValid())
    return m_data.size();

  return 0;
}

////////////////////////////////////////////////////////////////////////////

int PropertyModel::columnCount(const QModelIndex & parent) const
{
  return m_columns.count();
}

////////////////////////////////////////////////////////////////////////////

QVariant PropertyModel::headerData(int section, Qt::Orientation orientation,
                           int role) const
{
  if(role == Qt::DisplayRole && orientation == Qt::Horizontal && section >= 0
     && section < m_columns.size())
    return m_columns.at(section);

  return QVariant();
}

////////////////////////////////////////////////////////////////////////////

void PropertyModel::currentIndexChanged(const QModelIndex & newIndex,
                                        const QModelIndex & oldIndex)
{
  QMap<QString, QString> props;
  QMap<QString, QString>::iterator it;
  unsigned int i;

  emit layoutAboutToBeChanged();

  NTree::globalTree()->listNodeProperties(newIndex, props);

  this->emptyList();

  for(i = 0, it = props.begin() ; it != props.end() ; it++, i++)
  {
    m_data << new PropertyItem(it.key(), it.value(), i);
  }

  emit layoutChanged();
}

////////////////////////////////////////////////////////////////////////////

void PropertyModel::emptyList()
{
  while(!m_data.isEmpty())
    delete m_data.takeFirst();
}

////////////////////////////////////////////////////////////////////////////

} // Core
} // GUI
} // CF
