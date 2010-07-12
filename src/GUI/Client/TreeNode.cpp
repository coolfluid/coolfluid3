#include <QtCore>

#include "GUI/Client/NRoot.hpp"

#include "GUI/Client/TreeNode.hpp"

using namespace CF::GUI::Client;

TreeNode::TreeNode(CNode::Ptr node, TreeNode * parent, int rowNumber)
  : m_node(node),
    m_parent(parent),
    m_rowNumber(rowNumber)
{
  cf_assert(node.get() != CFNULL);
  cf_assert(rowNumber >= 0);

  this->updateChildList();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TreeNode::hasParent() const
{
  return m_parent != CFNULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TreeNode * TreeNode::getChild(int rowNumber)
{
  TreeNode * child = CFNULL;
  // if the TreeNode corresponding to this child has already been created,
  // it is returned...
  if (rowNumber >= 0 && rowNumber < m_childNodes.size())
    child = m_childNodes.at(rowNumber);

  // ...otherwise, if the index is valid, it is created and returned...
  if(child == CFNULL && rowNumber>= 0 && rowNumber < m_childNodes.size())
  {
    CNode::Ptr childNode;

    if(m_node->checkType(CNode::ROOT_NODE))
      childNode = CNode::convertTo<NRoot>(m_node)->getNodeFromRoot(rowNumber);
    else
      childNode = m_node->getNode(rowNumber);

    child = new TreeNode(childNode, this, rowNumber);
    m_childNodes.replace(rowNumber, child);
  }

  // ...if the index is not valid, return a CFNULL pointer
  return child;

//  cf_assert(index < m_childNodes.size());

//  return m_childNodes.at(index);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr TreeNode::getNode() const
{
  return m_node;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TreeNode * TreeNode::getParent() const
{
  return m_parent;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int TreeNode::getRowNumber() const
{
  return m_rowNumber;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int TreeNode::getChildCount() const
{
  if(m_node->checkType(CNode::ROOT_NODE))
    return CNode::convertTo<NRoot>(m_node)->root()->get_child_count();

  return m_node->get_child_count();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TreeNode * TreeNode::getChildByName(const QString & name)
{
  TreeNode * treeNode = CFNULL;
  bool found = false;

  /// @todo find a better algorithm !!!
  for(int i = 0 ; i < m_childNodes.count() && !found ; i++)
  {
    treeNode = this->getChild(i);
    found = treeNode->getName() == name;
  }

  if(!found)
    treeNode = CFNULL;

  return treeNode;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeNode::updateChildList()
{
  int childCount;

  while(!m_childNodes.isEmpty())
    delete m_childNodes.takeFirst();

  m_childNodes.clear();

  if(m_node->checkType(CNode::ROOT_NODE))
    childCount = CNode::convertTo<NRoot>(m_node)->root()->get_child_count();
  else
    childCount = m_node->get_child_count();

  for(int i = 0 ; i < childCount ; i++)
    m_childNodes << CFNULL;
}

