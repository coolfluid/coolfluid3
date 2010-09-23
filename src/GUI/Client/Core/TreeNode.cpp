// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "GUI/Client/Core/NRoot.hpp"

#include "GUI/Client/Core/TreeNode.hpp"

using namespace CF::GUI::ClientCore;

TreeNode::TreeNode(CNode::Ptr node, TreeNode * parent, int rowNumber)
  : m_node(node),
    m_parent(parent),
    m_rowNumber(rowNumber)
{
  cf_assert(node.get() != CFNULL);
  cf_assert(rowNumber >= 0);

  m_node->connectNotifier(this, SIGNAL(childCountChanged()), SLOT(updateChildList()));

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

    childNode = m_node->getNode(rowNumber);

    child = new TreeNode(childNode, this, rowNumber);
    m_childNodes.replace(rowNumber, child);
  }

  // ...if the index is not valid, return a CFNULL pointer
  return child;
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
    return m_node->convertTo<NRoot>()->root()->get_child_count();

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

  if(m_node->checkType(CNode::ROOT_NODE))
    childCount = m_node->convertTo<NRoot>()->root()->get_child_count();
  else
    childCount = m_node->get_child_count();

  for(int i = 0 ; i < childCount ; i++)
    m_childNodes << CFNULL;
}

