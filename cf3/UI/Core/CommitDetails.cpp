// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QModelIndex>
#include <QVariant>

#include "common/CF.hpp"

#include "UI/Core/CommitDetailsItem.hpp"
#include "UI/Core/CommitDetails.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

////////////////////////////////////////////////////////////////////////////////

CommitDetails::CommitDetails(QObject * parent, const QString & nodePath)
: QAbstractItemModel(parent)
{
  m_node_path = nodePath;
}

////////////////////////////////////////////////////////////////////////////////

QVariant CommitDetails::data(const QModelIndex &index, int role) const
{
  QVariant returnValue;


  if(role == Qt::DisplayRole && index.isValid())
  {
    int rowNumber = index.row();
    int colNumber = index.column();

    if(rowNumber >= 0 && rowNumber < m_items.size())
    {
      CommitDetailsItem * item = m_items.at(rowNumber);

      switch (colNumber)
      {
        case 0:
          returnValue = item->option_name();
          break;

        case 1:
        {
          QString oldValue = item->old_value();
          returnValue = oldValue.isEmpty() ? "--" : QString("\"%1\"").arg(oldValue);
          break;
        }

        case 2:
        {
          QString value = item->current_value();
          returnValue = value.isEmpty() ? "--" : QString("\"%1\"").arg(value);
          break;
        }

        default:
          break;
      }

    }
  }

  return returnValue;
}


////////////////////////////////////////////////////////////////////////////////

QVariant CommitDetails::headerData( int section,
                                    Qt::Orientation orientation,
                                    int role ) const
{
  QVariant return_value;

  if (role == Qt::DisplayRole)
  {
    if(orientation == Qt::Horizontal)
    {
      switch (section)
      {
        case 0:
          return_value = "Name";
          break;

        case 1:
          return_value = "Old Value";
          break;

        case 2:
          return_value = "New Value";
          break;
      }
    }
    else
    {
      return_value = QString("Option #%1").arg(section+1);
    }
  }
  return return_value;
}

////////////////////////////////////////////////////////////////////////////////

QModelIndex CommitDetails::index( int row,
                                  int column,
                                  const QModelIndex & parent) const
{
  CommitDetailsItem * item;
  QModelIndex index;

  if(!this->hasIndex(row, column, parent))
    return QModelIndex();

  if(m_items.isEmpty())
    return QModelIndex();

  item = m_items.at(row);
  index = createIndex(row, column, item);

  return index;
}

////////////////////////////////////////////////////////////////////////////////

QModelIndex CommitDetails::parent(const QModelIndex &index) const
{
  return QModelIndex();
}

////////////////////////////////////////////////////////////////////////////////

int CommitDetails::rowCount(const QModelIndex &parent) const
{
  if( !parent.isValid() )
    return m_items.size();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

int CommitDetails::columnCount(const QModelIndex &parent) const
{
  return 3;
}

////////////////////////////////////////////////////////////////////////////////

void CommitDetails::set_option( const QString & option_name,
                                const QString & old_value,
                                const QString & current_value )
{
  m_items << new CommitDetailsItem(option_name, old_value, current_value);
}

////////////////////////////////////////////////////////////////////////////////

bool CommitDetails::has_options() const
{
  return !m_items.isEmpty();
}

////////////////////////////////////////////////////////////////////////////////

void CommitDetails::clear()
{
  m_items.clear();
  m_node_path.clear();
}

////////////////////////////////////////////////////////////////////////////////

QString CommitDetails::node_path() const
{
  return m_node_path;
}

////////////////////////////////////////////////////////////////////////////////

void CommitDetails::set_node_path(const QString & node_path)
{
  m_node_path = node_path;
}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3
