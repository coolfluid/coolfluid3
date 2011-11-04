// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QMutex>

#include "rapidxml/rapidxml.hpp"

#include "common/Signal.hpp"
#include "common/FindComponents.hpp"

#include "ui/core/TreeThread.hpp"
#include "ui/core/NetworkQueue.hpp"
#include "ui/core/NLog.hpp"
#include "ui/core/NRoot.hpp"
#include "ui/core/ThreadManager.hpp"
#include "ui/core/TreeNode.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include "ui/core/NTree.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

/////////////////////////////////////////////////////////////////////////

NTree::NTree(NRoot::Ptr rootNode)
  : CNode(CLIENT_TREE, "NTree", CNode::DEBUG_NODE),
    m_advanced_mode(false),
    m_debug_mode_enabled(false)
{


  if(rootNode.get() == nullptr)
    m_root_node = new TreeNode(ThreadManager::instance().tree().root(), nullptr, 0);
  else
    m_root_node = new TreeNode(rootNode, nullptr, 0);

  m_mutex = new QMutex();

  m_columns << "Name" << "Type";

  unregist_signal("list_tree"); // unregister base class signal

  regist_signal( "list_tree" )
    ->description("New tree")
    ->pretty_name("")->connect(boost::bind(&NTree::list_tree_reply, this, _1));
}

////////////////////////////////////////////////////////////////////////////

void NTree::set_tree_root(NRoot::Ptr rootNode)
{


  //QMutexLocker locker(m_mutex);

  // initiate the removing process
  emit layoutAboutToBeChanged();
  delete m_root_node;

  m_root_node = nullptr;

  if(rootNode.get() != nullptr)
    m_root_node = new TreeNode(rootNode, nullptr, 0);

  emit layoutChanged();
}

////////////////////////////////////////////////////////////////////////////

NRoot::Ptr NTree::tree_root() const
{


  return m_root_node->node()->castTo<NRoot>();
}

////////////////////////////////////////////////////////////////////////////

void NTree::set_current_index(const QModelIndex & newIndex)
{


  //QMutexLocker locker(m_mutex);

  if(!this->are_from_same_node(m_current_index, newIndex))
  {
    QModelIndex oldIndex = m_current_index;
    m_current_index = newIndex;
    emit current_index_changed(newIndex, oldIndex);
  }
}

////////////////////////////////////////////////////////////////////////////

QModelIndex NTree::current_index() const
{


  return m_current_index;
}

////////////////////////////////////////////////////////////////////////////

URI NTree::current_path() const
{


  return pathFromIndex( m_current_index );
}

////////////////////////////////////////////////////////////////////////////

void NTree::list_node_options(const QModelIndex & index,
                           QList<Option::ConstPtr> & options,
                           bool * ok) const
{


  //QMutexLocker locker(m_mutex);

  TreeNode * node = this->index_to_tree_node(index);

  if(ok != nullptr)
    *ok = node != nullptr;

  options.clear();

  if(node != nullptr && node->node().get() != nullptr)
    node->node()->list_options(options);
}

////////////////////////////////////////////////////////////////////////////

void NTree::list_node_properties(const QModelIndex &index,
                              QMap<QString, QString> &props, bool *ok) const
{


  //QMutexLocker locker(m_mutex);

  TreeNode * node = this->index_to_tree_node(index);

  if(ok != nullptr)
    *ok = node != nullptr;

  props.clear();

  if(node != nullptr && node->node().get() != nullptr)
    node->node()->list_properties(props);
}

////////////////////////////////////////////////////////////////////////////

void NTree::list_node_actions(const QModelIndex & index, QList<ActionInfo> & actions,
                           bool * ok) const
{


  //QMutexLocker locker(m_mutex);

  TreeNode * node = this->index_to_tree_node(index);

  if(ok != nullptr)
    *ok = node != nullptr;

  actions.clear();

  if(node != nullptr && node->node().get() != nullptr)
    node->node()->list_signals(actions);
}

////////////////////////////////////////////////////////////////////////////

QString NTree::node_path(const QModelIndex & index) const
{


  //QMutexLocker locker(m_mutex);

  QString path;

  this->build_node_path_recursive(index, path);

  return path;
}

////////////////////////////////////////////////////////////////////////////

void NTree::set_advanced_mode(bool advanceMode)
{


  //QMutexLocker locker(m_mutex);

  if(m_advanced_mode ^ advanceMode) // if values are different
  {
    /// @todo find a better way to refresh the tree
    emit layoutAboutToBeChanged();
    m_advanced_mode = advanceMode;
    emit advanced_mode_changed(m_advanced_mode);
    emit layoutChanged();
  }
}

////////////////////////////////////////////////////////////////////////////

bool NTree::is_advanced_mode() const
{


  return m_advanced_mode;
}

////////////////////////////////////////////////////////////////////////////

bool NTree::are_from_same_node(const QModelIndex & left, const QModelIndex & right) const
{


  return left.isValid() && left.internalPointer() == right.internalPointer();
}

////////////////////////////////////////////////////////////////////////////

CNode::ConstPtr NTree::node_by_path(const URI & path) const
{


  //QMutexLocker locker(m_mutex);

  QString pathStr = path.path().c_str();
  QStringList comps;
  QStringList::iterator it;
  CNode::ConstPtr node = m_root_node->node();

  if(path.is_absolute())
  {
    comps = pathStr.split(URI::separator().c_str(), QString::SkipEmptyParts);

    if(comps.first().toStdString() == node->name())
      comps.removeFirst();

    for(it = comps.begin() ; it != comps.end() && node.get() != nullptr ; it++)
    {
      Component::ConstPtr comp = node->get_child_ptr(it->toStdString());

      if( is_not_null(comp.get()) )
        node = comp->as_ptr_checked<CNode>();
      else
        node = CNode::ConstPtr();
    }
  }

  return node;
}

////////////////////////////////////////////////////////////////////////////

CNode::Ptr NTree::node_by_path(const URI & path)
{


  //QMutexLocker locker(m_mutex);

  QString pathStr = path.path().c_str();
  QStringList comps;
  QStringList::iterator it;
  CNode::Ptr node = m_root_node->node();

  if(path.is_absolute())
  {
    comps = pathStr.split(URI::separator().c_str(), QString::SkipEmptyParts);

    if(comps.first().toStdString() == node->name())
      comps.removeFirst();

    for(it = comps.begin() ; it != comps.end() && node.get() != nullptr ; it++)
    {
      Component::Ptr comp = node->get_child_ptr(it->toStdString());

      if( is_not_null(comp.get()) )
        node = comp->as_ptr_checked<CNode>();
      else
        node = CNode::Ptr();
    }
  }

  return node;
}

////////////////////////////////////////////////////////////////////////////

QModelIndex NTree::index_from_path(const URI & path) const
{


  QModelIndex index = this->index(0,0);
  QString pathStr = path.path().c_str();
  QStringList comps;
  QStringList::iterator it;
  TreeNode * treeNode = m_root_node;

  cf3_always_assert( path.scheme() == URI::Scheme::CPATH );
  cf3_always_assert( treeNode != nullptr );

  if(path.is_absolute())
  {
    comps = pathStr.split(URI::separator().c_str(), QString::SkipEmptyParts);

    for(it = comps.begin() ; it != comps.end() && treeNode != nullptr ; it++)
    {
      treeNode = treeNode->child_by_name(*it);

      if(treeNode != nullptr)
        index = this->index(treeNode->row_number(), 0, index);
      else
        index = QModelIndex();
    }
  }
  else
    index = QModelIndex();

  return index;
}

////////////////////////////////////////////////////////////////////////////

URI NTree::pathFromIndex(const QModelIndex & index) const
{


  //QMutexLocker locker(m_mutex);

  TreeNode * treeNode = this->index_to_tree_node(index);
  URI path;

  if(treeNode != nullptr)
    path = treeNode->node()->uri();

  return path;
}

////////////////////////////////////////////////////////////////////////////

void NTree::set_debug_mode_enabled(bool debugMode)
{


  //QMutexLocker locker(m_mutex);

  if(m_debug_mode_enabled ^ debugMode)
  {
    emit layoutAboutToBeChanged();
    m_debug_mode_enabled = debugMode;
    emit layoutChanged();
  }
}

////////////////////////////////////////////////////////////////////////////

bool NTree::is_debug_mode_enabled() const
{


  return m_debug_mode_enabled;
}

////////////////////////////////////////////////////////////////////////////

void NTree::update_root_children()
{


  //QMutexLocker locker(m_mutex);

  emit layoutAboutToBeChanged();
  m_root_node->update_child_list();
  emit layoutChanged();
}

////////////////////////////////////////////////////////////////////////////

void NTree::options_changed(const URI & path)
{


  QModelIndex index = this->index_from_path(path);

  if(index.isValid())
    emit dataChanged(index, index);
}

////////////////////////////////////////////////////////////////////////////

bool NTree::node_matches(const QModelIndex & index, const QRegExp & regex) const
{


  //QMutexLocker locker(m_mutex);

  Component::Ptr node = m_root_node->node()->castTo<NRoot>();

  // if the index is value, we get the right node
  if(index.isValid() && index_to_tree_node(index) != m_root_node)
    node = index_to_node(index);

  if(node.get() != nullptr)
    return this->node_matches_recursive(node, regex);
  else
    return false;
}

////////////////////////////////////////////////////////////////////////////

bool NTree::check_index_visible(const QModelIndex & index) const
{


  //QMutexLocker locker(m_mutex);

  bool visible = false;

  if(index.isValid())
  {
    CNode::Ptr node = this->index_to_node(index);

    if( is_not_null(node.get()) )
    {
      // below, true if node is basic or if we are in advanced mode
      visible = node->has_tag("basic") || m_advanced_mode;
      // below, true if node is not of debug type or if we are in debug mode
      visible &= (node->type() != CNode::DEBUG_NODE) || m_debug_mode_enabled;
    }

  }

  return visible;
}

////////////////////////////////////////////////////////////////////////////

void NTree::modify_options(const QModelIndex & index,
                          const QMap<QString, QString> & options)
{


  //QMutexLocker locker(m_mutex);

  TreeNode * node = this->index_to_tree_node(index);

  if(node != nullptr)
    node->node()->modify_options(options);
  else
    NLog::global()->add_error("Could not modify options! Invalid node.");
}

////////////////////////////////////////////////////////////////////////////

QVariant NTree::data(const QModelIndex & index, int role) const
{


  QVariant data;

  if(check_index_visible(index))
  {
    //QMutexLocker locker(m_mutex);
    CNode::Ptr node = this->index_to_node(index);

    if(role == Qt::DisplayRole)
    {
      switch(index.column())
      {
      case 0:
        data = QString(node->name().c_str());
        break;
      case 1:
        data = QString(node->component_type());
        break;
      }
    }
    else if(role == Qt::ToolTipRole)
      data = node->tool_tip();
  }

  return data;
}

////////////////////////////////////////////////////////////////////////////

QModelIndex NTree::index(int row, int column, const QModelIndex & parent) const
{



  TreeNode * childNode;
  QModelIndex index;

  if(this->hasIndex(row, column, parent))
  {
    if( !parent.isValid())
      childNode = m_root_node;
    else
      childNode = this->index_to_tree_node(parent)->child(row);

    if(childNode != nullptr)
      index = createIndex(row, column, childNode);
  }

  return index;
}

////////////////////////////////////////////////////////////////////////////

QModelIndex NTree::parent(const QModelIndex &child) const
{


  //QMutexLocker locker(m_mutex);

  QModelIndex index;

  if(child.isValid())
  {
    TreeNode * parentNode = this->index_to_tree_node(child)->parent_node();

    if (parentNode != nullptr)
      index = createIndex(parentNode->row_number(), 0, parentNode);
  }

  return index;
}

////////////////////////////////////////////////////////////////////////////

int NTree::rowCount(const QModelIndex & parent) const
{


  /// @find
//  //QMutexLocker locker(m_mutex);

  if (parent.column() > 0)
    return 0;

  // if the parent is not valid, we have one child: the root
  if (!parent.isValid())
    return 1;

  TreeNode* tree_node = this->index_to_tree_node(parent);
  if(is_not_null(tree_node))
    return tree_node->child_count();

  return 0;
}

////////////////////////////////////////////////////////////////////////////

int NTree::columnCount(const QModelIndex & parent) const
{


  return m_columns.count();
}

////////////////////////////////////////////////////////////////////////////

QVariant NTree::headerData(int section, Qt::Orientation orientation,
                           int role) const
{


  if(role == Qt::DisplayRole && orientation == Qt::Horizontal && section >= 0
     && section < m_columns.size())
    return m_columns.at(section);

  return QVariant();
}

////////////////////////////////////////////////////////////////////////////

void NTree::list_tree_reply(SignalArgs & args)
{
  //QMutexLocker locker(m_mutex);
  emit begin_update_tree();
  beginResetModel();

  try
  {
    NRoot::Ptr tree_root = m_root_node->node()->castTo<NRoot>();
    CNode::Ptr root_node = CNode::create_from_xml(args.main_map.content.content->first_node());
    ComponentIterator<CNode> it = component_begin<CNode>(root_node->root());
    ComponentIterator<CNode> root_end = component_end<CNode>(root_node->root());
    URI currentIndexPath;

    if(m_current_index.isValid())
    {
      currentIndexPath = index_to_tree_node(m_current_index)->node()->uri();
    }

    //
    // rename the root
    //
    tree_root->rename(root_node->name());
    tree_root->rename(root_node->name());

    //
    // remove old nodes
    //
    ComponentIterator<CNode> itRem = component_begin<CNode>(*tree_root);
    ComponentIterator<CNode> tree_root_end = component_end<CNode>(*tree_root);

    QList<std::string> list_to_remove;
    QList<std::string>::iterator itList;

    for( ; itRem != tree_root_end ; itRem++)
    {
      if(!itRem->is_local_component() && !itRem->is_root() )
        list_to_remove << itRem->name();
    }

    itList = list_to_remove.begin();

    for( ; itList != list_to_remove.end() ; itList++)
    {
      tree_root->access_component_ptr_checked(*itList)->as_ptr<CNode>()->about_to_be_removed();
      tree_root->remove_component(*itList);
    }

    //
    // add the new nodes
    //

    for( ; it != root_end ; it++)
      tree_root->add_component(it.get());

    // child count may have changed, ask the root TreeNode to update its internal data
    m_root_node->update_child_list();

    // retrieve the previous index, if it still exists
    if(!currentIndexPath.path().empty())
      m_current_index = this->index_from_path(currentIndexPath);


    NLog::global()->add_message("Tree updated.");
  }
  catch(XmlError & xe)
  {
    NLog::global()->add_exception(xe.what());
  }

  // tell the view to update the whole thing
  endResetModel();

  emit end_update_tree();

  emit current_index_changed(m_current_index, QModelIndex());

}

////////////////////////////////////////////////////////////////////////////

void NTree::clear_tree()
{
  beginResetModel();

  //QMutexLocker locker(m_mutex);

  NRoot::Ptr treeRoot = m_root_node->node()->castTo<NRoot>();
  ComponentIterator<CNode> itRem = component_begin<CNode>(*treeRoot);
  ComponentIterator<CNode> tree_root_end = component_end<CNode>(*treeRoot);
  QMap<int, std::string> listToRemove;
  QMutableMapIterator<int, std::string> itList(listToRemove);

  for(int i = 0 ; itRem != tree_root_end ; itRem++, i++)
  {
    if(!itRem->is_local_component())
      listToRemove[i] = itRem->name();
  }

  itList.toBack();

  for( ; itList.hasPrevious() ; )
  {
    itList.previous();

    emit beginRemoveRows(index(0,0), itList.key(), itList.key());
    treeRoot->remove_component(itList.value());
    m_root_node->update_child_list();
    emit endRemoveRows();
  }

  endResetModel();
}

////////////////////////////////////////////////////////////////////////////

URI NTree::complete_relativepath(const URI & uri) const
{


  //QMutexLocker locker(m_mutex);

  cf3_assert(m_current_index.isValid());

  URI completedPath(uri);

  index_to_node(m_current_index)->complete_path(completedPath);

  return completedPath;
}

////////////////////////////////////////////////////////////////////////////

void NTree::content_listed(Component::Ptr node)
{


  //QMutexLocker locker(m_mutex);

  if(m_current_index.isValid() && index_to_node(m_current_index).get() == node.get())
  {
    emit current_index_changed(m_current_index, QModelIndex());
  }
}

////////////////////////////////////////////////////////////////////////////

void NTree::update_tree()
{
  SignalFrame frame("list_tree", CLIENT_TREE_PATH, SERVER_ROOT_PATH);
  NetworkQueue::global()->send( frame );
}

/*============================================================================

                             PRIVATE METHODS

============================================================================*/

void NTree::build_node_path_recursive(const QModelIndex & index, QString & path) const
{


  TreeNode * node = this->index_to_tree_node(index);

  if(node != nullptr)
  {
    path.prepend('/').prepend(node->node_name());
    this->build_node_path_recursive(index.parent(), path);
  }
//  else
//    path.prepend("//");
}

////////////////////////////////////////////////////////////////////////////

QString NTree::tool_tip() const
{


  return this->component_type();
}

////////////////////////////////////////////////////////////////////////////

bool NTree::node_matches_recursive(Component::Ptr node, const QRegExp regex) const
{


  bool match = QString(node->name().c_str()).contains(regex);
  ComponentIterator<CNode> it = component_begin<CNode>(*node);
  ComponentIterator<CNode> end = component_end<CNode>(*node);

  for( ; it != end ; it++)
    match |= (m_debug_mode_enabled || !it->is_local_component()) && this->node_matches_recursive(it.get(), regex);

  return match;
}

////////////////////////////////////////////////////////////////////////////

NTree::Ptr NTree::global()
{


  static NTree::Ptr tree = ThreadManager::instance().tree().root_child<NTree>(CLIENT_TREE);
  cf3_assert( tree.get() != nullptr );

  return tree;
}

////////////////////////////////////////////////////////////////////////////

TreeNode * NTree::index_to_tree_node(const QModelIndex & index) const
{


  return static_cast<TreeNode *>(index.internalPointer());
}

////////////////////////////////////////////////////////////////////////////

CNode::Ptr NTree::index_to_node(const QModelIndex & index) const
{
  return this->index_to_tree_node(index)->node();
}

////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3
