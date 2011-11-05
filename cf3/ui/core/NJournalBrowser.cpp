// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>

#include "rapidxml/rapidxml.hpp"

#include "common/Signal.hpp"
#include "common/XML/XmlDoc.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include "common/UUCount.hpp"
#include "common/XML/Protocol.hpp"

#include "ui/core/NBrowser.hpp"
#include "ui/core/NetworkQueue.hpp"

#include "ui/core/NJournalBrowser.hpp"

////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::common::XML;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

////////////////////////////////////////////////////////////////////////////

NJournalBrowser::NJournalBrowser(const XmlNode * rootNode, QObject *parent) :
    QAbstractItemModel(parent),
    CNode(NBrowser::global()->generate_name().toStdString(), "NJournalBrowser", CNode::STANDARD_NODE)
{
  set_root_node(rootNode);

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
    SignalArgs & node = *this->index_to_xml_node(index);

    switch(index.column())
    {
    case 0: // target
      data = read_attribute(node, "target");
      break;
    case 1: // sender
      data = read_attribute(node, "sender");
      break;
    case 2: // receiver
      data = read_attribute(node, "receiver");
      break;
    case 3: // type
      data = read_attribute(node, "type");
      break;
    case 4: // direction
      data = QString("???");
      break;
    case 5: // time
      data = read_attribute(node, "time");
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

QString NJournalBrowser::tool_tip() const
{
  return component_type();
}

////////////////////////////////////////////////////////////////////////////

const SignalArgs & NJournalBrowser::signal(const QModelIndex & index) const
{
  SignalArgs * signal = index_to_xml_node(index);

  cf3_assert(signal != nullptr);

  return *signal;
}

////////////////////////////////////////////////////////////////////////////

void NJournalBrowser::set_root_node(const XmlNode * rootNode)
{
  emit layoutAboutToBeChanged();

  m_current_doc = XmlDoc::Ptr(new XmlDoc());
  m_root_node = m_current_doc->add_node("tmp");

  if(is_not_null(rootNode))
    rootNode->deep_copy(m_root_node);

  m_children.clear();


  if(m_root_node.is_valid())
  {

    rapidxml::xml_node<> * currNode = m_root_node.content->first_node("frame");

    for( ; currNode != nullptr ; currNode = currNode->next_sibling() )
      m_children.append( new SignalArgs(currNode) );
  }

  // the underlying data changed, so we tell the view(s) to update
  emit layoutChanged();
}

////////////////////////////////////////////////////////////////////////////

void NJournalBrowser::request_journal()
{
  SignalFrame frame("list_journal", uri(), SERVER_JOURNAL_PATH);

  NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
}

////////////////////////////////////////////////////////////////////////////

void NJournalBrowser::list_journal(SignalArgs & args)
{
//  if(args.has_map(Protocol::Tags::key_signals()))
  //    setRootNode(&args.map(Protocol::Tags::key_signals()).node);
  XmlNode node = args.main_map.find_value(Protocol::Tags::key_signals());
  set_root_node(&node);
}

////////////////////////////////////////////////////////////////////////////

void NJournalBrowser::send_exec_signal(const QModelIndex & index)
{
  if(!index.isValid())
    throw ValueNotFound(FromHere(), "Index is not valid.");

  if(!hasIndex(index.row(), index.column(), index.parent()) )
    throw ValueNotFound(FromHere(), "Index is not part of this model");

  std::stringstream ss;
  SignalFrame frame;

  SignalArgs * signal_node = index_to_xml_node(index);
  signal_node->node.deep_copy(frame.node);

  rapidxml::xml_attribute<> * clientIdAttr;

  clientIdAttr = frame.node.content->first_attribute( "clientid" );

  // if found,  we remove the old client UuiD.
  // (sending the signal automatically adds the correct UuiD)
  if(clientIdAttr != nullptr)
    frame.node.content->remove_attribute(clientIdAttr);

  // modify the frame UuiD
  ss << UUCount().string();
  frame.node.set_attribute( Protocol::Tags::attr_frameid(), ss.str());

  NetworkQueue::global()->send( frame );
}

////////////////////////////////////////////////////////////////////////////

QString NJournalBrowser::read_attribute(const SignalArgs &sig, const char *name) const
{
  rapidxml::xml_attribute<>* attr = sig.node.content->first_attribute(name);

  if(attr != nullptr)
    return attr->value();

  return QString();
}

////////////////////////////////////////////////////////////////////////////

} // namespace Core
} // namespace ui
} // namespace cf3
