#include <QtTest>
#include <QModelIndex>

#include "Common/CPath.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/NLink.hpp"
#include "GUI/Client/NGroup.hpp"
#include "GUI/Client/NTree.hpp"

#include "GUI/Client/uTests/TreeHandler.hpp"
#include "GUI/Client/uTests/ExceptionThrowHandler.hpp"

#include "GUI/Client/uTests/NLinkTest.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;
using namespace CF::GUI::ClientTest;

Q_DECLARE_METATYPE(QModelIndex);

void NLinkTest::test_getTootip()
{
  NRoot::Ptr root(new NRoot("Root"));
  NGroup::Ptr target(new NGroup("Target"));

  NLink::Ptr l1(new NLink("Link1"));
  NLink::Ptr l2(new NLink("Link2"));

  root->addNode(target);
  root->addNode(l1);

  l1->setTargetPath("//Root/Target");

  QCOMPARE(l1->getToolTip(), QString("Target: //Root/Target"));
  QCOMPARE(l2->getToolTip(), QString("Target: <No target>"));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLinkTest::test_getTargetPath()
{
  NRoot::Ptr root(new NRoot("Root"));
  NGroup::Ptr target(new NGroup("Target"));

  NLink::Ptr l1(new NLink("Link1"));
  NLink::Ptr l2(new NLink("Link2"));

  root->addNode(target);
  root->addNode(l1);

  l1->setTargetPath("//Root/Target");

  QCOMPARE(QString(l1->getTargetPath().string().c_str()), QString("//Root/Target"));
  QCOMPARE(QString(l2->getTargetPath().string().c_str()), QString(""));
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

  boost::shared_ptr<XmlDoc> doc;
  NLink::Ptr link;
  ComponentIterator<CNode> it = t->getRoot()->root()->begin<CNode>();

   GUI_CHECK_NO_THROW(doc = XmlOps::parse(boost::filesystem::path("./tree.xml")));
  th.addChildren(CNode::createFromXml(*doc->first_node()));
  link = boost::dynamic_pointer_cast<NLink>(t->getRoot()->root()->access_component("//Simulator/Flow/Mesh"));

  QVERIFY(link.get() != CFNULL);
  t->setCurrentIndex(t->index(0, 0));

  index = t->getIndexByPath("//Simulator/MG/Mesh1");
  link->goToTarget();

  // 2 signals should have been thrown, one by setCurrentIndex() and one by
  // goToTarget()
  QCOMPARE(spy.count(), 2);

  QCOMPARE(qvariant_cast<QModelIndex>(spy.at(1).at(0)), index);
}
