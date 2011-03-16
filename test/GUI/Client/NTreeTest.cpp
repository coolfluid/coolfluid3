// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtCore>
#include <QtTest>

#include "rapidxml/rapidxml.hpp"

#include "Common/CGroup.hpp"
#include "Common/CLink.hpp"

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/TreeThread.hpp"
#include "GUI/Client/Core/NGeneric.hpp"
#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/Core/NRoot.hpp"
#include "GUI/Client/Core/NTree.hpp"
#include "GUI/Client/Core/ThreadManager.hpp"

#include "test/GUI/Client/CommonFunctions.hpp"
#include "test/GUI/Client/ExceptionThrowHandler.hpp"
#include "test/GUI/Client/MyNode.hpp"

#include "test/GUI/Client/NTreeTest.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientTest;

Q_DECLARE_METATYPE(QModelIndex);

///////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientTest {

/////////////////////////////////////////////////////////////////////////

void NTreeTest::test_constructor()
{
  NTree t;
  NTree t2(makeTreeFromFile());

  // the root must be the same as the client root
  QCOMPARE(t.treeRoot().get(), ThreadManager::instance().tree().root().get());
  QCOMPARE(makeTreeFromFile().get(), t2.treeRoot().get());

  // the root must be different from nullptr
  QVERIFY(t2.treeRoot().get() != nullptr);
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_setRoot()
{
  NTree t;
  NRoot::Ptr newRoot(new NRoot("Root"));
  QSignalSpy spy(&t, SIGNAL(layoutChanged()));

  newRoot->root()->create_component<CLink>("link");
  newRoot->root()->create_component<CGroup>("Group1");
  newRoot->root()->create_component<CGroup>("Group2");
  newRoot->root()->create_component<CGroup>("Group3");
  newRoot->root()->create_component<CGroup>("Group4");

  t.setRoot(newRoot);

  // the tree must have emitted a layoutChanged signal exactly once
  QCOMPARE(spy.count(), 1);

  // newRoot must be the tree root now
  QCOMPARE(t.treeRoot(), newRoot);

  // the tree root should have 5 children now
  QCOMPARE( t.treeRoot()->root()->count_children(), std::size_t(5));
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_setCurrentIndex()
{
  // QModelIndex needs to be registered. See QSignalSpy class doc.
  qRegisterMetaType<QModelIndex>("QModelIndex");
  QList<QVariant> arguments;
  NTree t;
  QModelIndex index = t.currentIndex();
  QSignalSpy spy(&t, SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)));

  //
  // 1. setting a correct index
  //
  t.setCurrentIndex(t.index(0, 0));

  QVERIFY(index != t.currentIndex());

  // the tree must have emitted a currentIndexChanged signal exactly once
  QCOMPARE(spy.count(), 1);

  // check signal parameters
  arguments = spy.takeFirst();
  QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(0)), t.currentIndex());
  QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(1)), index);

  //
  // 2. setting the same index as the current one (no signal should be emitted)
  //
  index = t.currentIndex();
  spy.clear();
  t.setCurrentIndex( t.index(0,0) );

  QCOMPARE(spy.count(), 0);

  //
  // 3. setting an invalid index (should work as well)
  //
  index = t.currentIndex();
  spy.clear();
  t.setCurrentIndex(QModelIndex());

  QCOMPARE(spy.count(), 1);

  // check signal parameters
  arguments = spy.takeFirst();
  QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(0)), QModelIndex());
  QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(1)), index);
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_currentPath()
{
  NTree t;
  QModelIndex rootIndex = t.index(0, 0);

  // 1. when the current index is not valid
  QCOMPARE( QString( t.currentPath().string().c_str()), QString() );

  // 2. when the current index is the root
  t.setCurrentIndex( rootIndex );
  QCOMPARE( QString( t.currentPath().string().c_str()), QString("cpath://Root") );

  // 3. when the current index is not the root (i.e the browsers)
  t.setCurrentIndex( t.index(0, 0, rootIndex) );
  QCOMPARE( QString( t.currentPath().string().c_str()), QString("cpath://Root/Browsers") );
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_nodePath()
{
  NTree t;
  QModelIndex rootIndex = t.index(0, 0);

  // 1. when the index is not valid
  QCOMPARE( t.nodePath( QModelIndex() ), QString() );

  // 2. when the index is the root
  QCOMPARE( t.nodePath( rootIndex ), QString("Root/") );

  // 3. when the index is not the root (i.e the browsers)
  QCOMPARE( t.nodePath( t.index(0, 0, rootIndex) ), QString("Root/Browsers/") );
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_pathFromIndex()
{
  NTree t;
  QModelIndex rootIndex = t.index(0, 0);

  // 1. when the current index is not valid
  QCOMPARE( QString( t.pathFromIndex( QModelIndex() ).string().c_str()), QString() );

  // 2. when the current index is the root
  QCOMPARE( QString( t.pathFromIndex( rootIndex ).string().c_str()), QString("cpath://Root") );

  // 3. when the current index is not the root (i.e the browsers)
  QCOMPARE( QString( t.pathFromIndex( t.index(0, 0, rootIndex) ).string().c_str()), QString("cpath://Root/Browsers") );

}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_listNodeOptions()
{
  NTree t;
  MyNode::Ptr node(new MyNode("UselessNode"));
  QModelIndex index;
  QList<Option::ConstPtr> options;
  bool ok = false;

  //
  // 1. index is not valid
  //
  t.listNodeOptions(QModelIndex(), options, &ok);
  QVERIFY(!ok);
  QCOMPARE(options.count(), 0);

  //
  // 2. the list is not empty
  //
  options.append( Option::Ptr(new OptionT<bool>("opt1", "", true)) );
  options.append( Option::Ptr(new OptionT<int>("opt2", "", 42)) );
  options.append( Option::Ptr(new OptionT<std::string>("opt3", "", std::string())) );
  t.listNodeOptions(QModelIndex(), options, &ok);
  QVERIFY(!ok);
  QCOMPARE(options.count(), 0);

  //
  // 3. everything is OK
  //
  t.treeRoot()->addNode(node);

  index = t.indexFromPath( node->full_path() );

  QVERIFY(index.isValid());

  t.listNodeOptions(index, options, &ok);

  QVERIFY(ok);
  QCOMPARE(options.count(), 3);

  t.treeRoot()->root()->remove_component(node->name());
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_setAdvancedMode()
{
  NTree t;
  QSignalSpy spy(&t, SIGNAL(advancedModeChanged(bool)));
  QList<QVariant> arguments;

  //
  // 1. default value
  //

  // by default, advanced is disabled
  QVERIFY(!t.isAdvancedMode());

  //
  // 2. enable advanced mode
  //
  t.setAdvancedMode(true);

  // the tree must have emitted a advancedModeChanged signal exactly once
  QCOMPARE(spy.count(), 1);

  // check signal parameter
  arguments = spy.takeFirst();
  QCOMPARE(arguments.at(0).toBool(), true);

  //
  // 3. disable advanced mode
  //
  spy.clear();
  t.setAdvancedMode(false);

  // the tree must have emitted a advancedModeChanged signal exactly once
  QCOMPARE(spy.count(), 1);

  // check signal parameter
  arguments = spy.takeFirst();
  QCOMPARE(arguments.at(0).toBool(), false);
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_areFromSameNode()
{
  NTree t;
  QModelIndex index = t.index(0, 0);
  QModelIndex anotherIndex = t.index(0, 0, index);

  t.setCurrentIndex(index);

  QVERIFY(t.areFromSameNode(t.currentIndex(), index));
  QVERIFY(!t.areFromSameNode(t.currentIndex(), anotherIndex));
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_nodeByPath()
{
  NTree t;
  CNode::Ptr logNode = t.nodeByPath("cpath://Path/That/Does/Not/Exist");

  QVERIFY(logNode.get() == nullptr);

  logNode = t.nodeByPath(CLIENT_LOG_PATH);

  QCOMPARE(logNode.get(), NLog::globalLog().get());

  // note: we can freely use logNode here, even if the previous QCOMPARE() failed,
  // since a failing QCOMPARE() interrupts the test case execution
  QCOMPARE(logNode->full_path().path(), std::string(CLIENT_LOG_PATH));
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_indexFromPath()
{
  NTree t;
  QModelIndex rootIndex = t.index(0, 0);
  QModelIndex index = t.index(1, 0, rootIndex);

  CNode::Ptr node = static_cast<TreeNode*>(index.internalPointer())->node();

  // 1. get the root
  QModelIndex foundRootIndex = t.indexFromPath("cpath://Root");
  QVERIFY( foundRootIndex.isValid() );
  QCOMPARE( foundRootIndex, t.index(0, 0) );

  // 2. get another node
  QModelIndex foundIndex = t.indexFromPath(node->full_path());
  QVERIFY( foundIndex.isValid() );
  QCOMPARE( foundIndex, index );

  // 3. unexisting path
  QModelIndex badIndex = t.indexFromPath("cpath://Unexisting/Path");
  QVERIFY( !badIndex.isValid() );

  // 4. unexisting path (bis, no path but just a name)
  QModelIndex badIndexBis = t.indexFromPath("cpath:UnexistingPath");
  QVERIFY( !badIndexBis.isValid() );

  // 5. path is not a CPATH
  GUI_CHECK_THROW( t.indexFromPath("http://www.google.com"), FailedAssertion);
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_data()
{
  NTree t;
  QModelIndex logIndex = t.indexFromPath(CLIENT_LOG_PATH);
  QModelIndex logScndCol = t.index(logIndex.row(), 1, logIndex.parent());

  //
  // 1. index is not valid
  //
  QVERIFY( !t.data(QModelIndex(), Qt::DisplayRole).isValid() );

  //
  // 2. try to get the log (local component) while in non-debug mode
  //    (should return invalid data)
  //
  QVERIFY( !t.data(logIndex, Qt::DisplayRole).isValid()   );
  QVERIFY( !t.data(logIndex, Qt::ToolTip).isValid()       );
  QVERIFY( !t.data(logScndCol, Qt::DisplayRole).isValid() );
  QVERIFY( !t.data(logScndCol, Qt::ToolTip).isValid()     );

  t.setDebugModeEnabled(true);

  //
  // 3. try to get the log (local component) while in debug mode
  //    (should return correct data)
  //
  QVariant logName = t.data(logIndex, Qt::DisplayRole);
  QVariant logToolTip = t.data(logIndex, Qt::ToolTipRole);
  QVariant logToolTipScndCol = t.data(logScndCol, Qt::ToolTipRole);

  // verify data
  QCOMPARE( logName.toString(), QString(CLIENT_LOG)        );
  QCOMPARE( logToolTip.toString(), NLog().toolTip()        );
  QCOMPARE( logToolTipScndCol.toString(), NLog().toolTip() );
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_index()
{
  NTree t;
  QModelIndex index = t.index(0, 0);

  // 1. get the first item (the root), 1st column. Should be valid.
  QVERIFY( index.isValid() );

  // 2. get the first item (the root), 2nd column. Should be valid.
  QVERIFY( t.index(0, 1).isValid() );

  // 3. get the 12th child under the root. Should *not* be valid.
  QVERIFY( !t.index(12, 0, index).isValid() );
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_parent()
{
  NTree t;
  QModelIndex rootIndex = t.index(0, 0);
  QModelIndex childIndex = t.index(1, 0, rootIndex);

  QVERIFY( !t.parent(rootIndex).isValid() );
  QCOMPARE( t.parent(childIndex), rootIndex );
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_rowCount()
{
  NTree t;

  QCOMPARE(t.rowCount(), 1);
  QCOMPARE(t.rowCount(t.index(0, 0)), (int) ThreadManager::instance().tree().root()->root()->count_children());
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

void NTreeTest::test_signal_list_tree()
{
  NTree::Ptr t = NTree::globalTree();
  NGeneric::Ptr node(new NGeneric("ThisNodeShouldDisappear", "MyType"));
  XmlDoc::Ptr doc;
  SignalFrame frame("", "", "");
  NRoot::Ptr root = t->treeRoot();
  CRoot::Ptr newRoot = CRoot::create("Root");

  newRoot->create_component<CLink>("Environment");
  newRoot->create_component<CGroup>("Tools");

  newRoot->signal_list_tree( frame );

  SignalFrame replyFrame = frame.get_reply();

  // add a node
  root->addNode(node);

  // set the new tree
  GUI_CHECK_NO_THROW ( t->list_tree_reply( replyFrame ) );

  // check that the previously added node has been removed
  GUI_CHECK_THROW( root->root()->get_child("ThisNodeShouldDisappear"),  ValueNotFound);

  // check that new nodes have been added
  GUI_CHECK_NO_THROW( root->root()->get_child("Environment") );
  GUI_CHECK_NO_THROW( root->root()->get_child("Tools") );

  // check that the local components are still there
  GUI_CHECK_NO_THROW( root->root()->get_child("Log") );
  GUI_CHECK_NO_THROW( root->root()->get_child("Tree") );
  GUI_CHECK_NO_THROW( root->root()->get_child("Browsers") );
  GUI_CHECK_NO_THROW( root->root()->get_child("Tools") );
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_optionsChanged()
{
  NTree t;
  QSignalSpy spy(&t, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
  QModelIndex index = t.indexFromPath( CLIENT_LOG_PATH );
  QList<QVariant> args;

  t.optionsChanged("//A/Path/That/Does/Not/Exist");

  QCOMPARE( spy.count(), 0 );

  t.optionsChanged( CLIENT_LOG_PATH );

  QCOMPARE( spy.count(), 1 );
  args = spy.takeFirst();
  QCOMPARE( qvariant_cast<QModelIndex>(args.at(0)), index );
  QCOMPARE( qvariant_cast<QModelIndex>(args.at(1)), index );
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_nodeMatches()
{
  NTree t;
  NGeneric::Ptr node(new NGeneric("MyNode", "MyType"));
  QModelIndex rootIndex = t.indexFromPath( CLIENT_ROOT_PATH );
  QModelIndex treeIndex = t.indexFromPath( CLIENT_TREE_PATH );
  QModelIndex logIndex = t.indexFromPath( CLIENT_LOG_PATH );

  t.treeRoot()->root()->get_child_ptr("Log")->as_ptr<NLog>()->addNode( node );

  QModelIndex nodeIndex = t.indexFromPath( CLIENT_TREE_PATH "/MyNode" );

  //
  // 1. check with an invalid index
  //
  QVERIFY( !t.nodeMatches(QModelIndex(), QRegExp("(nothing)")));

  //
  // 2. check with a direct local component child while not in debug mode
  //
  QVERIFY( !t.nodeMatches(rootIndex, QRegExp("Tree", Qt::CaseSensitive)) );

  t.setDebugModeEnabled(true);

  //
  // 3. check with a direct child
  //
  // 3a. lower case and case sensitive (should return false)
  QVERIFY( !t.nodeMatches(rootIndex, QRegExp("tree", Qt::CaseSensitive)) );
  // 3b. lower case and case insensitive (should return true)
  QVERIFY( t.nodeMatches(rootIndex, QRegExp("tree", Qt::CaseInsensitive)) );
  // 3c. correct case and case sensitive (should return true)
  QVERIFY( t.nodeMatches(rootIndex, QRegExp("Tree", Qt::CaseSensitive)) );

  //
  // 4. check with a non-direct child
  //
  // 4a. lower case and case sensitive (should return false)
  QVERIFY( !t.nodeMatches(rootIndex, QRegExp("mynode", Qt::CaseSensitive)) );
  // 4b. lower case and case insensitive (should return true)
  QVERIFY( t.nodeMatches(rootIndex, QRegExp("mynode", Qt::CaseInsensitive)) );
  // 4c. correct case and case sensitive (should return true)
  QVERIFY( t.nodeMatches(rootIndex, QRegExp("MyNode", Qt::CaseSensitive)) );
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_indexIsVisible()
{
  NTree t;
  NGeneric::Ptr node(new NGeneric("Node", "MyType"));
  MyNode::Ptr myNode(new MyNode("AnotherNode"));

  t.treeRoot()->addNode(node);
  t.treeRoot()->addNode(myNode);

  QModelIndex nodeIndex = t.indexFromPath( node->full_path() );
  QModelIndex myNodeIndex = t.indexFromPath( myNode->full_path() );

  // 1. invalid index
  QVERIFY( !t.indexIsVisible( QModelIndex() ) );

  // 2. check with the root (should always be visible)
  QVERIFY( t.indexIsVisible( t.index(0,0) ) );

  //
  // 3. checks with a non-local but advanced component
  //
  // 3a. in basic mode (components are advanced by default)
  QVERIFY( !t.indexIsVisible( nodeIndex ) );
  // 3b. in advanced mode
  t.setAdvancedMode(true);
  QVERIFY( t.indexIsVisible( nodeIndex ) );
  // 3c. in advanced mode with the component marked as basic
  node->mark_basic();
  QVERIFY( t.indexIsVisible( nodeIndex ) );

  t.setAdvancedMode(true);

  //
  // 4. checks with a non-local but advanced component
  //
  // 4a. in basic mode (components are advanced by default)
  QVERIFY( !t.indexIsVisible( myNodeIndex ) );
  // 4b. in advanced mode
  t.setAdvancedMode(true);
  QVERIFY( !t.indexIsVisible( myNodeIndex ) );
  // 4c. in advanced mode with the component marked as basic
  myNode->mark_basic();
  QVERIFY( !t.indexIsVisible( myNodeIndex ) );
  // 4d. in debug mode
  t.setDebugModeEnabled(true);
  QVERIFY( t.indexIsVisible( myNodeIndex ) );
}

////////////////////////////////////////////////////////////////////////////

} // ClientTest
} // GUI
} // CF
