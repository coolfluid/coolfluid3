// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtTest>

#include "Common/BasicExceptions.hpp"

#include "UI/Core/NGeneric.hpp"
#include "UI/Core/NRoot.hpp"
#include "UI/Core/TreeNode.hpp"

#include "test/GUI/Client/Core/ExceptionThrowHandler.hpp"

#include "test/GUI/Client/Core/TreeNodeTest.hpp"

using namespace CF::Common;
using namespace CF::UI::Core;

//////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientTest {

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
  QVERIFY( !parentLess.hasParent() );

  // 2. has a parent
  QVERIFY( withParent.hasParent() );
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

  node->addNode(node1);
  node->addNode(node2);
  node->addNode(node3);

  node1->addNode(node11);
  node1->addNode(node12);

  node2->addNode(node21);
  node2->addNode(node22);
  node2->addNode(node23);

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
  QCOMPARE( treeNode1->parentNode(), treeNode );
  QCOMPARE( treeNode2->parentNode(), treeNode );
  QCOMPARE( treeNode3->parentNode(), treeNode );

  // check their name
  QCOMPARE( treeNode1->nodeName(), QString("Node1") );
  QCOMPARE( treeNode2->nodeName(), QString("Node2") );
  QCOMPARE( treeNode3->nodeName(), QString("Node3") );

  //
  // 2. children of Node1
  //
  treeNode11 = treeNode1->child(0);
  treeNode12 = treeNode1->child(1);

  // check that pointer are not null
  QVERIFY( treeNode11 != (TreeNode*) nullptr );
  QVERIFY( treeNode12 != (TreeNode*) nullptr );

  // their parent should be the root
  QCOMPARE( treeNode11->parentNode(), treeNode1 );
  QCOMPARE( treeNode12->parentNode(), treeNode1 );

  // check their name
  QCOMPARE( treeNode11->nodeName(), QString("Node11") );
  QCOMPARE( treeNode12->nodeName(), QString("Node12") );

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
  QCOMPARE( treeNode21->parentNode(), treeNode2 );
  QCOMPARE( treeNode22->parentNode(), treeNode2 );
  QCOMPARE( treeNode23->parentNode(), treeNode2 );

  // check their name
  QCOMPARE( treeNode21->nodeName(), QString("Node21") );
  QCOMPARE( treeNode22->nodeName(), QString("Node22") );
  QCOMPARE( treeNode23->nodeName(), QString("Node23") );

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

  node->addNode(node1);
  node->addNode(node2);
  node->addNode(node3);

  node1->addNode(node11);
  node1->addNode(node12);

  node2->addNode(node21);
  node2->addNode(node22);
  node2->addNode(node23);

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
  treeNode1 = treeNode->childByName("Node1");
  treeNode2 = treeNode->childByName("Node2");
  treeNode3 = treeNode->childByName("Node3");

  // check that pointer are not null
  QVERIFY( treeNode1 != (TreeNode*) nullptr );
  QVERIFY( treeNode2 != (TreeNode*) nullptr );
  QVERIFY( treeNode3 != (TreeNode*) nullptr );

  // their parent should be the root
  QCOMPARE( treeNode1->parentNode(), treeNode );
  QCOMPARE( treeNode2->parentNode(), treeNode );
  QCOMPARE( treeNode3->parentNode(), treeNode );

  // check their row number
  QCOMPARE( treeNode1->rowNumber(), 0 );
  QCOMPARE( treeNode2->rowNumber(), 1 );
  QCOMPARE( treeNode3->rowNumber(), 2 );

  //
  // 2. children of Node1
  //
  treeNode11 = treeNode1->childByName("Node11");
  treeNode12 = treeNode1->childByName("Node12");

  // check that pointer are not null
  QVERIFY( treeNode11 != (TreeNode*) nullptr );
  QVERIFY( treeNode12 != (TreeNode*) nullptr );

  // their parent should be the root
  QCOMPARE( treeNode11->parentNode(), treeNode1 );
  QCOMPARE( treeNode12->parentNode(), treeNode1 );

  // check their row number
  QCOMPARE( treeNode11->rowNumber(), 0 );
  QCOMPARE( treeNode12->rowNumber(), 1 );

  //
  // 3. wrong name
  //
  QCOMPARE ( treeNode2->childByName("Node"), (TreeNode*) nullptr );


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

  node->addNode(node1);

  treeNode = new TreeNode(node, nullptr, 0);

  QCOMPARE( treeNode->childCount(), 1);

  node->addNode(node2);

  QCOMPARE( treeNode->childCount(), 2);

  child = treeNode->child(1);
  QVERIFY( child != nullptr );

  QCOMPARE( child->nodeName(), QString("Node2"));

  delete treeNode;
}

//////////////////////////////////////////////////////////////////////////

} // ClientTest
} // GUI
} // CF

