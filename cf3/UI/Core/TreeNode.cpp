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
    m_row_number(rowNumber)
{
  cf3_assert(node.get() != nullptr);
  cf3_assert(rowNumber >= 0);

  m_node.lock()->connect_notifier(this, SIGNAL(child_count_changed()), SLOT(update_child_list()));

  this->update_child_list();
}

////////////////////////////////////////////////////////////////////////////

TreeNode::~TreeNode()
{
  if( is_not_null( m_parent ) )
    m_parent->remove_child( this );

  while(!m_child_nodes.isEmpty())
    delete m_child_nodes.takeLast();
}

////////////////////////////////////////////////////////////////////////////

bool TreeNode::has_parent() const
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
    if (rowNumber >= 0 && rowNumber < child_count())
      child = m_child_nodes.at(rowNumber);

    // ...otherwise, if the index is valid, it is created and returned...
    if(child == nullptr && rowNumber>= 0 && rowNumber < child_count())
    {
      CNode::Ptr childNode;

      childNode = m_node.lock()->child(rowNumber);

      child = new TreeNode(childNode, this, rowNumber);
      m_child_nodes.replace(rowNumber, child);
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

TreeNode * TreeNode::parent_node() const
{
  return m_parent;
}

////////////////////////////////////////////////////////////////////////////

int TreeNode::row_number() const
{
  return m_row_number;
}

////////////////////////////////////////////////////////////////////////////

int TreeNode::child_count() const
{
  if( !m_node.expired() )
    return m_node.lock()->real_component()->count_children();
  else
    return 0;
}

////////////////////////////////////////////////////////////////////////////

TreeNode * TreeNode::child_by_name(const QString & name)
{
  TreeNode * treeNode = nullptr;
  bool found = false;

  /// @todo find a better algorithm !!!
  for(int i = 0 ; i < child_count() && !found ; i++)
  {
    treeNode = this->child(i);

    if(treeNode != nullptr)
      found = treeNode->node_name() == name;
  }

  if(!found)
    treeNode = nullptr;

  return treeNode;
}

////////////////////////////////////////////////////////////////////////////

void TreeNode::update_child_list()
{
  int childCount = this->child_count();

  while(!m_child_nodes.isEmpty())
    delete m_child_nodes.takeFirst();

  for(int i = 0 ; i < childCount ; i++)
    m_child_nodes << nullptr;
}

////////////////////////////////////////////////////////////////////////////

void TreeNode::remove_child( TreeNode *child )
{
  m_child_nodes.removeAll( child );
}

////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3
