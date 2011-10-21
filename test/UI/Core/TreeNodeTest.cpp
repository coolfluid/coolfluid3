// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtTest>

#include "common/BasicExceptions.hpp"

#include "UI/Core/NGeneric.hpp"
#include "UI/Core/NRoot.hpp"
#include "UI/Core/TreeNode.hpp"

#include "test/UI/Core/ExceptionThrowHandler.hpp"

#include "test/UI/Core/TreeNodeTest.hpp"

using namespace cf3::common;
using namespace cf3::UI::Core;

//////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace CoreTest {

//////////////////////////////////////////////////////////////////////////

void TreeNodeTest::test_constructor()
{
  // 1. the node is null
  GUI_CHECK_THROW( TreeNode(CNode::Ptr(), nullptr, 0), FailedAssertion );

  // 2. the row number is below 0
  NGeneric::Ptr node(new NGeneric("MyNode", "MyType"));
  GUI_CHECK_THROW( TreeNode(node, nullptr, -1), FailedAssertion);

  // 3. everything is OK
  GUI_CHECK_NO_THROW( TreeNode(node, nullptr, 0) );
}

//////////////////////////////////////////////////////////////////////////

void TreeNodeTest::test_hasParent()
{
  NGeneric::Ptr node(new NGeneric("MyNode", "MyType"));
  TreeNode parentLess(node, nullptr, 0);
  TreeNode withParent(node, &parentLess, 0);

  // 1. does not have a parent
  QVERIFY( !parentLess.has_parent() );

  // 2. has a parent
  QVERIFY( withParent.has_parent() );
}

//////////////////////////////////////////////////////////////////////////

void TreeNodeTest::test_child()
{
  /*

   The tree we will work with:

   Root
    |----> Node1
    |       |----> Node11
    |       |----> Node12
    |
    |----> Node2
    |       |----> Node21
    |       |----> Node22
    |       |----> Node23
    |
    |----> Node3


  */

  NRoot::Ptr node(new NRoot("Root"));
  NGeneric::Ptr node1(new NGeneric("Node1", "MyType"));
  NGeneric::Ptr node11(new NGeneric("Node11", "MyType"));
  NGeneric::Ptr node12(new NGeneric("Node12", "MyType"));
  NGeneric::Ptr node2(new NGeneric("Node2", "MyType"));
  NGeneric::Ptr node21(new NGeneric("Node21", "MyType"));
  NGeneric::Ptr node22(new NGeneric("Node22", "MyType"));
  NGeneric::Ptr node23(new NGeneric("Node23", "MyType"));
  NGeneric::Ptr node3(new NGeneric("Node3", "MyType"));

  TreeNode * treeNode = nullptr;
  TreeNode * treeNode1 = nullptr;
  TreeNode * treeNode11 = nullptr;
  TreeNode * treeNode12 = nullptr;
  TreeNode * treeNode2 = nullptr;
  TreeNode * treeNode21 = nullptr;
  TreeNode * treeNode22 = nullptr;
  TreeNode * treeNode23 = nullptr;
  TreeNode * treeNode3 = nullptr;

  node->add_node(node1);
  node->add_node(node2);
  node->add_node(node3);

  node1->add_node(node11);
  node1->add_node(node12);

  node2->add_node(node21);
  node2->add_node(node22);
  node2->add_node(node23);

  treeNode = new TreeNode(node, nullptr, 0);

  //
  // 1. check the direct children
  //
  treeNode1 = treeNode->child(0);
  treeNode2 = treeNode->child(1);
  treeNode3 = treeNode->child(2);

  // check that pointer are not null
  QVERIFY( treeNode1 != (TreeNode*) nullptr );
  QVERIFY( treeNode2 != (TreeNode*) nullptr );
  QVERIFY( treeNode3 != (TreeNode*) nullptr );

  // their parent should be the root
  QCOMPARE( treeNode1->parent_node(), treeNode );
  QCOMPARE( treeNode2->parent_node(), treeNode );
  QCOMPARE( treeNode3->parent_node(), treeNode );

  // check their name
  QCOMPARE( treeNode1->node_name(), QString("Node1") );
  QCOMPARE( treeNode2->node_name(), QString("Node2") );
  QCOMPARE( treeNode3->node_name(), QString("Node3") );

  //
  // 2. children of Node1
  //
  treeNode11 = treeNode1->child(0);
  treeNode12 = treeNode1->child(1);

  // check that pointer are not null
  QVERIFY( treeNode11 != (TreeNode*) nullptr );
  QVERIFY( treeNode12 != (TreeNode*) nullptr );

  // their parent should be the root
  QCOMPARE( treeNode11->parent_node(), treeNode1 );
  QCOMPARE( treeNode12->parent_node(), treeNode1 );

  // check their name
  QCOMPARE( treeNode11->node_name(), QString("Node11") );
  QCOMPARE( treeNode12->node_name(), QString("Node12") );

  //
  // 3. children of Node2
  //
  treeNode21 = treeNode2->child(0);
  treeNode22 = treeNode2->child(1);
  treeNode23 = treeNode2->child(2);

  // check that pointer are not null
  QVERIFY( treeNode21 != (TreeNode*) nullptr );
  QVERIFY( treeNode22 != (TreeNode*) nullptr );
  QVERIFY( treeNode23 != (TreeNode*) nullptr );

  // their parent should be the root
  QCOMPARE( treeNode21->parent_node(), treeNode2 );
  QCOMPARE( treeNode22->parent_node(), treeNode2 );
  QCOMPARE( treeNode23->parent_node(), treeNode2 );

  // check their name
  QCOMPARE( treeNode21->node_name(), QString("Node21") );
  QCOMPARE( treeNode22->node_name(), QString("Node22") );
  QCOMPARE( treeNode23->node_name(), QString("Node23") );

  //
  // 4. no child for Node3 (should return nullptr)
  //
  QCOMPARE( treeNode3->child(0), (TreeNode*) nullptr );

  delete treeNode; // deletes the children as well
}

//////////////////////////////////////////////////////////////////////////

void TreeNodeTest::test_childByName()
{
  // same tree as for test_child()

  NRoot::Ptr node(new NRoot("Root"));
  NGeneric::Ptr node1(new NGeneric("Node1", "MyType"));
  NGeneric::Ptr node11(new NGeneric("Node11", "MyType"));
  NGeneric::Ptr node12(new NGeneric("Node12", "MyType"));
  NGeneric::Ptr node2(new NGeneric("Node2", "MyType"));
  NGeneric::Ptr node21(new NGeneric("Node21", "MyType"));
  NGeneric::Ptr node22(new NGeneric("Node22", "MyType"));
  NGeneric::Ptr node23(new NGeneric("Node23", "MyType"));
  NGeneric::Ptr node3(new NGeneric("Node3", "MyType"));

  TreeNode * treeNode = nullptr;
  TreeNode * treeNode1 = nullptr;
  TreeNode * treeNode11 = nullptr;
  TreeNode * treeNode12 = nullptr;
  TreeNode * treeNode2 = nullptr;
  TreeNode * treeNode21 = nullptr;
  TreeNode * treeNode22 = nullptr;
  TreeNode * treeNode23 = nullptr;
  TreeNode * treeNode3 = nullptr;

  node->add_node(node1);
  node->add_node(node2);
  node->add_node(node3);

  node1->add_node(node11);
  node1->add_node(node12);

  node2->add_node(node21);
  node2->add_node(node22);
  node2->add_node(node23);

  treeNode = new TreeNode(node, nullptr, 0);
  treeNode1 = treeNode->child(0);
  treeNode11 = treeNode1->child(0);
  treeNode12 = treeNode1->child(1);
  treeNode2 = treeNode->child(1);
  treeNode21 = treeNode2->child(0);
  treeNode22 = treeNode2->child(1);
  treeNode23 = treeNode2->child(2);
  treeNode3 = treeNode->child(2);

  //
  // 1. check the direct children
  //
  treeNode1 = treeNode->child_by_name("Node1");
  treeNode2 = treeNode->child_by_name("Node2");
  treeNode3 = treeNode->child_by_name("Node3");

  // check that pointer are not null
  QVERIFY( treeNode1 != (TreeNode*) nullptr );
  QVERIFY( treeNode2 != (TreeNode*) nullptr );
  QVERIFY( treeNode3 != (TreeNode*) nullptr );

  // their parent should be the root
  QCOMPARE( treeNode1->parent_node(), treeNode );
  QCOMPARE( treeNode2->parent_node(), treeNode );
  QCOMPARE( treeNode3->parent_node(), treeNode );

  // check their row number
  QCOMPARE( treeNode1->row_number(), 0 );
  QCOMPARE( treeNode2->row_number(), 1 );
  QCOMPARE( treeNode3->row_number(), 2 );

  //
  // 2. children of Node1
  //
  treeNode11 = treeNode1->child_by_name("Node11");
  treeNode12 = treeNode1->child_by_name("Node12");

  // check that pointer are not null
  QVERIFY( treeNode11 != (TreeNode*) nullptr );
  QVERIFY( treeNode12 != (TreeNode*) nullptr );

  // their parent should be the root
  QCOMPARE( treeNode11->parent_node(), treeNode1 );
  QCOMPARE( treeNode12->parent_node(), treeNode1 );

  // check their row number
  QCOMPARE( treeNode11->row_number(), 0 );
  QCOMPARE( treeNode12->row_number(), 1 );

  //
  // 3. wrong name
  //
  QCOMPARE ( treeNode2->child_by_name("Node"), (TreeNode*) nullptr );


  delete treeNode; // deletes the children as well
}

//////////////////////////////////////////////////////////////////////////

void TreeNodeTest::test_updateChildList()
{
  NRoot::Ptr node(new NRoot("Root"));
  NGeneric::Ptr node1(new NGeneric("Node1", "MyType"));
  NGeneric::Ptr node2(new NGeneric("Node2", "MyType"));
  TreeNode * treeNode;
  TreeNode * child;

  node->add_node(node1);

  treeNode = new TreeNode(node, nullptr, 0);

  QCOMPARE( treeNode->child_count(), 1);

  node->add_node(node2);

  QCOMPARE( treeNode->child_count(), 2);

  child = treeNode->child(1);
  QVERIFY( child != nullptr );

  QCOMPARE( child->node_name(), QString("Node2"));

  delete treeNode;
}

//////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // cf3

