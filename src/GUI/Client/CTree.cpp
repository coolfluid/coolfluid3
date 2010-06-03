#include <QtCore>
#include <QtGui>

#include "Common/CF.hpp"

#include "GUI/Client/CNode.hpp"
#include "GUI/Client/TreeNode.hpp"

#include "GUI/Client/CTree.hpp"

#define TO_CNODE_PTR(a) boost::dynamic_pointer_cast<CNode>(a).get()
#define INDEX_TO_TREE_NODE(a) static_cast<TreeNode *>(a.internalPointer())
#define INDEX_TO_NODE(a) INDEX_TO_TREE_NODE(a)->getNode()

using namespace CF::GUI::Client;

CTree::CTree(CNode::Ptr rootNode)
{
  cf_assert(rootNode.get() != CFNULL);

  m_rootNode = new TreeNode(rootNode, CFNULL, 0);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant CTree::data(const QModelIndex & index, int role) const
{
  QVariant data;

  if(index.isValid())
  {
    CNode::Ptr node = INDEX_TO_NODE(index);

    if(role == Qt::DisplayRole)
    {
      switch(index.column())
      {
      case 0:
        data = node->name().c_str();
        break;
      }
    }
    else
    {
      if(role == Qt::DecorationRole)
        data = node->getIcon();
    }
  }

  return data;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QModelIndex CTree::index(int row, int column, const QModelIndex & parent) const
{
  TreeNode * childNode;
  TreeNode * parentNode;
  QModelIndex index;

  if(this->hasIndex(row, column, parent))
  {
    if(!parent.isValid())
      parentNode = m_rootNode;
    else
      parentNode = INDEX_TO_TREE_NODE(parent);

    childNode = parentNode->getChild(row);

    if(childNode != CFNULL)
      index = createIndex(row, column, childNode);
  }
  return index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QModelIndex CTree::parent(const QModelIndex &child) const
{
  QModelIndex index;

  if(child.isValid())
  {
    TreeNode * childNode = INDEX_TO_TREE_NODE(child);
    TreeNode * parentNode = childNode->getParent();

    if (parentNode != CFNULL && parentNode != m_rootNode)
      index = createIndex(parentNode->getRowNumber(), 0, parentNode);
  }

  return index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int CTree::rowCount(const QModelIndex & parent) const
{
  TreeNode * parentItem;

  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = m_rootNode;
  else
    parentItem = INDEX_TO_TREE_NODE(parent);

  return parentItem->getNode()->getNodeCount();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int CTree::columnCount(const QModelIndex & parent) const
{
  return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant CTree::headerData(int section, Qt::Orientation orientation,
                           int role) const
{
  return QVariant();
}

/*============================================================================

                             PRIVATE METHODS

============================================================================*/

