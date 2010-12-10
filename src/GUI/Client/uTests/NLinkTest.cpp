// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtTest>
#include <QModelIndex>

#include "Common/URI.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/NLink.hpp"
#include "GUI/Client/Core/NGeneric.hpp"
#include "GUI/Client/Core/NTree.hpp"

#include "GUI/Client/uTests/CommonFunctions.hpp"
#include "GUI/Client/uTests/ExceptionThrowHandler.hpp"
#include "GUI/Client/uTests/TreeHandler.hpp"

#include "GUI/Client/uTests/NLinkTest.hpp"

using namespace CF::Common;
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

  QCOMPARE(QString(l1->targetPath().string().c_str()), QString("//Root/Target"));
  QCOMPARE(QString(l2->targetPath().string().c_str()), QString(""));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLinkTest::test_goToTarget()
{
  // QModelIndex needs to be registered. See QSignalSpy class doc.
 qRegisterMetaType<QModelIndex>("QModelIndex");

  TreeHandler th;
  NTree::Ptr t = ClientRoot::tree();
  QModelIndex index;
  QSignalSpy spy(t.get(), SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)));

  NLink::Ptr link;

  th.addChildren(makeTreeFromFile());
  link = boost::dynamic_pointer_cast<NLink>(t->treeRoot()->root()->access_component("//Root/Flow/Mesh"));

  QVERIFY(link.get() != nullptr);
  t->setCurrentIndex(t->index(0, 0));

  index = t->indexByPath("//Root/MG/Mesh1");
  link->goToTarget(*XmlOps::create_doc().get());

  // 2 signals should have been thrown, one by setCurrentIndex() and one by
  // goToTarget()
  QCOMPARE(spy.count(), 2);

  QCOMPARE(qvariant_cast<QModelIndex>(spy.at(1).at(0)), index);
}
