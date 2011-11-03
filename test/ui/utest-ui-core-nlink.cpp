// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the ui NLink class"

#include <QModelIndex>
#include <QSignalSpy>

#include "common/XML/SignalFrame.hpp"

#include "ui/core/NGeneric.hpp"
#include "ui/core/NLink.hpp"
#include "ui/core/NRoot.hpp"
#include "ui/core/NTree.hpp"
#include "ui/core/TreeThread.hpp"
#include "ui/core/ThreadManager.hpp"

#include "test/ui/CoreApplication.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::core;

Q_DECLARE_METATYPE(QModelIndex)

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( uiCoreNLinkSuite )

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

  l1->set_target_path("cpath:/Target");

  BOOST_CHECK_EQUAL(l1->tool_tip().toStdString(), std::string("Target: /Target"));
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

  l1->set_target_path("cpath:/Target");

  BOOST_CHECK_EQUAL( l1->target_path().string(), std::string("cpath:/Target"));
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
 NGeneric::Ptr wrongTargetParent(new NGeneric("WrongTargetParent", "MyType")); // not part of the tree
 NGeneric::Ptr wrongTarget(new NGeneric("WrongTarget", "MyType"));
 wrongTargetParent->add_component(wrongTarget);
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
 index = tree->index_from_path("cpath:/Target");
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

  tree->tree_root()->add_node(link);

  // 1. path does not exist, assertion should fail
  BOOST_CHECK_THROW( link->set_target_path("cpath:/Unexisting/Component"), InvalidURI );

  // 2. everything is ok
  BOOST_REQUIRE_NO_THROW( link->set_target_path("cpath:/UI/Log") );
  BOOST_CHECK_EQUAL( link->target_path().string(), std::string("cpath:/UI/Log") );

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
  BOOST_CHECK_EQUAL( link->target_path().string(), std::string("cpath:/Target") );

  tree->tree_root()->remove_node("link");
  tree->tree_root()->remove_node("Target");
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
