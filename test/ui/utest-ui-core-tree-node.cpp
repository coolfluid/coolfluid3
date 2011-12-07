
// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the ui TreeNode class"

#include "ui/core/NGeneric.hpp"
#include "ui/core/NRoot.hpp"
#include "ui/core/TreeNode.hpp"

#include "test/ui/CoreApplication.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::ui::core;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( uiCoreNBrowserSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  application();

  AssertionManager::instance().AssertionDumps = false;
  AssertionManager::instance().AssertionThrows = true;
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionOutputs = false;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( contructor )
{
  // 1. the node is null
  BOOST_CHECK_THROW( TreeNode(Handle< CNode >(), nullptr, 0), FailedAssertion );

  // 2. the row number is below 0
  boost::shared_ptr< NGeneric > node(new NGeneric("MyNode", "MyType"));
  BOOST_CHECK_THROW( TreeNode(node->handle<CNode>(), nullptr, -1), FailedAssertion);

  // 3. everything is OK
  BOOST_CHECK_NO_THROW( TreeNode(node->handle<CNode>(), nullptr, 0) );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( has_parent )
{
  boost::shared_ptr< NGeneric > node(new NGeneric("MyNode", "MyType"));
  TreeNode parentLess(node->handle<CNode>(), nullptr, 0);
  TreeNode withParent(node->handle<CNode>(), &parentLess, 0);

  // 1. does not have a parent
  BOOST_CHECK( !parentLess.has_parent() );

  // 2. has a parent
  BOOST_CHECK( withParent.has_parent() );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( child )
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

  boost::shared_ptr< NRoot > node(new NRoot("Root"));
  boost::shared_ptr< NGeneric > node1(new NGeneric("Node1", "MyType"));
  boost::shared_ptr< NGeneric > node11(new NGeneric("Node11", "MyType"));
  boost::shared_ptr< NGeneric > node12(new NGeneric("Node12", "MyType"));
  boost::shared_ptr< NGeneric > node2(new NGeneric("Node2", "MyType"));
  boost::shared_ptr< NGeneric > node21(new NGeneric("Node21", "MyType"));
  boost::shared_ptr< NGeneric > node22(new NGeneric("Node22", "MyType"));
  boost::shared_ptr< NGeneric > node23(new NGeneric("Node23", "MyType"));
  boost::shared_ptr< NGeneric > node3(new NGeneric("Node3", "MyType"));

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

  treeNode = new TreeNode(node->handle<CNode>(), nullptr, 0);

  //
  // 1. check the direct children
  //
  treeNode1 = treeNode->child(0);
  treeNode2 = treeNode->child(1);
  treeNode3 = treeNode->child(2);

  // check that pointer are not null
  BOOST_CHECK( treeNode1 != (TreeNode*) nullptr );
  BOOST_CHECK( treeNode2 != (TreeNode*) nullptr );
  BOOST_CHECK( treeNode3 != (TreeNode*) nullptr );

  // their parent should be the root
  BOOST_CHECK_EQUAL( treeNode1->parent_node(), treeNode );
  BOOST_CHECK_EQUAL( treeNode2->parent_node(), treeNode );
  BOOST_CHECK_EQUAL( treeNode3->parent_node(), treeNode );

  // check their name
  BOOST_CHECK_EQUAL( treeNode1->node_name().toStdString(), std::string("Node1") );
  BOOST_CHECK_EQUAL( treeNode2->node_name().toStdString(), std::string("Node2") );
  BOOST_CHECK_EQUAL( treeNode3->node_name().toStdString(), std::string("Node3") );

  //
  // 2. children of Node1
  //
  treeNode11 = treeNode1->child(0);
  treeNode12 = treeNode1->child(1);

  // check that pointer are not null
  BOOST_CHECK( treeNode11 != (TreeNode*) nullptr );
  BOOST_CHECK( treeNode12 != (TreeNode*) nullptr );

  // their parent should be the root
  BOOST_CHECK_EQUAL( treeNode11->parent_node(), treeNode1 );
  BOOST_CHECK_EQUAL( treeNode12->parent_node(), treeNode1 );

  // check their name
  BOOST_CHECK_EQUAL( treeNode11->node_name().toStdString(), std::string("Node11") );
  BOOST_CHECK_EQUAL( treeNode12->node_name().toStdString(), std::string("Node12") );

  //
  // 3. children of Node2
  //
  treeNode21 = treeNode2->child(0);
  treeNode22 = treeNode2->child(1);
  treeNode23 = treeNode2->child(2);

  // check that pointer are not null
  BOOST_CHECK( treeNode21 != (TreeNode*) nullptr );
  BOOST_CHECK( treeNode22 != (TreeNode*) nullptr );
  BOOST_CHECK( treeNode23 != (TreeNode*) nullptr );

  // their parent should be the root
  BOOST_CHECK_EQUAL( treeNode21->parent_node(), treeNode2 );
  BOOST_CHECK_EQUAL( treeNode22->parent_node(), treeNode2 );
  BOOST_CHECK_EQUAL( treeNode23->parent_node(), treeNode2 );

  // check their name
  BOOST_CHECK_EQUAL( treeNode21->node_name().toStdString(), std::string("Node21") );
  BOOST_CHECK_EQUAL( treeNode22->node_name().toStdString(), std::string("Node22") );
  BOOST_CHECK_EQUAL( treeNode23->node_name().toStdString(), std::string("Node23") );

  //
  // 4. no child for Node3 (should return nullptr)
  //
  BOOST_CHECK_EQUAL( treeNode3->child(0), (TreeNode*) nullptr );

  delete treeNode; // deletes the children as well
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( child_by_name )
{
  // same tree as for test_child()

  boost::shared_ptr< NRoot > node(new NRoot("Root"));
  boost::shared_ptr< NGeneric > node1(new NGeneric("Node1", "MyType"));
  boost::shared_ptr< NGeneric > node11(new NGeneric("Node11", "MyType"));
  boost::shared_ptr< NGeneric > node12(new NGeneric("Node12", "MyType"));
  boost::shared_ptr< NGeneric > node2(new NGeneric("Node2", "MyType"));
  boost::shared_ptr< NGeneric > node21(new NGeneric("Node21", "MyType"));
  boost::shared_ptr< NGeneric > node22(new NGeneric("Node22", "MyType"));
  boost::shared_ptr< NGeneric > node23(new NGeneric("Node23", "MyType"));
  boost::shared_ptr< NGeneric > node3(new NGeneric("Node3", "MyType"));

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

  treeNode = new TreeNode(node->handle<CNode>(), nullptr, 0);
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
  BOOST_CHECK( treeNode1 != (TreeNode*) nullptr );
  BOOST_CHECK( treeNode2 != (TreeNode*) nullptr );
  BOOST_CHECK( treeNode3 != (TreeNode*) nullptr );

  // their parent should be the root
  BOOST_CHECK_EQUAL( treeNode1->parent_node(), treeNode );
  BOOST_CHECK_EQUAL( treeNode2->parent_node(), treeNode );
  BOOST_CHECK_EQUAL( treeNode3->parent_node(), treeNode );

  // check their row number
  BOOST_CHECK_EQUAL( treeNode1->row_number(), 0 );
  BOOST_CHECK_EQUAL( treeNode2->row_number(), 1 );
  BOOST_CHECK_EQUAL( treeNode3->row_number(), 2 );

  //
  // 2. children of Node1
  //
  treeNode11 = treeNode1->child_by_name("Node11");
  treeNode12 = treeNode1->child_by_name("Node12");

  // check that pointer are not null
  BOOST_CHECK( treeNode11 != (TreeNode*) nullptr );
  BOOST_CHECK( treeNode12 != (TreeNode*) nullptr );

  // their parent should be the root
  BOOST_CHECK_EQUAL( treeNode11->parent_node(), treeNode1 );
  BOOST_CHECK_EQUAL( treeNode12->parent_node(), treeNode1 );

  // check their row number
  BOOST_CHECK_EQUAL( treeNode11->row_number(), 0 );
  BOOST_CHECK_EQUAL( treeNode12->row_number(), 1 );

  //
  // 3. wrong name
  //
  BOOST_CHECK_EQUAL ( treeNode2->child_by_name("Node"), (TreeNode*) nullptr );


  delete treeNode; // deletes the children as well
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( update_child_list )
{
  boost::shared_ptr< NRoot > node(new NRoot("Root"));
  boost::shared_ptr< NGeneric > node1(new NGeneric("Node1", "MyType"));
  boost::shared_ptr< NGeneric > node2(new NGeneric("Node2", "MyType"));
  TreeNode * treeNode;
  TreeNode * child;

  node->add_node(node1);

  treeNode = new TreeNode(node->handle<CNode>(), nullptr, 0);

  BOOST_CHECK_EQUAL( treeNode->child_count(), 1);

  node->add_node(node2);

  BOOST_CHECK_EQUAL( treeNode->child_count(), 2);

  child = treeNode->child(1);
  BOOST_CHECK( child != nullptr );

  BOOST_CHECK_EQUAL( child->node_name().toStdString(), std::string("Node2"));

  delete treeNode;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
