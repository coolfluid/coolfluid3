#include <QtCore>
#include <QtGui>

#include "Common/CF.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/CNode.hpp"
#include "GUI/Client/NRoot.hpp"
#include "GUI/Client/TreeNode.hpp"
#include "GUI/Client/NLink.hpp"

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/NTree.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

NTree::NTree(CNode::Ptr rootNode)
  : CNode(CLIENT_TREE, "NTree", CNode::TREE_NODE),
    m_advancedMode(false),
    m_debugModeEnabled(false)
{
  BUILD_COMPONENT;

  if(rootNode.get() == CFNULL)
    m_rootNode = new TreeNode(ClientRoot::getRoot(), CFNULL, 0);
  else
    m_rootNode = new TreeNode(rootNode, CFNULL, 0);

  m_columns << "Name" << "Type";

  regist_signal("list_tree", "New tree")->connect(boost::bind(&NTree::list_tree, this, _1));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::setRoot(NRoot::Ptr rootNode)
{
  // initiate the removing process
  emit layoutAboutToBeChanged();
  delete m_rootNode;

  m_rootNode = CFNULL;

  if(rootNode.get() != CFNULL)
    m_rootNode = new TreeNode(rootNode, CFNULL, 0);

  emit layoutChanged();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NRoot::Ptr NTree::getRoot() const
{
  return CNode::convertTo<NRoot>(m_rootNode->getNode());
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

void NTree::getNodeParams(const QModelIndex & index, QList<NodeOption> & params,
                          bool * ok) const
{
  TreeNode * node = this->indexToTreeNode(index);

  if(ok != CFNULL)
    *ok = node != CFNULL;

  params.clear();

  if(node != CFNULL && node->getNode().get() != CFNULL)
    node->getNode()->getOptions(params);
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
  if(m_advancedMode != advanceMode)
  {
    m_advancedMode = advanceMode;
    emit advancedModeChanged(m_advancedMode);
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

bool NTree::haveSameData(const QModelIndex & left, const QModelIndex & right) const
{
  bool sameData = false;
  TreeNode * leftTreeNode = this->indexToTreeNode(left);
  TreeNode * rightTreeNode = this->indexToTreeNode(right);

  if(leftTreeNode != CFNULL && rightTreeNode != CFNULL)
  {
    CNode::Ptr leftNode = leftTreeNode->getNode();
    CNode::Ptr rightNode = rightTreeNode->getNode();

    if(leftNode->checkType(CNode::LINK_NODE))
    {
      sameData = CNode::convertTo<NLink>(leftNode)->getTargetPath().string() == QString("//%1").arg(rightNode->full_path().string().c_str()).toStdString();
    }
    else if(rightNode->checkType(CNode::LINK_NODE))
    {
      sameData = CNode::convertTo<NLink>(rightNode)->getTargetPath().string() == QString("//%1").arg(leftNode->full_path().string().c_str()).toStdString();
    }
  }

  return sameData;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr NTree::getNodeByPath(const CPath & path) const
{
  QString pathStr = path.string().c_str();
  QStringList comps;
  QStringList::iterator it;
  CNode::Ptr node = m_rootNode->getNode();

  if(!path.is_absolute())
    ClientRoot::getLog()->addError(QString("\"%1\" is not an absolute path").arg(pathStr));
  else
  {
    comps = pathStr.split(CPath::separator().c_str(), QString::SkipEmptyParts);

    if(comps.first().toStdString() == node->name())
      comps.removeFirst();

    for(it = comps.begin() ; it != comps.end() && node.get() != CFNULL ; it++)
    {
      if(node->checkType(CNode::ROOT_NODE))
        node = boost::dynamic_pointer_cast<CNode>(CNode::convertTo<NRoot>(node)->root()->get_child(it->toStdString()));
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

  cf_assert(treeNode != CFNULL);

  /// @todo find a better algorithm !!!
  if(!path.is_absolute())
    ClientRoot::getLog()->addError(QString("\"%1\" is not an absolute path").arg(pathStr));
  else
  {
    comps = pathStr.split(CPath::separator().c_str(), QString::SkipEmptyParts);

    if(comps.first() == treeNode->getName())
      comps.removeFirst();

    for(it = comps.begin() ; it != comps.end() && treeNode != CFNULL ; it++)
    {
      treeNode = treeNode->getChildByName(*it);

      if(treeNode != CFNULL)
        index = this->index(treeNode->getRowNumber(), 0, index);
      else
        ClientRoot::getLog()->addError("index not found");
    }
  }

  return index;
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

void NTree::modifyOptions(const QModelIndex & index,
                          const QHash<QString, QString> & options)
{
  TreeNode * node = this->indexToTreeNode(index);

  if(node != CFNULL)
    node->getNode()->modifyOptions(options);
  else
    ClientRoot::getLog()->addError("Could not modify options! Invalid node.");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant NTree::data(const QModelIndex & index, int role) const
{
  QVariant data;

  if(index.isValid())
  {
    CNode::Ptr node = this->indexToNode(index);

    if(m_debugModeEnabled || !node->isClientComponent())
    {
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
      else
      {
        if(role == Qt::DecorationRole && index.column() == 0)
          data = node->getIcon();

        if(role == Qt::ToolTipRole)
          data = node->getToolTip();
      }
    }
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

    if(childNode != CFNULL)
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

    if (parentNode != CFNULL)
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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::showNodeMenu(const QModelIndex & index, const QPoint & pos) const
{
  TreeNode * treeNode = indexToTreeNode(index);

  cf_assert(treeNode != CFNULL);

  if(treeNode != CFNULL)
    treeNode->getNode()->showContextMenu(pos);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::list_tree(XmlNode & node)
{
  NRoot::Ptr treeRoot = ClientRoot::getRoot();
  CNode::Ptr rootNode = CNode::createFromXml(*node.first_node());
  ComponentIterator<CNode> it = rootNode->begin<CNode>();

  /// @todo delete old nodes

  //
  // rename the root
  //
  QModelIndex rootIndex = index(0, 0);
  ClientRoot::getRoot()->rename(rootNode->name());
  ClientRoot::getRoot()->root()->rename(rootNode->name());
  emit dataChanged(rootIndex, rootIndex); // tell the view to update the node

  //
  // add the new nodes
  //
  int currentCount = treeRoot->root()->get_child_count();
  int newCount = treeRoot->root()->get_child_count();

  // tell the view that some nodes are about to be added (how many and where)
  emit beginInsertRows(rootIndex, currentCount, currentCount + newCount - 1);

  // add the nodes
  while(it != rootNode->end<CNode>())
  {
    treeRoot->root()->add_component(it.get());
    it++;
  }

  // child count has changed, ask the root TreeNode to update its internal data
  m_rootNode->updateChildList();

  // tell the view to update
  emit endInsertRows();
}

/*============================================================================

                             PRIVATE METHODS

============================================================================*/

void NTree::getNodePathRec(const QModelIndex & index, QString & path) const
{
  TreeNode * node = this->indexToTreeNode(index);

  if(node != CFNULL)
  {
    path.prepend('/').prepend(node->getName());
    this->getNodePathRec(index.parent(), path);
  }
  else
    path.prepend("//");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NTree::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::Folder);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NTree::getToolTip() const
{
  return this->getComponentType();
}
