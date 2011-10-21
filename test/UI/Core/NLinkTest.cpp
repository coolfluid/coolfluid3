// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtTest>
#include <QModelIndex>

#include "common/URI.hpp"

#include "common/XML/SignalFrame.hpp"

#include "UI/Core/ThreadManager.hpp"
#include "UI/Core/TreeThread.hpp"
#include "UI/Core/NLink.hpp"
#include "UI/Core/NGeneric.hpp"
#include "UI/Core/NTree.hpp"
#include "UI/Core/NLog.hpp"

#include "test/UI/Core/CommonFunctions.hpp"
#include "test/UI/Core/ExceptionThrowHandler.hpp"
#include "test/UI/Core/TreeHandler.hpp"

#include "test/UI/Core/NLinkTest.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::UI::Core;
using namespace cf3::UI::CoreTest;

Q_DECLARE_METATYPE(QModelIndex);


////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace CoreTest {

////////////////////////////////////////////////////////////////////////////////

void NLinkTest::test_tootip()
{
  NRoot::Ptr root(new NRoot("Root"));
  NGeneric::Ptr target(new NGeneric("Target", "MyType"));

  NLink::Ptr l1(new NLink("Link1"));
  NLink::Ptr l2(new NLink("Link2"));

  root->add_node(target);
  root->add_node(l1);

  l1->set_target_path("cpath://Root/Target");

  QCOMPARE(l1->tool_tip(), QString("Target: //Root/Target"));
  QCOMPARE(l2->tool_tip(), QString("Target: <No target>"));

  root->remove_node( "Link1" );
  root->remove_node( "Target" );
}

///////////////////////////////////////////////////////////////////////////////

void NLinkTest::test_targetPath()
{
  NRoot::Ptr root(new NRoot("Root"));
  NGeneric::Ptr target(new NGeneric("Target", "MyType"));

  NLink::Ptr l1(new NLink("Link1"));
  NLink::Ptr l2(new NLink("Link2"));

  root->add_node(target);
  root->add_node(l1);

  l1->set_target_path("cpath://Root/Target");

  QCOMPARE(QString(l1->target_path().string().c_str()), QString("cpath://Root/Target"));
  QCOMPARE(QString(l2->target_path().string().c_str()), QString(""));

  root->remove_node( "Link1" );
  root->remove_node( "Target" );
}

///////////////////////////////////////////////////////////////////////////////

void NLinkTest::test_goToTarget()
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
 GUI_CHECK_THROW( link->go_to_target(frame) , ValueNotFound );

 // 2. target does not belong to the tree
 link->set_target_node(wrongTarget);
 GUI_CHECK_THROW( link->go_to_target(frame) , ValueNotFound );

 // 3. everything is OK
 spy.clear();
 link->set_target_node(target);
 index = tree->index_from_path("cpath://Root/Target");
 GUI_CHECK_NO_THROW(link->go_to_target(frame));

 QCOMPARE(spy.count(), 1);

 // check that the correct index was selected
 QCOMPARE(qvariant_cast<QModelIndex>(spy.at(0).at(0)), index);

 root->remove_node( "link" );
 root->remove_node( "Target" );
}

///////////////////////////////////////////////////////////////////////////////

void NLinkTest::test_setTargetPath()
{
  NTree::Ptr tree = NTree::global();
  NLink::Ptr link(new NLink("link"));

  // 1. link has no root, assertion should fail
  GUI_CHECK_THROW( link->set_target_path(""), FailedAssertion );

  tree->tree_root()->add_node(link);

  // 2. path does not exist, assertion should fail
  GUI_CHECK_THROW( link->set_target_path("cpath://Root/Unexisting/Component"), FailedAssertion );

  // 3. everything is ok
  GUI_CHECK_NO_THROW( link->set_target_path("cpath://Root/UI/Log") );
  QCOMPARE( QString(link->target_path().string().c_str()), QString("cpath://Root/UI/Log") );

  tree->tree_root()->remove_node("link");
}

///////////////////////////////////////////////////////////////////////////////

void NLinkTest::test_setTargetNode()
{
  NTree::Ptr tree = NTree::global();
  NLink::Ptr link(new NLink("link"));
  NGeneric::Ptr target(new NGeneric("Target", "MyType"));
  NGeneric::Ptr emptyTarget;

  tree->tree_root()->add_node(link);
  tree->tree_root()->add_node(target);

  // 1. give an empty target
  GUI_CHECK_THROW( link->set_target_node( emptyTarget ), FailedAssertion );

  // 2. everything is ok
  GUI_CHECK_NO_THROW( link->set_target_node( target ) );
  QCOMPARE( QString(link->target_path().string().c_str()), QString("cpath://Root/Target") );

  tree->tree_root()->remove_node("link");
  tree->tree_root()->remove_node("Target");
}

///////////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // cf3
