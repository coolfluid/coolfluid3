// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>

#include <QModelIndex>
#include <QVariant>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "rapidxml/rapidxml.hpp"

#include "Common/Signal.hpp"

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/NBrowser.hpp"
#include "GUI/Client/Core/NetworkThread.hpp"
#include "GUI/Client/Core/SignalNode.hpp"
#include "GUI/Client/Core/ThreadManager.hpp"

#include "GUI/Client/Core/NJournalBrowser.hpp"

////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////

NJournalBrowser::NJournalBrowser(const XmlNode * rootNode, QObject *parent) :
    QAbstractItemModel(parent),
    CNode(NBrowser::globalBrowser()->generateName(), "NJournalBrowser", CNode::JOURNAL_BROWSER_NODE)
{
  setRootNode(rootNode);

  regist_signal("list_journal", "List journal", "List journal")->
      signal->connect(boost::bind(&NJournalBrowser::list_journal, this, _1));

  m_columns << "Target" << "Sender" << "Receiver" << "Type" << "Direction" << "Time" /*<< "Status" << "Excecute"*/;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant NJournalBrowser::data(const QModelIndex & index, int role) const
{
  QVariant data;

  if(index.isValid() && role == Qt::DisplayRole)
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
  if(role == Qt::DisplayRole && section >= 0 )
  {
    if(orientation == Qt::Horizontal && section < m_columns.count())
      return m_columns.at(section);
    else if(orientation == Qt::Vertical && section < m_children.count())
      return QString("#%1").arg(section + 1);
  }

  return QVariant();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NJournalBrowser::toolTip() const
{
  return getComponentType();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

const SignalNode & NJournalBrowser::signal(const QModelIndex & index) const
{
  SignalNode * signal = indexToSignalNode(index);

  cf_assert(signal != nullptr);

  return *signal;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NJournalBrowser::setRootNode(const XmlNode * rootNode)
{
  emit layoutAboutToBeChanged();

  m_rootNode = rootNode;

  if(m_rootNode != nullptr)
    m_doc = ClientRoot::instance().docFromPtr(new XmlDoc(m_rootNode->content->document()));
  else
    m_doc = boost::shared_ptr<XmlDoc>();

  m_children.clear();

  if(m_rootNode != nullptr)
  {
    rapidxml::xml_node<> * currNode = m_rootNode->content->first_node("frame");

    for( ; currNode != nullptr ; currNode = currNode->next_sibling() )
      m_children.append(new SignalNode(new SignalFrame(currNode)));
  }

  // the underlying data changed, so we tell the view(s) to update
  emit layoutChanged();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NJournalBrowser::requestJournal()
{
  SignalFrame frame("list_journal", full_path(), SERVER_JOURNAL_PATH);

  ThreadManager::instance().network().send(frame);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NJournalBrowser::list_journal(SignalArgs & args)
{
  setRootNode(&args.node);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NJournalBrowser::sendExecSignal(const QModelIndex & index)
{
  if(!index.isValid())
    throw ValueNotFound(FromHere(), "Index is not valid.");

  if(!hasIndex(index.row(), index.column(), index.parent()) )
    throw ValueNotFound(FromHere(), "Index is not part of this model");

  std::stringstream ss;
  SignalFrame frame("","","");

  SignalNode * signal_node = indexToSignalNode(index);
  signal_node->node()->node.deep_copy(frame.node);

  rapidxml::xml_attribute<> * clientIdAttr;

  clientIdAttr = frame.node.content->first_attribute( "clientid" );

  // if found,  we remove the old client UUID.
  // (sending the signal automatically adds the correct UUID)
  if(clientIdAttr != nullptr)
    frame.node.content->remove_attribute(clientIdAttr);

  // modify the frame UUID
  ss << boost::uuids::random_generator()();
  frame.node.set_attribute( Protocol::Tags::attr_frameid(), ss.str());

  ThreadManager::instance().network().send(frame);
}

////////////////////////////////////////////////////////////////////////////

} // namespace ClientCore
} // namespace GUI
} // namespace CF
