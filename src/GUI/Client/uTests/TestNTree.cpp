#include <QtCore>
#include <QtTest>

#include <iostream>

#include "Common/XmlHelpers.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/NRoot.hpp"
#include "GUI/Client/NTree.hpp"

#include "GUI/Client/UnknownTypeException.hpp"

#include "GUI/Client/uTests/ExceptionThrowHandler.hpp"
#include "GUI/Client/uTests/TestNTree.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;
using namespace CF::GUI::ClientTest;

Q_DECLARE_METATYPE(QModelIndex);

////////////////////////////////////////////////////////////////////////////

NRoot::Ptr NTreeTest::makeTreeFromFile()
{
  boost::shared_ptr<XmlDoc> doc = XmlOps::parse(boost::filesystem::path("./tree.xml"));
  return CNode::convertTo<NRoot>(CNode::createFromXml(*doc->first_node()));
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_constructor()
{
  NTree t;
  NTree t2(makeTreeFromFile());

  // the root must be the same as the client root
  QVERIFY(t.getRoot().get() == ClientRoot::getRoot().get());

  // the root must be different from CFNULL and from the client root
  QVERIFY(t2.getRoot().get() != CFNULL);
  QVERIFY(t2.getRoot().get() != ClientRoot::getRoot().get());
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_setRoot()
{
  NTree t;
  NRoot::Ptr newRoot(makeTreeFromFile());
  QSignalSpy spy(&t, SIGNAL(layoutChanged()));

  t.setRoot(newRoot);

  // the root must be different from CFNULL and equal to the new root
  QVERIFY(t.getRoot().get() != CFNULL);
  QVERIFY(t.getRoot().get() == newRoot.get());

  // the tree must have emitted a layoutChanged signal exactly once
  QCOMPARE(spy.count(), 1);
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_setCurrentIndex()
{
  // QModelIndex needs to be registered. See QSignalSpy class doc.
  qRegisterMetaType<QModelIndex>("QModelIndex");

  NTree t;
  QModelIndex index = t.getCurrentIndex();
  QSignalSpy spy(&t, SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)));

  t.setCurrentIndex(t.index(0, 0));

  QVERIFY(index != t.getCurrentIndex());

  // the tree must have emitted a currentIndexChanged signal exactly once
  QCOMPARE(spy.count(), 1);

  // check signal parameters
  QList<QVariant> arguments = spy.takeFirst();
  QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(0)), t.getCurrentIndex());
  QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(1)), index);

  // note : the fact that the index is not set if it is the same as the
  // current index is not tested here. It would be like testing
  // areFromSameNode() method.
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_getNodeParams()
{
  // find something
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_setAdvancedMode()
{
  NTree t;
  QSignalSpy spy(&t, SIGNAL(advancedModeChanged(bool)));

  // by default, advanced is disabled
  QVERIFY(!t.isAdvancedMode());

  t.setAdvancedMode(true);

  // the tree must have emitted a advancedModeChanged signal exactly once
  QCOMPARE(spy.count(), 1);

  // check signal parameter
  QList<QVariant> arguments = spy.takeFirst();
  QCOMPARE(arguments.at(0).toBool(), true);
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_areFromSameNode()
{
  NTree t;
  QModelIndex index = t.index(0, 0);
  QModelIndex anotherIndex = t.index(0, 0, index);

  t.setCurrentIndex(index);

  QVERIFY(t.areFromSameNode(t.getCurrentIndex(), index));
  QVERIFY(!t.areFromSameNode(t.getCurrentIndex(), anotherIndex));
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_haveSameData()
{
 // to test
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_getNodeByPath()
{
  NTree t;
  CNode::Ptr logNode = t.getNodeByPath("//blabla"); // the path does not exist

  QVERIFY(logNode.get() == CFNULL);

  logNode = t.getNodeByPath(CLIENT_LOG_PATH);

  QVERIFY(logNode.get() != CFNULL);

  // no risk of segfault if the test has failed (null pointer)
  QCOMPARE(logNode->full_path().string(), std::string(CLIENT_LOG_PATH));
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_getIndexByPath()
{
  // to test
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_data()
{
  NTree t;
  QModelIndex logIndex = t.getIndexByPath(CLIENT_LOG_PATH);
  QModelIndex logScndCol = t.index(logIndex.row(), 1, logIndex.parent());

  // check with an invalid index
  QVERIFY(!t.data(QModelIndex(), Qt::DisplayRole).isValid());

  // we are not in debug mode, so the data is not valid
  QVERIFY(!t.data(logIndex, Qt::DisplayRole).isValid());
  QVERIFY(!t.data(logIndex, Qt::DecorationRole).isValid());
  QVERIFY(!t.data(logScndCol, Qt::DisplayRole).isValid());
  QVERIFY(!t.data(logScndCol, Qt::DecorationRole).isValid());

  t.setDebugModeEnabled(true);

  // now that we are in debug mode, the method should return correct values
  QVariant logName = t.data(logIndex, Qt::DisplayRole);
  QVariant logIcon = t.data(logIndex, Qt::DecorationRole);
  QVariant logToolTip = t.data(logIndex, Qt::ToolTipRole);
  QVariant logToolTipScndCol = t.data(logScndCol, Qt::ToolTipRole);

  QCOMPARE(logName.toString(), QString(CLIENT_LOG));
  QCOMPARE(logIcon.type(), QVariant::Icon);
  QCOMPARE(qvariant_cast<QIcon>(logIcon), NLog().getIcon());
  QCOMPARE(logToolTip.toString(), NLog().getToolTip());
  QCOMPARE(logToolTipScndCol.toString(), NLog().getToolTip());
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_index()
{
  NTree t;
  QModelIndex index = t.index(0, 0);

  // test with both trees

  QVERIFY(index.isValid());
  QVERIFY(t.index(0, 1).isValid());
  QVERIFY(!t.index(12, 0, index).isValid());
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_parent()
{
  NTree t;
  QModelIndex rootIndex = t.index(0, 0);
  QModelIndex childIndex = t.index(1, 0, rootIndex);

  QVERIFY(!t.parent(rootIndex).isValid());
  QCOMPARE(t.parent(childIndex), rootIndex);
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_rowCount()
{
  NTree t;

  QCOMPARE(t.rowCount(), 1);
  QCOMPARE(t.rowCount(t.index(0, 0)), ClientRoot::getRoot()->root()->get_child_count());
  QCOMPARE(t.rowCount(t.index(0, 1)), 0);
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_headerData()
{
  NTree t;

  QCOMPARE(t.headerData(0, Qt::Horizontal).toString(), QString("Name"));
  QCOMPARE(t.headerData(1, Qt::Horizontal).toString(), QString("Type"));

  QVERIFY(!t.headerData(0, Qt::Vertical).isValid());
  QVERIFY(!t.headerData(0, Qt::Horizontal, Qt::DecorationRole).isValid());
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_setDebugModeEnabled()
{
  NTree t;
  QSignalSpy spy(&t, SIGNAL(layoutChanged()));

  // by default, debug mode is disabled
  QVERIFY(!t.isDebugModeEnabled());

  t.setDebugModeEnabled(true);

  // the tree must have emitted a layout changed signal exactly once
  QCOMPARE(spy.count(), 1);
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_list_tree()
{
  //
}
