// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "UI/Core/NRoot.hpp"

#include "UI/Core/TreeNode.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

  TreeNode::TreeNode(CNode::Ptr node, TreeNode * parent, int rowNumber)
  : m_node(node),
    m_parent(parent),
    m_rowNumber(rowNumber)
{
  cf3_assert(node.get() != nullptr);
  cf3_assert(rowNumber >= 0);

  m_node.lock()->connectNotifier(this, SIGNAL(childCountChanged()), SLOT(updateChildList()));

  this->updateChildList();
}

////////////////////////////////////////////////////////////////////////////

TreeNode::~TreeNode()
{
  while(!m_childNodes.isEmpty())
    delete m_childNodes.takeFirst();
}

////////////////////////////////////////////////////////////////////////////

bool TreeNode::hasParent() const
{
  return m_parent != nullptr;
}

////////////////////////////////////////////////////////////////////////////

TreeNode * TreeNode::child(int rowNumber)
{
  TreeNode * child = nullptr;

  if( !m_node.expired() )
  {
    // if the TreeNode corresponding to this child has already been created,
    // it is returned...
    if (rowNumber >= 0 && rowNumber < childCount())
      child = m_childNodes.at(rowNumber);

    // ...otherwise, if the index is valid, it is created and returned...
    if(child == nullptr && rowNumber>= 0 && rowNumber < childCount())
    {
      CNode::Ptr childNode;

      childNode = m_node.lock()->child(rowNumber);

      child = new TreeNode(childNode, this, rowNumber);
      m_childNodes.replace(rowNumber, child);
    }
  }

  // ...if the index is not valid, return a nullptr pointer
  return child;
}

////////////////////////////////////////////////////////////////////////////

CNode::Ptr TreeNode::node()
{
  cf3_assert( !m_node.expired() );
  return m_node.lock();
}

////////////////////////////////////////////////////////////////////////////

CNode::ConstPtr TreeNode::node() const
{
  cf3_assert( !m_node.expired() );
  return m_node.lock();
}

////////////////////////////////////////////////////////////////////////////

TreeNode * TreeNode::parentNode() const
{
  return m_parent;
}

////////////////////////////////////////////////////////////////////////////

int TreeNode::rowNumber() const
{
  return m_rowNumber;
}

////////////////////////////////////////////////////////////////////////////

int TreeNode::childCount() const
{
  if( !m_node.expired() )
    return m_node.lock()->realComponent()->count_children();
  else
    return 0;
}

////////////////////////////////////////////////////////////////////////////

TreeNode * TreeNode::childByName(const QString & name)
{
  TreeNode * treeNode = nullptr;
  bool found = false;

  /// @todo find a better algorithm !!!
  for(int i = 0 ; i < childCount() && !found ; i++)
  {
    treeNode = this->child(i);

    if(treeNode != nullptr)
      found = treeNode->nodeName() == name;
  }

  if(!found)
    treeNode = nullptr;

  return treeNode;
}

////////////////////////////////////////////////////////////////////////////

void TreeNode::updateChildList()
{
  int childCount = this->childCount();

  while(!m_childNodes.isEmpty())
    delete m_childNodes.takeFirst();

  for(int i = 0 ; i < childCount ; i++)
    m_childNodes << nullptr;
}

////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3
