// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtTest>
#include <QModelIndex>

#include "Common/URI.hpp"

#include "GUI/Client/Core/ThreadManager.hpp"
#include "GUI/Client/Core/TreeThread.hpp"
#include "GUI/Client/Core/NLink.hpp"
#include "GUI/Client/Core/NGeneric.hpp"
#include "GUI/Client/Core/NTree.hpp"
#include "GUI/Client/Core/NLog.hpp"

#include "test/GUI/Client/CommonFunctions.hpp"
#include "test/GUI/Client/ExceptionThrowHandler.hpp"
#include "test/GUI/Client/TreeHandler.hpp"

#include "test/GUI/Client/NLinkTest.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientTest;

Q_DECLARE_METATYPE(QModelIndex);

void NLinkTest::test_getTootip()
{
  NRoot::Ptr root(new NRoot("Root"));
  NGeneric::Ptr target(new NGeneric("Target", "MyType"));

  NLink::Ptr l1(new NLink("Link1"));
  NLink::Ptr l2(new NLink("Link2"));

  root->addNode(target);
  root->addNode(l1);

  l1->setTargetPath("//Root/Target");

  QCOMPARE(l1->toolTip(), QString("Target: //Root/Target"));
  QCOMPARE(l2->toolTip(), QString("Target: <No target>"));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLinkTest::test_getTargetPath()
{
  NRoot::Ptr root(new NRoot("Root"));
  NGeneric::Ptr target(new NGeneric("Target", "MyType"));

  NLink::Ptr l1(new NLink("Link1"));
  NLink::Ptr l2(new NLink("Link2"));

  root->addNode(target);
  root->addNode(l1);

  l1->setTargetPath("//Root/Target");

  QCOMPARE(QString(l1->targetPath().path().c_str()), QString("//Root/Target"));
  QCOMPARE(QString(l2->targetPath().string().c_str()), QString(""));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLinkTest::test_goToTarget()
{
  // QModelIndex needs to be registered. See QSignalSpy class doc.
 qRegisterMetaType<QModelIndex>("QModelIndex");

 QModelIndex index;
 SignalFrame frame("", "", "");
 NRoot::Ptr root = ThreadManager::instance().tree().root();;
 NGeneric::Ptr target(new NGeneric("Target", "MyType"));
 NLog::Ptr wrongTarget(new NLog()); // not part of the tree
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
 index = tree->indexFromPath("//Root/Target");
 GUI_CHECK_NO_THROW(link->goToTarget(frame));

 QCOMPARE(spy.count(), 1);

 // check that the correct index was selected
 QCOMPARE(qvariant_cast<QModelIndex>(spy.at(0).at(0)), index);
}
