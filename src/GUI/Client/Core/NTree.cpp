// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFileIconProvider>
#include <QMutableMapIterator>
#include <QDebug>

#include "Common/CF.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/CNode.hpp"
#include "GUI/Client/Core/NRoot.hpp"
#include "GUI/Client/Core/TreeNode.hpp"
#include "GUI/Client/Core/NLink.hpp"

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/NTree.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;

NTree::NTree(NRoot::Ptr rootNode)
  : CNode(CLIENT_TREE, "NTree", CNode::TREE_NODE),
    m_advancedMode(false),
    m_debugModeEnabled(false)
{
  BUILD_COMPONENT;

  if(rootNode.get() == nullptr)
    m_rootNode = new TreeNode(ClientRoot::root(), nullptr, 0);
  else
    m_rootNode = new TreeNode(rootNode, nullptr, 0);

  m_columns << "Name" << "Type";

  regist_signal("list_tree", "New tree")->connect(boost::bind(&NTree::list_tree_reply, this, _1));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::setRoot(NRoot::Ptr rootNode)
{
  // initiate the removing process
  emit layoutAboutToBeChanged();
  delete m_rootNode;

  m_rootNode = nullptr;

  if(rootNode.get() != nullptr)
    m_rootNode = new TreeNode(rootNode, nullptr, 0);

  emit layoutChanged();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NRoot::Ptr NTree::getRoot() const
{
  return m_rootNode->getNode()->convertTo<NRoot>();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::setCurrentIndex(const QModelIndex & newIndex)
{
  if(!this->areFromSameNode(m_currentIndex, newIndex))
  {
    QModelIndex oldIndex = m_currentIndex;
    m_currentIndex = newIndex;
    emit currentIndexChanged(newIndex, oldIndex);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QModelIndex NTree::getCurrentIndex() const
{
  return m_currentIndex;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CPath NTree::getCurrentPath() const
{
  TreeNode * node = this->indexToTreeNode(m_currentIndex);
  CPath path;

  if(node != nullptr)
  {
    CNode::Ptr cnode = node->getNode();

    if(cnode->checkType(ROOT_NODE))
      path = cnode->convertTo<NRoot>()->root()->full_path();
    else
      path = cnode->full_path();
  }

  return path;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::getNodeOptions(const QModelIndex & index,
                           QList<Option::ConstPtr> & options,
                           bool * ok) const
{
  TreeNode * node = this->indexToTreeNode(index);

  if(ok != nullptr)
    *ok = node != nullptr;

  options.clear();

  if(node != nullptr && node->getNode().get() != nullptr)
    node->getNode()->getOptions(options);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::getNodeProperties(const QModelIndex &index,
                              QMap<QString, QString> &props, bool *ok) const
{
  TreeNode * node = this->indexToTreeNode(index);

  if(ok != nullptr)
    *ok = node != nullptr;

  props.clear();

  if(node != nullptr && node->getNode().get() != nullptr)
    node->getNode()->getProperties(props);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::getNodeActions(const QModelIndex & index, QList<ActionInfo> & actions,
                           bool * ok) const
{
  TreeNode * node = this->indexToTreeNode(index);

  if(ok != nullptr)
    *ok = node != nullptr;

  actions.clear();

  if(node != nullptr && node->getNode().get() != nullptr)
    node->getNode()->getActions(actions);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NTree::getNodePath(const QModelIndex & index) const
{
  QString path;

  this->getNodePathRec(index, path);

  return path;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::setAdvancedMode(bool advanceMode)
{
  if(m_advancedMode ^ advanceMode) // if values are different
  {
    /// @todo find a better way to refresh the tree
    emit layoutAboutToBeChanged();
    m_advancedMode = advanceMode;
    emit advancedModeChanged(m_advancedMode);
    emit layoutChanged();
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NTree::isAdvancedMode() const
{
  return m_advancedMode;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NTree::areFromSameNode(const QModelIndex & left, const QModelIndex & right) const
{
  return left.isValid() && left.internalPointer() == right.internalPointer();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr NTree::getNodeByPath(const CPath & path) const
{
  QString pathStr = path.string().c_str();
  QStringList comps;
  QStringList::iterator it;
  CNode::Ptr node = m_rootNode->getNode();

  if(path.is_absolute())
  {
    comps = pathStr.split(CPath::separator().c_str(), QString::SkipEmptyParts);

    if(comps.first().toStdString() == node->name())
      comps.removeFirst();

    for(it = comps.begin() ; it != comps.end() && node.get() != nullptr ; it++)
    {
      if(node->checkType(CNode::ROOT_NODE))
        node = boost::dynamic_pointer_cast<CNode>(node->convertTo<NRoot>()->root()->get_child(it->toStdString()));
      else
        node = boost::dynamic_pointer_cast<CNode>(node->get_child(it->toStdString()));
    }
  }

  return node;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QModelIndex NTree::getIndexByPath(const CPath & path) const
{
  QModelIndex index = this->index(0,0);
  QString pathStr = path.string().c_str();
  QStringList comps;
  QStringList::iterator it;
  TreeNode * treeNode = m_rootNode;

  cf_assert(treeNode != nullptr);

  if(path.is_absolute())
  {
    comps = pathStr.split(CPath::separator().c_str(), QString::SkipEmptyParts);

    if(comps.first() == treeNode->getName())
      comps.removeFirst();

    for(it = comps.begin() ; it != comps.end() && treeNode != nullptr ; it++)
    {
      treeNode = treeNode->getChildByName(*it);

      if(treeNode != nullptr)
        index = this->index(treeNode->getRowNumber(), 0, index);
      else
      {
        index = QModelIndex();
        ClientRoot::log()->addError("index not found");
      }
    }
  }

  return index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CPath NTree::getIndexPath(const QModelIndex & index) const
{
  TreeNode * treeNode = this->indexToTreeNode(index);
  CPath path;

  if(treeNode != nullptr)
  {
    CNode::Ptr node = treeNode->getNode();

    if(node->checkType(CNode::ROOT_NODE))
      path = node->convertTo<NRoot>()->root()->full_path();
    else
      path = treeNode->getNode()->full_path();
  }
  return path;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::setDebugModeEnabled(bool debugMode)
{
  if(m_debugModeEnabled ^ debugMode)
  {
    emit layoutAboutToBeChanged();
    m_debugModeEnabled = debugMode;
    emit layoutChanged();
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NTree::isDebugModeEnabled() const
{
  return m_debugModeEnabled;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::updateRootChildren()
{
  emit layoutAboutToBeChanged();
  m_rootNode->updateChildList();
  emit layoutChanged();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::optionsChanged(const CPath & path)
{
  QModelIndex index = this->getIndexByPath(path);

  if(index.isValid())
  {
    emit dataChanged(index, index);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NTree::nodeMatches(const QModelIndex & index, const QRegExp & regex) const
{
  Component::Ptr node = m_rootNode->getNode()->convertTo<NRoot>()->root();

  if(index.isValid() && indexToTreeNode(index) != m_rootNode)
    node = indexToNode(index);

  if(node.get() != nullptr)
    return this->nodeMatchesRec(node, regex);
  else
    return false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NTree::nodeIsVisible(const QModelIndex & index) const
{
  bool visible = false;

  if(index.isValid())
  {
    CNode::Ptr node = this->indexToNode(index);

    if(node.get() != nullptr)
    {
      visible = node->has_tag("basic") || m_advancedMode;
      visible &= (!node->isClientComponent() || m_debugModeEnabled);
    }
  }

  return visible;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::modifyOptions(const QModelIndex & index,
                          const QMap<QString, QString> & options)
{
  TreeNode * node = this->indexToTreeNode(index);

  if(node != nullptr)
    node->getNode()->modifyOptions(options);
  else
    ClientRoot::log()->addError("Could not modify options! Invalid node.");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant NTree::data(const QModelIndex & index, int role) const
{
  QVariant data;

  if(nodeIsVisible(index))
  {
    CNode::Ptr node = this->indexToNode(index);

    if(role == Qt::DisplayRole)
    {
      switch(index.column())
      {
      case 0:
        data = QString(node->name().c_str());
        break;
      case 1:
        data = QString(node->getComponentType());
        break;
      }
    }
    else if(role == Qt::ToolTipRole)
      data = node->getToolTip();
  }

  return data;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QModelIndex NTree::index(int row, int column, const QModelIndex & parent) const
{
  TreeNode * childNode;
  QModelIndex index;

  if(this->hasIndex(row, column, parent))
  {
    if( !parent.isValid())
      childNode = m_rootNode;
    else
      childNode = this->indexToTreeNode(parent)->getChild(row);

    if(childNode != nullptr)
      index = createIndex(row, column, childNode);
  }

  return index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QModelIndex NTree::parent(const QModelIndex &child) const
{
  QModelIndex index;

  if(child.isValid())
  {
    TreeNode * parentNode = this->indexToTreeNode(child)->getParent();

    if (parentNode != nullptr)
      index = createIndex(parentNode->getRowNumber(), 0, parentNode);
  }

  return index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int NTree::rowCount(const QModelIndex & parent) const
{
  if (parent.column() > 0)
    return 0;

  // if the parent is not valid, we have one child: the root
  if (!parent.isValid())
    return 1;

  return this->indexToTreeNode(parent)->getChildCount();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int NTree::columnCount(const QModelIndex & parent) const
{
  return m_columns.count();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant NTree::headerData(int section, Qt::Orientation orientation,
                           int role) const
{
  if(role == Qt::DisplayRole && orientation == Qt::Horizontal && section >= 0
     && section < m_columns.size())
    return m_columns.at(section);

  return QVariant();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::list_tree_reply(XmlNode & node)
{
  try
  {
    NRoot::Ptr treeRoot = m_rootNode->getNode()->convertTo<NRoot>();
    NRoot::Ptr rootNode = CNode::createFromXml(*node.first_node())->convertTo<NRoot>();
    ComponentIterator<CNode> it = rootNode->root()->begin<CNode>();
    CPath currentIndexPath;

    if(m_currentIndex.isValid())
    {
      currentIndexPath = indexToTreeNode(m_currentIndex)->getNode()->full_path();
    }

    emit beginResetModel();

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
      if(!itRem->isClientComponent())
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

    if(!currentIndexPath.string().empty())
      m_currentIndex = this->getIndexByPath(currentIndexPath);

    // tell the view to update the whole thing
    emit endResetModel();

    emit currentIndexChanged(m_currentIndex, QModelIndex());

    ClientRoot::log()->addMessage("Tree updated.");
  }
  catch(XmlError & xe)
  {
    ClientRoot::log()->addException(xe.what());
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::defineConfigProperties ( CF::Common::PropertyList& props )
{
  std::vector<int> vectInt;
  std::vector<bool> vectBool;
  std::vector<CF::Uint> vectUint;
  std::vector<CF::Real> vectReal;
  std::vector<std::string> vectString;

  props.add_option< OptionArrayT<int> >("intArray", "Array of ints", vectInt)->mark_basic();
  props.add_option< OptionArrayT<bool> >("boolArray", "Array of ints", vectBool)->mark_basic();
  props.add_option< OptionArrayT<CF::Uint> >("uIntArray", "Array of ints", vectUint)->mark_basic();
  props.add_option< OptionArrayT<CF::Real> >("realArray", "Array of ints", vectReal)->mark_basic();
  props.add_option< OptionArrayT<std::string> >("stringArray", "Array of ints", vectString)->mark_basic();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::clearTree()
{
  NRoot::Ptr treeRoot = m_rootNode->getNode()->convertTo<NRoot>();
  ComponentIterator<CNode> itRem = treeRoot->root()->begin<CNode>();
  QMap<int, std::string> listToRemove;
  QMutableMapIterator<int, std::string> itList(listToRemove);

  for(int i = 0 ; itRem != treeRoot->root()->end<CNode>() ; itRem++, i++)
  {
    if(!itRem->isClientComponent())
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

/*============================================================================

                             PRIVATE METHODS

============================================================================*/

void NTree::getNodePathRec(const QModelIndex & index, QString & path) const
{
  TreeNode * node = this->indexToTreeNode(index);

  if(node != nullptr)
  {
    path.prepend('/').prepend(node->getName());
    this->getNodePathRec(index.parent(), path);
  }
  else
    path.prepend("//");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NTree::getToolTip() const
{
  return this->getComponentType();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NTree::nodeMatchesRec(Component::Ptr node, const QRegExp regex) const
{
  bool match = QString(node->name().c_str()).contains(regex);
  ComponentIterator<CNode> it = node->begin<CNode>();

  for( ; it != node->end<CNode>() ; it++)
    match |= (m_debugModeEnabled || !it->isClientComponent()) && this->nodeMatchesRec(it.get(), regex);

  return match;
}
