// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QMutex>

#include "rapidxml/rapidxml.hpp"

#include "Common/Signal.hpp"

#include "UI/Core/TreeThread.hpp"
#include "UI/Core/NetworkQueue.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/Core/NRoot.hpp"
#include "UI/Core/ThreadManager.hpp"
#include "UI/Core/TreeNode.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/NTree.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

/////////////////////////////////////////////////////////////////////////

NTree::NTree(NRoot::Ptr rootNode)
  : CNode(CLIENT_TREE, "NTree", CNode::DEBUG_NODE),
    m_advancedMode(false),
    m_debugModeEnabled(false)
{


  if(rootNode.get() == nullptr)
    m_rootNode = new TreeNode(ThreadManager::instance().tree().root(), nullptr, 0);
  else
    m_rootNode = new TreeNode(rootNode, nullptr, 0);

  m_mutex = new QMutex();

  m_columns << "Name" << "Type";

  unregist_signal("list_tree"); // unregister base class signal

  regist_signal( "list_tree" )
    ->description("New tree")
    ->pretty_name("")->connect(boost::bind(&NTree::list_tree_reply, this, _1));
}

////////////////////////////////////////////////////////////////////////////

void NTree::setRoot(NRoot::Ptr rootNode)
{


  //QMutexLocker locker(m_mutex);

  // initiate the removing process
  emit layoutAboutToBeChanged();
  delete m_rootNode;

  m_rootNode = nullptr;

  if(rootNode.get() != nullptr)
    m_rootNode = new TreeNode(rootNode, nullptr, 0);

  emit layoutChanged();
}

////////////////////////////////////////////////////////////////////////////

NRoot::Ptr NTree::treeRoot() const
{


  return m_rootNode->node()->castTo<NRoot>();
}

////////////////////////////////////////////////////////////////////////////

void NTree::setCurrentIndex(const QModelIndex & newIndex)
{


  //QMutexLocker locker(m_mutex);

  if(!this->areFromSameNode(m_currentIndex, newIndex))
  {
    QModelIndex oldIndex = m_currentIndex;
    m_currentIndex = newIndex;
    emit currentIndexChanged(newIndex, oldIndex);
  }
}

////////////////////////////////////////////////////////////////////////////

QModelIndex NTree::currentIndex() const
{


  return m_currentIndex;
}

////////////////////////////////////////////////////////////////////////////

URI NTree::currentPath() const
{


  return pathFromIndex( m_currentIndex );
}

////////////////////////////////////////////////////////////////////////////

void NTree::listNodeOptions(const QModelIndex & index,
                           QList<Option::ConstPtr> & options,
                           bool * ok) const
{


  //QMutexLocker locker(m_mutex);

  TreeNode * node = this->indexToTreeNode(index);

  if(ok != nullptr)
    *ok = node != nullptr;

  options.clear();

  if(node != nullptr && node->node().get() != nullptr)
    node->node()->listOptions(options);
}

////////////////////////////////////////////////////////////////////////////

void NTree::listNodeProperties(const QModelIndex &index,
                              QMap<QString, QString> &props, bool *ok) const
{


  //QMutexLocker locker(m_mutex);

  TreeNode * node = this->indexToTreeNode(index);

  if(ok != nullptr)
    *ok = node != nullptr;

  props.clear();

  if(node != nullptr && node->node().get() != nullptr)
    node->node()->listProperties(props);
}

////////////////////////////////////////////////////////////////////////////

void NTree::listNodeActions(const QModelIndex & index, QList<ActionInfo> & actions,
                           bool * ok) const
{


  //QMutexLocker locker(m_mutex);

  TreeNode * node = this->indexToTreeNode(index);

  if(ok != nullptr)
    *ok = node != nullptr;

  actions.clear();

  if(node != nullptr && node->node().get() != nullptr)
    node->node()->listSignals(actions);
}

////////////////////////////////////////////////////////////////////////////

QString NTree::nodePath(const QModelIndex & index) const
{


  //QMutexLocker locker(m_mutex);

  QString path;

  this->buildNodePathRec(index, path);

  return path;
}

////////////////////////////////////////////////////////////////////////////

void NTree::setAdvancedMode(bool advanceMode)
{


  //QMutexLocker locker(m_mutex);

  if(m_advancedMode ^ advanceMode) // if values are different
  {
    /// @todo find a better way to refresh the tree
    emit layoutAboutToBeChanged();
    m_advancedMode = advanceMode;
    emit advancedModeChanged(m_advancedMode);
    emit layoutChanged();
  }
}

////////////////////////////////////////////////////////////////////////////

bool NTree::isAdvancedMode() const
{


  return m_advancedMode;
}

////////////////////////////////////////////////////////////////////////////

bool NTree::areFromSameNode(const QModelIndex & left, const QModelIndex & right) const
{


  return left.isValid() && left.internalPointer() == right.internalPointer();
}

////////////////////////////////////////////////////////////////////////////

CNode::ConstPtr NTree::nodeByPath(const URI & path) const
{


  //QMutexLocker locker(m_mutex);

  QString pathStr = path.path().c_str();
  QStringList comps;
  QStringList::iterator it;
  CNode::ConstPtr node = m_rootNode->node();

  if(path.is_absolute())
  {
    comps = pathStr.split(URI::separator().c_str(), QString::SkipEmptyParts);

    if(comps.first().toStdString() == node->name())
      comps.removeFirst();

    for(it = comps.begin() ; it != comps.end() && node.get() != nullptr ; it++)
    {
      Component::ConstPtr comp = node->realComponent()->get_child_ptr(it->toStdString());

      if( is_not_null(comp.get()) )
        node = comp->as_ptr_checked<CNode>();
      else
        node = CNode::ConstPtr();
    }
  }

  return node;
}

////////////////////////////////////////////////////////////////////////////

CNode::Ptr NTree::nodeByPath(const URI & path)
{


  //QMutexLocker locker(m_mutex);

  QString pathStr = path.path().c_str();
  QStringList comps;
  QStringList::iterator it;
  CNode::Ptr node = m_rootNode->node();

  if(path.is_absolute())
  {
    comps = pathStr.split(URI::separator().c_str(), QString::SkipEmptyParts);

    if(comps.first().toStdString() == node->name())
      comps.removeFirst();

    for(it = comps.begin() ; it != comps.end() && node.get() != nullptr ; it++)
    {
      Component::Ptr comp = node->realComponent()->get_child_ptr(it->toStdString());

      if( is_not_null(comp.get()) )
        node = comp->as_ptr_checked<CNode>();
      else
        node = CNode::Ptr();
    }
  }

  return node;
}

////////////////////////////////////////////////////////////////////////////

QModelIndex NTree::indexFromPath(const URI & path) const
{


  QModelIndex index = this->index(0,0);
  QString pathStr = path.path().c_str();
  QStringList comps;
  QStringList::iterator it;
  TreeNode * treeNode = m_rootNode;

  cf_assert( path.scheme() == URI::Scheme::CPATH );
  cf_assert( treeNode != nullptr );

  if(path.is_absolute())
  {
    comps = pathStr.split(URI::separator().c_str(), QString::SkipEmptyParts);

    if(comps.first() == treeNode->nodeName())
      comps.removeFirst();

    for(it = comps.begin() ; it != comps.end() && treeNode != nullptr ; it++)
    {
      treeNode = treeNode->childByName(*it);

      if(treeNode != nullptr)
        index = this->index(treeNode->rowNumber(), 0, index);
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

  TreeNode * treeNode = this->indexToTreeNode(index);
  URI path;

  if(treeNode != nullptr)
    path = treeNode->node()->realComponent()->uri();

  return path;
}

////////////////////////////////////////////////////////////////////////////

void NTree::setDebugModeEnabled(bool debugMode)
{


  //QMutexLocker locker(m_mutex);

  if(m_debugModeEnabled ^ debugMode)
  {
    emit layoutAboutToBeChanged();
    m_debugModeEnabled = debugMode;
    emit layoutChanged();
  }
}

////////////////////////////////////////////////////////////////////////////

bool NTree::isDebugModeEnabled() const
{


  return m_debugModeEnabled;
}

////////////////////////////////////////////////////////////////////////////

void NTree::updateRootChildren()
{


  //QMutexLocker locker(m_mutex);

  emit layoutAboutToBeChanged();
  m_rootNode->updateChildList();
  emit layoutChanged();
}

////////////////////////////////////////////////////////////////////////////

void NTree::optionsChanged(const URI & path)
{


  QModelIndex index = this->indexFromPath(path);

  if(index.isValid())
    emit dataChanged(index, index);
}

////////////////////////////////////////////////////////////////////////////

bool NTree::nodeMatches(const QModelIndex & index, const QRegExp & regex) const
{


  //QMutexLocker locker(m_mutex);

  Component::Ptr node = m_rootNode->node()->castTo<NRoot>()->root();

  // if the index is value, we get the right node
  if(index.isValid() && indexToTreeNode(index) != m_rootNode)
    node = indexToNode(index);

  if(node.get() != nullptr)
    return this->nodeMatchesRec(node, regex);
  else
    return false;
}

////////////////////////////////////////////////////////////////////////////

bool NTree::isIndexVisible(const QModelIndex & index) const
{


  //QMutexLocker locker(m_mutex);

  bool visible = false;

  if(index.isValid())
  {
    CNode::Ptr node = this->indexToNode(index);

    if( is_not_null(node.get()) )
    {
      // below, true if node is basic or if we are in advanced mode
      visible = node->has_tag("basic") || m_advancedMode;
      // below, true if node is not of debug type or if we are in debug mode
      visible &= (node->type() != CNode::DEBUG_NODE) || m_debugModeEnabled;
    }

  }

  return visible;
}

////////////////////////////////////////////////////////////////////////////

void NTree::modifyOptions(const QModelIndex & index,
                          const QMap<QString, QString> & options)
{


  //QMutexLocker locker(m_mutex);

  TreeNode * node = this->indexToTreeNode(index);

  if(node != nullptr)
    node->node()->modifyOptions(options);
  else
    NLog::globalLog()->addError("Could not modify options! Invalid node.");
}

////////////////////////////////////////////////////////////////////////////

QVariant NTree::data(const QModelIndex & index, int role) const
{


  QVariant data;

  if(isIndexVisible(index))
  {
    //QMutexLocker locker(m_mutex);
    CNode::Ptr node = this->indexToNode(index);

    if(role == Qt::DisplayRole)
    {
      switch(index.column())
      {
      case 0:
        data = QString(node->name().c_str());
        break;
      case 1:
        data = QString(node->componentType());
        break;
      }
    }
    else if(role == Qt::ToolTipRole)
      data = node->toolTip();
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
      childNode = m_rootNode;
    else
      childNode = this->indexToTreeNode(parent)->child(row);

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
    TreeNode * parentNode = this->indexToTreeNode(child)->parentNode();

    if (parentNode != nullptr)
      index = createIndex(parentNode->rowNumber(), 0, parentNode);
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

  TreeNode* tree_node = this->indexToTreeNode(parent);
  if(is_not_null(tree_node))
    return tree_node->childCount();

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
  emit beginResetModel();

  try
  {
    NRoot::Ptr treeRoot = m_rootNode->node()->castTo<NRoot>();
    NRoot::Ptr rootNode = CNode::createFromXml(args.main_map.content.content->first_node())->castTo<NRoot>();
    ComponentIterator<CNode> it = rootNode->root()->begin<CNode>();
    URI currentIndexPath;

    if(m_currentIndex.isValid())
    {
      currentIndexPath = indexToTreeNode(m_currentIndex)->node()->uri();
    }



    //
    // rename the root
    //
    treeRoot->rename(rootNode->root()->name());
    treeRoot->root()->rename(rootNode->root()->name());

    //
    // remove old nodes
    //
    ComponentIterator<CNode> itRem = treeRoot->root()->begin<CNode>();
    QList<std::string> listToRemove;
    QList<std::string>::iterator itList;

    for( ; itRem != treeRoot->root()->end<CNode>() ; itRem++)
    {
      if(!itRem->isLocalComponent() && !itRem->isRoot() )
        listToRemove << itRem->name();
    }

    itList = listToRemove.begin();

    for( ; itList != listToRemove.end() ; itList++)
      treeRoot->root()->remove_component(*itList);

    //
    // add the new nodes
    //

    for( ; it != rootNode->root()->end<CNode>() ; it++)
      treeRoot->root()->add_component(it.get());

    // child count may have changed, ask the root TreeNode to update its internal data
    m_rootNode->updateChildList();

    // retrieve the previous index, if it still exists
    if(!currentIndexPath.path().empty())
      m_currentIndex = this->indexFromPath(currentIndexPath);


    NLog::globalLog()->addMessage("Tree updated.");
  }
  catch(XmlError & xe)
  {
    NLog::globalLog()->addException(xe.what());
  }

  // tell the view to update the whole thing
  emit endResetModel();

  emit currentIndexChanged(m_currentIndex, QModelIndex());

//  qDebug() << "tree updated !";
}

////////////////////////////////////////////////////////////////////////////

void NTree::clearTree()
{


  //QMutexLocker locker(m_mutex);

  NRoot::Ptr treeRoot = m_rootNode->node()->castTo<NRoot>();
  ComponentIterator<CNode> itRem = treeRoot->root()->begin<CNode>();
  QMap<int, std::string> listToRemove;
  QMutableMapIterator<int, std::string> itList(listToRemove);

  for(int i = 0 ; itRem != treeRoot->root()->end<CNode>() ; itRem++, i++)
  {
    if(!itRem->isLocalComponent())
      listToRemove[i] = itRem->name();
  }

  itList.toBack();

  for( ; itList.hasPrevious() ; )
  {
    itList.previous();

    emit beginRemoveRows(index(0,0), itList.key(), itList.key());
    treeRoot->root()->remove_component(itList.value());
    m_rootNode->updateChildList();
    emit endRemoveRows();
  }
}

////////////////////////////////////////////////////////////////////////////

URI NTree::completeRelativePath(const URI & uri) const
{


  //QMutexLocker locker(m_mutex);

  cf_assert(m_currentIndex.isValid());

  URI completedPath(uri);

  indexToNode(m_currentIndex)->complete_path(completedPath);

  return completedPath;
}

////////////////////////////////////////////////////////////////////////////

void NTree::contentListed(Component::Ptr node)
{


  //QMutexLocker locker(m_mutex);

  if(m_currentIndex.isValid() && indexToNode(m_currentIndex).get() == node.get())
  {
    emit currentIndexChanged(m_currentIndex, QModelIndex());
  }
}

////////////////////////////////////////////////////////////////////////////

void NTree::updateTree()
{
  SignalFrame frame("list_tree", CLIENT_TREE_PATH, SERVER_ROOT_PATH);
  NetworkQueue::global_queue()->send( frame );
}

/*============================================================================

                             PRIVATE METHODS

============================================================================*/

void NTree::buildNodePathRec(const QModelIndex & index, QString & path) const
{


  TreeNode * node = this->indexToTreeNode(index);

  if(node != nullptr)
  {
    path.prepend('/').prepend(node->nodeName());
    this->buildNodePathRec(index.parent(), path);
  }
//  else
//    path.prepend("//");
}

////////////////////////////////////////////////////////////////////////////

QString NTree::toolTip() const
{


  return this->componentType();
}

////////////////////////////////////////////////////////////////////////////

bool NTree::nodeMatchesRec(Component::Ptr node, const QRegExp regex) const
{


  bool match = QString(node->name().c_str()).contains(regex);
  ComponentIterator<CNode> it = node->begin<CNode>();

  for( ; it != node->end<CNode>() ; it++)
    match |= (m_debugModeEnabled || !it->isLocalComponent()) && this->nodeMatchesRec(it.get(), regex);

  return match;
}

////////////////////////////////////////////////////////////////////////////

NTree::Ptr NTree::globalTree()
{


  static NTree::Ptr tree = ThreadManager::instance().tree().rootChild<NTree>(CLIENT_TREE);
  cf_assert( tree.get() != nullptr );

  return tree;
}

////////////////////////////////////////////////////////////////////////////

TreeNode * NTree::indexToTreeNode(const QModelIndex & index) const
{


  return static_cast<TreeNode *>(index.internalPointer());
}

////////////////////////////////////////////////////////////////////////////

CNode::Ptr NTree::indexToNode(const QModelIndex & index) const
{


  return this->indexToTreeNode(index)->node();
}

////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF
