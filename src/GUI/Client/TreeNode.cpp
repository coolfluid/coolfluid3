#include "GUI/Client/TreeNode.hpp"

using namespace CF::GUI::Client;

TreeNode::TreeNode(CNode::Ptr node, TreeNode * parent, int rowNumber)
  : m_node(node),
    m_parent(parent),
    m_rowNumber(rowNumber)
{
  for(int i = 0 ; i < m_node->getNodeCount() ; i++)
    m_childNodes << CFNULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TreeNode::hasParent() const
{
  return m_parent != CFNULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TreeNode * TreeNode::getChild(int index)
{
  cf_assert(index < m_childNodes.size());

  return m_childNodes.at(index);
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
