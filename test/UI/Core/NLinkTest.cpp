// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtTest>
#include <QModelIndex>

#include "Common/URI.hpp"

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

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::UI::Core;
using namespace CF::UI::CoreTest;

Q_DECLARE_METATYPE(QModelIndex);


////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace CoreTest {

////////////////////////////////////////////////////////////////////////////////

void NLinkTest::test_tootip()
{
  NRoot::Ptr root(new NRoot("Root"));
  NGeneric::Ptr target(new NGeneric("Target", "MyType"));

  NLink::Ptr l1(new NLink("Link1"));
  NLink::Ptr l2(new NLink("Link2"));

  root->addNode(target);
  root->addNode(l1);

  l1->setTargetPath("cpath://Root/Target");

  QCOMPARE(l1->toolTip(), QString("Target: //Root/Target"));
  QCOMPARE(l2->toolTip(), QString("Target: <No target>"));

  root->removeNode( "Link1" );
  root->removeNode( "Target" );
}

///////////////////////////////////////////////////////////////////////////////

void NLinkTest::test_targetPath()
{
  NRoot::Ptr root(new NRoot("Root"));
  NGeneric::Ptr target(new NGeneric("Target", "MyType"));

  NLink::Ptr l1(new NLink("Link1"));
  NLink::Ptr l2(new NLink("Link2"));

  root->addNode(target);
  root->addNode(l1);

  l1->setTargetPath("cpath://Root/Target");

  QCOMPARE(QString(l1->targetPath().string().c_str()), QString("cpath://Root/Target"));
  QCOMPARE(QString(l2->targetPath().string().c_str()), QString(""));

  root->removeNode( "Link1" );
  root->removeNode( "Target" );
}

///////////////////////////////////////////////////////////////////////////////

void NLinkTest::test_goToTarget()
{
  // QModelIndex needs to be registered. See QSignalSpy class doc.
 qRegisterMetaType<QModelIndex>("QModelIndex");

 QModelIndex index;
 SignalFrame frame;
 NRoot::Ptr root = ThreadManager::instance().tree().root();;
 NGeneric::Ptr target(new NGeneric("Target", "MyType"));
 NGeneric::Ptr wrongTarget(new NGeneric("WrongTarget", "MyType")); // not part of the tree
 NTree::Ptr tree = NTree::globalTree();
 NLink::Ptr link(new NLink("link"));
 QSignalSpy spy(tree.get(), SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)));

 root->addNode(target);
 root->addNode(link);

 tree->setCurrentIndex(tree->index(0, 0));

 // 1. The link has no target
 GUI_CHECK_THROW( link->goToTarget(frame) , ValueNotFound );

 // 2. target does not belong to the tree
 link->setTargetNode(wrongTarget);
 GUI_CHECK_THROW( link->goToTarget(frame) , ValueNotFound );

 // 3. everything is OK
 spy.clear();
 link->setTargetNode(target);
 index = tree->indexFromPath("cpath://Root/Target");
 GUI_CHECK_NO_THROW(link->goToTarget(frame));

 QCOMPARE(spy.count(), 1);

 // check that the correct index was selected
 QCOMPARE(qvariant_cast<QModelIndex>(spy.at(0).at(0)), index);

 root->removeNode( "link" );
 root->removeNode( "Target" );
}

///////////////////////////////////////////////////////////////////////////////

void NLinkTest::test_setTargetPath()
{
  NTree::Ptr tree = NTree::globalTree();
  NLink::Ptr link(new NLink("link"));

  // 1. link has no root, assertion should fail
  GUI_CHECK_THROW( link->setTargetPath(""), FailedAssertion );

  tree->treeRoot()->addNode(link);

  // 2. path does not exist, assertion should fail
  GUI_CHECK_THROW( link->setTargetPath("cpath://Root/Unexisting/Component"), FailedAssertion );

  // 3. everything is ok
  GUI_CHECK_NO_THROW( link->setTargetPath("cpath://Root/Log") );
  QCOMPARE( QString(link->targetPath().string().c_str()), QString("cpath://Root/Log") );

  tree->treeRoot()->removeNode("link");
}

///////////////////////////////////////////////////////////////////////////////

void NLinkTest::test_setTargetNode()
{
  NTree::Ptr tree = NTree::globalTree();
  NLink::Ptr link(new NLink("link"));
  NGeneric::Ptr target(new NGeneric("Target", "MyType"));
  NGeneric::Ptr emptyTarget;

  tree->treeRoot()->addNode(link);
  tree->treeRoot()->addNode(target);

  // 1. give an empty target
  GUI_CHECK_THROW( link->setTargetNode( emptyTarget ), FailedAssertion );

  // 2. everything is ok
  GUI_CHECK_NO_THROW( link->setTargetNode( target ) );
  QCOMPARE( QString(link->targetPath().string().c_str()), QString("cpath://Root/Target") );

  tree->treeRoot()->removeNode("link");
  tree->treeRoot()->removeNode("Target");
}

///////////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // CF
