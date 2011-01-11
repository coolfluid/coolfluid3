// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QModelIndex>
#include <QVariant>

#include "Common/XmlHelpers.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/SignalNode.hpp"

#include "GUI/Client/Core/NJournalBrowser.hpp"

////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////

NJournalBrowser::NJournalBrowser(const XmlNode * rootNode, QObject *parent) :
    QAbstractItemModel(parent),
    CNode(ClientRoot::browser()->generateName(), "NJournalBrowser", CNode::JOURNAL_BROWSER_NODE),
    m_rootNode(rootNode)
{
  cf_assert(rootNode);

  const XmlNode * currNode = m_rootNode->first_node("frame");

  for( ; currNode != nullptr ; currNode = currNode->next_sibling() )
    m_children.append(new SignalNode(currNode));

  m_columns << "Target" << "Sender" << "Receiver" << "Type" << "Direction" << "Time" << "Status" << "Excecute";
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant NJournalBrowser::data(const QModelIndex & index, int role) const
{
  QVariant data;

  if(data.isValid() && role == Qt::DisplayRole)
  {
    SignalNode * node = this->indexToSignalNode(index);

    switch(index.column())
    {
    case 0: // target
      data = QString(node->target());
      break;
    case 1: // sender
      data = QString(node->sender());
      break;
    case 2: // receiver
      data = QString(node->receiver());
      break;
    case 3: // type
      data = QString(node->type());
      break;
    case 4: // direction
      data = QString(node->direction());
      break;
    case 5: // time
      data = QString(node->time());
      break;
    case 6: // status (inspect ?)
      data = QString("Inspect");
      break;
    case 7: // execute (useful ?)
      data = QString("Execute");
      break;
    }
  }

  return data;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QModelIndex NJournalBrowser::index(int row, int column, const QModelIndex & parent) const
{
  SignalNode * childNode = nullptr;
  QModelIndex index;

  if(this->hasIndex(row, column, parent))
  {
    if( !parent.isValid())
//      childNode = m_rootNode;
////    else
      childNode = m_children.at(row);

    if(childNode != nullptr)
      index = createIndex(row, column, childNode);
  }

  return index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QModelIndex NJournalBrowser::parent(const QModelIndex &child) const
{
  QModelIndex index;

//  if(child.isValid())
//  {
//    TreeNode * parentNode = this->indexToTreeNode(child)->parentNode();

//    if (parentNode != nullptr)
//      index = createIndex(parentNode->rowNumber(), 0, parentNode);
//  }

  return index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int NJournalBrowser::rowCount(const QModelIndex & parent) const
{
//  if (parent.column() > 0)
//    return 0;

  // if the parent is not valid, we have one child: the root
  if (!parent.isValid())
    return m_children.count();

  return 0;//this->indexToTreeNode(parent)->childCount();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int NJournalBrowser::columnCount(const QModelIndex & parent) const
{
  return m_columns.count();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant NJournalBrowser::headerData(int section, Qt::Orientation orientation,
                           int role) const
{
  if(role == Qt::DisplayRole && orientation == Qt::Horizontal && section >= 0
     && section < m_columns.size())
    return m_columns.at(section);

  return QVariant();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NJournalBrowser::toolTip() const
{
  return getComponentType();
}

////////////////////////////////////////////////////////////////////////////

} // namespace ClientCore
} // namespace GUI
} // namespace CF
