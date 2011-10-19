// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "rapidxml/rapidxml.hpp"

#include "Common/Signal.hpp"
#include "Common/XML/XmlDoc.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "Common/XML/Protocol.hpp"

#include "UI/Core/NBrowser.hpp"
#include "UI/Core/NetworkQueue.hpp"

#include "UI/Core/NJournalBrowser.hpp"

////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::common::XML;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

NJournalBrowser::NJournalBrowser(const XmlNode * rootNode, QObject *parent) :
    QAbstractItemModel(parent),
    CNode(NBrowser::globalBrowser()->generateName().toStdString(), "NJournalBrowser", CNode::STANDARD_NODE)
{
  setRootNode(rootNode);

  regist_signal( "list_journal" )
    ->connect( boost::bind( &NJournalBrowser::list_journal, this, _1 ) )
    ->description("List journal")
    ->pretty_name("List journal");

  m_columns << "Target" << "Sender" << "Receiver" << "Type" << "Direction" << "Time" /*<< "Status" << "Excecute"*/;
}

////////////////////////////////////////////////////////////////////////////

QVariant NJournalBrowser::data(const QModelIndex & index, int role) const
{
  QVariant data;

  if(index.isValid() && role == Qt::DisplayRole)
  {
    SignalArgs & node = *this->indexToXmlNode(index);

    switch(index.column())
    {
    case 0: // target
      data = readAttribute(node, "target");
      break;
    case 1: // sender
      data = readAttribute(node, "sender");
      break;
    case 2: // receiver
      data = readAttribute(node, "receiver");
      break;
    case 3: // type
      data = readAttribute(node, "type");
      break;
    case 4: // direction
      data = QString("???");
      break;
    case 5: // time
      data = readAttribute(node, "time");
      break;
    case 6: // status (inspect ?)
      data = QString("Inspect");
      break;
    case 7: // execute (useful ?)
      data = QString("Excecute");
      break;
    }
  }

  return data;
}

////////////////////////////////////////////////////////////////////////////

QModelIndex NJournalBrowser::index(int row, int column, const QModelIndex & parent) const
{
  SignalArgs * childNode = nullptr;
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

////////////////////////////////////////////////////////////////////////////

QModelIndex NJournalBrowser::parent(const QModelIndex &child) const
{
  return QModelIndex();
}

////////////////////////////////////////////////////////////////////////////

int NJournalBrowser::rowCount(const QModelIndex & parent) const
{
  // if the parent is not valid, we have one child: the root
  if (!parent.isValid())
    return m_children.count();

  return 0;
}

////////////////////////////////////////////////////////////////////////////

int NJournalBrowser::columnCount(const QModelIndex & parent) const
{
  return m_columns.count();
}

////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////

QString NJournalBrowser::toolTip() const
{
  return componentType();
}

////////////////////////////////////////////////////////////////////////////

const SignalArgs & NJournalBrowser::signal(const QModelIndex & index) const
{
  SignalArgs * signal = indexToXmlNode(index);

  cf_assert(signal != nullptr);

  return *signal;
}

////////////////////////////////////////////////////////////////////////////

void NJournalBrowser::setRootNode(const XmlNode * rootNode)
{
  emit layoutAboutToBeChanged();

  m_currentDoc = XmlDoc::Ptr(new XmlDoc());
  m_rootNode = m_currentDoc->add_node("tmp");

  if(is_not_null(rootNode))
    rootNode->deep_copy(m_rootNode);

  m_children.clear();


  if(m_rootNode.is_valid())
  {

    rapidxml::xml_node<> * currNode = m_rootNode.content->first_node("frame");

    for( ; currNode != nullptr ; currNode = currNode->next_sibling() )
      m_children.append( new SignalArgs(currNode) );
  }

  // the underlying data changed, so we tell the view(s) to update
  emit layoutChanged();
}

////////////////////////////////////////////////////////////////////////////

void NJournalBrowser::requestJournal()
{
  SignalFrame frame("list_journal", uri(), SERVER_JOURNAL_PATH);

  NetworkQueue::global_queue()->send( frame, NetworkQueue::IMMEDIATE );
}

////////////////////////////////////////////////////////////////////////////

void NJournalBrowser::list_journal(SignalArgs & args)
{
//  if(args.has_map(Protocol::Tags::key_signals()))
  //    setRootNode(&args.map(Protocol::Tags::key_signals()).node);
  XmlNode node = args.main_map.find_value(Protocol::Tags::key_signals());
  setRootNode(&node);
}

////////////////////////////////////////////////////////////////////////////

void NJournalBrowser::sendExecSignal(const QModelIndex & index)
{
  if(!index.isValid())
    throw ValueNotFound(FromHere(), "Index is not valid.");

  if(!hasIndex(index.row(), index.column(), index.parent()) )
    throw ValueNotFound(FromHere(), "Index is not part of this model");

  std::stringstream ss;
  SignalFrame frame;

  SignalArgs * signal_node = indexToXmlNode(index);
  signal_node->node.deep_copy(frame.node);

  rapidxml::xml_attribute<> * clientIdAttr;

  clientIdAttr = frame.node.content->first_attribute( "clientid" );

  // if found,  we remove the old client UUID.
  // (sending the signal automatically adds the correct UUID)
  if(clientIdAttr != nullptr)
    frame.node.content->remove_attribute(clientIdAttr);

  // modify the frame UUID
  ss << boost::uuids::random_generator()();
  frame.node.set_attribute( Protocol::Tags::attr_frameid(), ss.str());

  NetworkQueue::global_queue()->send( frame );
}

////////////////////////////////////////////////////////////////////////////

QString NJournalBrowser::readAttribute(const SignalArgs &sig, const char *name) const
{
  rapidxml::xml_attribute<>* attr = sig.node.content->first_attribute(name);

  if(attr != nullptr)
    return attr->value();

  return QString();
}

////////////////////////////////////////////////////////////////////////////

} // namespace Core
} // namespace UI
} // namespace CF
