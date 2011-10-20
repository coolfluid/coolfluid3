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
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////////

CommitDetails::CommitDetails(QObject * parent, const QString & nodePath)
: QAbstractItemModel(parent)
{
  m_nodePath = nodePath;
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
          returnValue = item->optionName();
          break;

        case 1:
        {
          QString oldValue = item->oldValue();
          returnValue = oldValue.isEmpty() ? "--" : QString("\"%1\"").arg(oldValue);
          break;
        }

        case 2:
        {
          QString value = item->currentValue();
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

QVariant CommitDetails::headerData(int section, Qt::Orientation orientation,
                                   int role) const
{
  QVariant returnValue;

  if (role == Qt::DisplayRole)
  {
    if(orientation == Qt::Horizontal)
    {
      switch (section)
      {
        case 0:
          returnValue = "Name";
          break;

        case 1:
          returnValue = "Old Value";
          break;

        case 2:
          returnValue = "New Value";
          break;
      }
    }
    else
    {
      returnValue = QString("Option #%1").arg(section+1);
    }
  }
  return returnValue;
}

////////////////////////////////////////////////////////////////////////////////

QModelIndex CommitDetails::index(int row, int column,
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

void CommitDetails::setOption(const QString & optionName,
                              const QString & oldValue,
                              const QString & currentValue)
{
  m_items << new CommitDetailsItem(optionName, oldValue, currentValue);
}

////////////////////////////////////////////////////////////////////////////////

bool CommitDetails::hasOptions() const
{
  return !m_items.isEmpty();
}

////////////////////////////////////////////////////////////////////////////////

void CommitDetails::clear()
{
  m_items.clear();
  m_nodePath.clear();
}

////////////////////////////////////////////////////////////////////////////////

QString CommitDetails::nodePath() const
{
  return m_nodePath;
}

////////////////////////////////////////////////////////////////////////////////

void CommitDetails::setNodePath(const QString & nodePath)
{
  m_nodePath = nodePath;
}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3
