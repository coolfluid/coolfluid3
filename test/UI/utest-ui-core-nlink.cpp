// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the UI NLink class"

#include <QModelIndex>
#include <QSignalSpy>

#include "common/XML/SignalFrame.hpp"

#include "UI/Core/NGeneric.hpp"
#include "UI/Core/NLink.hpp"
#include "UI/Core/NRoot.hpp"
#include "UI/Core/NTree.hpp"
#include "UI/Core/TreeThread.hpp"
#include "UI/Core/ThreadManager.hpp"

#include "test/UI/CoreApplication.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::UI::Core;

Q_DECLARE_METATYPE(QModelIndex)

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( UICoreNLinkSuite )

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

BOOST_AUTO_TEST_CASE( tool_tip )
{
  NRoot::Ptr root(new NRoot("Root"));
  NGeneric::Ptr target(new NGeneric("Target", "MyType"));

  NLink::Ptr l1(new NLink("Link1"));
  NLink::Ptr l2(new NLink("Link2"));

  root->add_node(target);
  root->add_node(l1);

  l1->set_target_path("cpath://Root/Target");

  BOOST_CHECK_EQUAL(l1->tool_tip().toStdString(), std::string("Target: //Root/Target"));
  BOOST_CHECK_EQUAL(l2->tool_tip().toStdString(), std::string("Target: <No target>"));

  root->remove_node( "Link1" );
  root->remove_node( "Target" );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( target_path )
{
  NRoot::Ptr root(new NRoot("Root"));
  NGeneric::Ptr target(new NGeneric("Target", "MyType"));

  NLink::Ptr l1(new NLink("Link1"));
  NLink::Ptr l2(new NLink("Link2"));

  root->add_node(target);
  root->add_node(l1);

  l1->set_target_path("cpath://Root/Target");

  BOOST_CHECK_EQUAL( l1->target_path().string(), std::string("cpath://Root/Target"));
  BOOST_CHECK_EQUAL( l2->target_path().string(), std::string(""));

  root->remove_node( "Link1" );
  root->remove_node( "Target" );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( go_to_target )
{
  // QModelIndex needs to be registered. See QSignalSpy class doc.
 qRegisterMetaType<QModelIndex>("QModelIndex");

 QModelIndex index;
 SignalFrame frame;
 NRoot::Ptr root = ThreadManager::instance().tree().root();
 NGeneric::Ptr target(new NGeneric("Target", "MyType"));
 NGeneric::Ptr wrongTarget(new NGeneric("WrongTarget", "MyType")); // not part of the tree
 NTree::Ptr tree = NTree::global();
 NLink::Ptr link(new NLink("link"));
 QSignalSpy spy(tree.get(), SIGNAL(current_index_changed(QModelIndex,QModelIndex)));

 root->add_node(target);
 root->add_node(link);

 tree->set_current_index(tree->index(0, 0));

 // 1. The link has no target
 BOOST_CHECK_THROW( link->go_to_target(frame) , ValueNotFound );

 // 2. target does not belong to the tree
 link->set_target_node(wrongTarget);
 BOOST_CHECK_THROW( link->go_to_target(frame) , ValueNotFound );

 // 3. everything is OK
 spy.clear();
 link->set_target_node(target);
 index = tree->index_from_path("cpath://Root/Target");
 BOOST_REQUIRE_NO_THROW(link->go_to_target(frame));

 BOOST_CHECK_EQUAL(spy.count(), 1);

 // check that the correct index was selected
 BOOST_CHECK_EQUAL(qvariant_cast<QModelIndex>(spy.at(0).at(0)).internalPointer(), index.internalPointer());

 root->remove_node( "link" );
 root->remove_node( "Target" );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_target_path )
{
  NTree::Ptr tree = NTree::global();
  NLink::Ptr link(new NLink("link"));

  // 1. link has no root, assertion should fail
  BOOST_CHECK_THROW( link->set_target_path(""), FailedAssertion );

  tree->tree_root()->add_node(link);

  // 2. path does not exist, assertion should fail
  BOOST_CHECK_THROW( link->set_target_path("cpath://Root/Unexisting/Component"), FailedAssertion );

  // 3. everything is ok
  BOOST_REQUIRE_NO_THROW( link->set_target_path("cpath://Root/UI/Log") );
  BOOST_CHECK_EQUAL( link->target_path().string(), std::string("cpath://Root/UI/Log") );

  tree->tree_root()->remove_node("link");
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_target_node )
{
  NTree::Ptr tree = NTree::global();
  NLink::Ptr link(new NLink("link"));
  NGeneric::Ptr target(new NGeneric("Target", "MyType"));
  NGeneric::Ptr emptyTarget;

  tree->tree_root()->add_node(link);
  tree->tree_root()->add_node(target);

  // 1. give an empty target
  BOOST_CHECK_THROW( link->set_target_node( emptyTarget ), FailedAssertion );

  // 2. everything is ok
  BOOST_REQUIRE_NO_THROW( link->set_target_node( target ) );
  BOOST_CHECK_EQUAL( link->target_path().string(), std::string("cpath://Root/Target") );

  tree->tree_root()->remove_node("link");
  tree->tree_root()->remove_node("Target");
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
