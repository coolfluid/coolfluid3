// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtCore>
#include <QtTest>

#include "rapidxml/rapidxml.hpp"

#include "common/CGroup.hpp"
#include "common/CLink.hpp"
#include "common/OptionT.hpp"

#include "common/XML/SignalFrame.hpp"
#include "common/XML/XmlDoc.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/NGeneric.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/Core/NRoot.hpp"
#include "UI/Core/NTree.hpp"
#include "UI/Core/ThreadManager.hpp"
#include "UI/Core/TreeNode.hpp"
#include "UI/Core/TreeThread.hpp"

#include "test/UI/Core/CommonFunctions.hpp"
#include "test/UI/Core/ExceptionThrowHandler.hpp"
#include "test/UI/Core/MyNode.hpp"

#include "test/UI/Core/NTreeTest.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::UI::Core;
using namespace cf3::UI::CoreTest;

Q_DECLARE_METATYPE(QModelIndex);

///////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace CoreTest {

/////////////////////////////////////////////////////////////////////////

void NTreeTest::test_constructor()
{
  NTree t;
  NTree t2(makeTreeFromFile());

  // the root must be the same as the client root
  QCOMPARE(t.tree_root().get(), ThreadManager::instance().tree().root().get());
  QCOMPARE(makeTreeFromFile().get(), t2.tree_root().get());

  // the root must be different from nullptr
  QVERIFY(t2.tree_root().get() != nullptr);
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_setRoot()
{
  NTree t;
  NRoot::Ptr newRoot(new NRoot("Root"));
  QSignalSpy spy(&t, SIGNAL(layoutChanged()));

  newRoot->root()->create_component_ptr<CLink>("link");
  newRoot->root()->create_component_ptr<CGroup>("Group1");
  newRoot->root()->create_component_ptr<CGroup>("Group2");
  newRoot->root()->create_component_ptr<CGroup>("Group3");
  newRoot->root()->create_component_ptr<CGroup>("Group4");

  t.set_tree_root(newRoot);

  // the tree must have emitted a layoutChanged signal exactly once
  QCOMPARE(spy.count(), 1);

  // newRoot must be the tree root now
  QCOMPARE(t.tree_root(), newRoot);

  // the tree root should have 5 children now
  QCOMPARE( t.tree_root()->root()->count_children(), std::size_t(5));
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_setCurrentIndex()
{
  // QModelIndex needs to be registered. See QSignalSpy class doc.
  qRegisterMetaType<QModelIndex>("QModelIndex");
  QList<QVariant> arguments;
  NTree t;
  QModelIndex index = t.current_index();
  QSignalSpy spy(&t, SIGNAL(current_index_changed(QModelIndex,QModelIndex)));

  //
  // 1. setting a correct index
  //
  t.set_current_index(t.index(0, 0));

  QVERIFY(index != t.current_index());

  // the tree must have emitted a current_index_changed signal exactly once
  QCOMPARE(spy.count(), 1);

  // check signal parameters
  arguments = spy.takeFirst();
  QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(0)), t.current_index());
  QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(1)), index);

  //
  // 2. setting the same index as the current one (no signal should be emitted)
  //
  index = t.current_index();
  spy.clear();
  t.set_current_index( t.index(0,0) );

  QCOMPARE(spy.count(), 0);

  //
  // 3. setting an invalid index (should work as well)
  //
  index = t.current_index();
  spy.clear();
  t.set_current_index(QModelIndex());

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
  QCOMPARE( QString( t.current_path().string().c_str()), QString() );

  // 2. when the current index is the root
  t.set_current_index( rootIndex );
  QCOMPARE( QString( t.current_path().string().c_str()), QString("cpath://Root") );

  // 3. when the current index is not the root (i.e the UI group)
  t.set_current_index( t.index(0, 0, rootIndex) );
  QCOMPARE( QString( t.current_path().string().c_str()), QString("cpath://Root/UI") );
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_nodePath()
{
  NTree t;
  QModelIndex rootIndex = t.index(0, 0);

  // 1. when the index is not valid
  QCOMPARE( t.node_path( QModelIndex() ), QString() );

  // 2. when the index is the root
  QCOMPARE( t.node_path( rootIndex ), QString("Root/") );

  // 3. when the index is not the root (i.e the UI group)
  QCOMPARE( t.node_path( t.index(0, 0, rootIndex) ), QString("Root/UI/") );
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

  // 3. when the current index is not the root (i.e the UI group)
  QCOMPARE( QString( t.pathFromIndex( t.index(0, 0, rootIndex) ).string().c_str()), QString("cpath://Root/UI") );

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
  t.list_node_options(QModelIndex(), options, &ok);
  QVERIFY(!ok);
  QCOMPARE(options.count(), 0);

  //
  // 2. the list is not empty
  //
  options.append( Option::Ptr(new OptionT<bool>("opt1", true)) );
  options.append( Option::Ptr(new OptionT<int>("opt2", 42)) );
  options.append( Option::Ptr(new OptionT<std::string>("opt3", std::string())) );
  t.list_node_options(QModelIndex(), options, &ok);
  QVERIFY(!ok);
  QCOMPARE(options.count(), 0);

  //
  // 3. everything is OK
  //
  t.tree_root()->add_node(node);

  index = t.index_from_path( node->uri() );

  QVERIFY(index.isValid());

  t.list_node_options(index, options, &ok);

  QVERIFY(ok);
  QCOMPARE(options.count(), 3);

  t.tree_root()->root()->remove_component(node->name());
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_setAdvancedMode()
{
  NTree t;
  QSignalSpy spy(&t, SIGNAL(advanced_mode_changed(bool)));
  QList<QVariant> arguments;

  //
  // 1. default value
  //

  // by default, advanced is disabled
  QVERIFY(!t.is_advanced_mode());

  //
  // 2. enable advanced mode
  //
  t.set_advanced_mode(true);

  // the tree must have emitted a advanced_mode_changed signal exactly once
  QCOMPARE(spy.count(), 1);

  // check signal parameter
  arguments = spy.takeFirst();
  QCOMPARE(arguments.at(0).toBool(), true);

  //
  // 3. disable advanced mode
  //
  spy.clear();
  t.set_advanced_mode(false);

  // the tree must have emitted a advanced_mode_changed signal exactly once
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

  t.set_current_index(index);

  QVERIFY(t.are_from_same_node(t.current_index(), index));
  QVERIFY(!t.are_from_same_node(t.current_index(), anotherIndex));
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_nodeByPath()
{
  NTree t;
  CNode::Ptr logNode = t.node_by_path("cpath://Path/That/Does/Not/Exist") ;

  QVERIFY(logNode.get() == nullptr);

  logNode = t.node_by_path(CLIENT_LOG_PATH);

  QCOMPARE(logNode.get(), NLog::global().get());

  // note: we can freely use logNode here, even if the previous QCOMPARE() failed,
  // since a failing QCOMPARE() interrupts the test case execution
  QCOMPARE(logNode->uri().path(), std::string(CLIENT_LOG_PATH));
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_indexFromPath()
{
  NTree t;
  QModelIndex rootIndex = t.index(0, 0);
  QModelIndex index = t.index(0, 0, rootIndex);

  CNode::Ptr node = static_cast<TreeNode*>(index.internalPointer())->node();


  // 1. get the root
  QModelIndex foundRootIndex = t.index_from_path("cpath://Root");
  QVERIFY( foundRootIndex.isValid() );
  QCOMPARE( foundRootIndex, rootIndex );

  // 2. get another node
  QModelIndex foundIndex = t.index_from_path(node->uri());
  QVERIFY( foundIndex.isValid() );
  QCOMPARE( foundIndex, index );

  // 3. unexisting path
  QModelIndex badIndex = t.index_from_path("cpath://Unexisting/Path");
  QVERIFY( !badIndex.isValid() );

  // 4. unexisting path (bis, no path but just a name)
  QModelIndex badIndexBis = t.index_from_path("cpath:UnexistingPath");
  QVERIFY( !badIndexBis.isValid() );

  // 5. path is not a CPATH
  GUI_CHECK_THROW( t.index_from_path("http://www.google.com"), FailedAssertion);
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_data()
{
  NTree t;
  QModelIndex logIndex = t.index_from_path(CLIENT_LOG_PATH);
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

  t.set_debug_mode_enabled(true);

  //
  // 3. try to get the log (local component) while in debug mode
  //    (should return correct data)
  //
  QVariant logName = t.data(logIndex, Qt::DisplayRole);
  QVariant logToolTip = t.data(logIndex, Qt::ToolTipRole);
  QVariant logToolTipScndCol = t.data(logScndCol, Qt::ToolTipRole);

  // verify data
  QCOMPARE( logName.toString(), QString(CLIENT_LOG)        );
  QCOMPARE( logToolTip.toString(), NLog().tool_tip()        );
  QCOMPARE( logToolTipScndCol.toString(), NLog().tool_tip() );
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

  // 3. get the 1st child under the root. *Should* be valid.
  QVERIFY( t.index(0, 0, index).isValid() );

  // 4. get the 12th child under the root. Should *not* be valid.
  QVERIFY( !t.index(12, 0, index).isValid() );
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_parent()
{
  NTree t;
  QModelIndex rootIndex = t.index(0, 0);
  QModelIndex childIndex = t.index(0, 0, rootIndex);

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
  QVERIFY(!t.is_debug_mode_enabled());

  t.set_debug_mode_enabled(true);

  // the tree must have emitted a layout changed signal exactly once
  QCOMPARE(spy.count(), 1);
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_signal_list_tree()
{
  NTree::Ptr t = NTree::global();
  NGeneric::Ptr node(new NGeneric("ThisNodeShouldDisappear", "MyType"));
  XmlDoc::Ptr doc;
  SignalFrame frame;
  NRoot::Ptr root = t->tree_root();
  CRoot::Ptr newRoot = CRoot::create("Root");

  newRoot->create_component_ptr<CLink>("Environment");
  newRoot->create_component_ptr<CGroup>("Tools");

  newRoot->signal_list_tree( frame );

  SignalFrame replyFrame = frame.get_reply();

  // add a node
  root->add_node(node);

  // set the new tree
  GUI_CHECK_NO_THROW ( t->list_tree_reply( replyFrame ) );

  // check that the previously added node has been removed
  GUI_CHECK_THROW( root->root()->get_child("ThisNodeShouldDisappear"),  ValueNotFound);

  // check that new nodes have been added
  GUI_CHECK_NO_THROW( root->root()->get_child("Environment") );
  GUI_CHECK_NO_THROW( root->root()->get_child("Tools") );

  // check that the local components are still there
  Component::Ptr uidir;

  GUI_CHECK_NO_THROW( uidir = root->root()->get_child_ptr("UI") );
  GUI_CHECK_NO_THROW( uidir->get_child("Browsers") );
  GUI_CHECK_NO_THROW( uidir->get_child("Log") );
  GUI_CHECK_NO_THROW( uidir->get_child("Plugins") );
  GUI_CHECK_NO_THROW( uidir->get_child("Tree") );
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_optionsChanged()
{
  NTree t;
  QSignalSpy spy(&t, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
  QModelIndex index = t.index_from_path( CLIENT_LOG_PATH );
  QList<QVariant> args;

  t.options_changed("//A/Path/That/Does/Not/Exist");

  QCOMPARE( spy.count(), 0 );

  t.options_changed( CLIENT_LOG_PATH );

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
  QModelIndex rootIndex = t.index_from_path( CLIENT_ROOT_PATH );
  QModelIndex treeIndex = t.index_from_path( CLIENT_TREE_PATH );
  QModelIndex logIndex = t.index_from_path( CLIENT_LOG_PATH );

  t.tree_root()->root()->get_child("UI").get_child("Log").as_type<NLog>().add_node( node );

  QModelIndex nodeIndex = t.index_from_path( CLIENT_TREE_PATH "/MyNode" );

  //
  // 1. check with an invalid index
  //
  QVERIFY( !t.node_matches(QModelIndex(), QRegExp("(nothing)")));

  //
  // 2. check with a direct local component child while not in debug mode
  //
  QVERIFY( !t.node_matches(rootIndex, QRegExp("Tree", Qt::CaseSensitive)) );

  t.set_debug_mode_enabled(true);

  //
  // 3. check with a direct child
  //
  // 3a. lower case and case sensitive (should return false)
  QVERIFY( !t.node_matches(rootIndex, QRegExp("tree", Qt::CaseSensitive)) );
  // 3b. lower case and case insensitive (should return true)
  QVERIFY( t.node_matches(rootIndex, QRegExp("tree", Qt::CaseInsensitive)) );
  // 3c. correct case and case sensitive (should return true)
  QVERIFY( t.node_matches(rootIndex, QRegExp("Tree", Qt::CaseSensitive)) );

  //
  // 4. check with a non-direct child
  //
  // 4a. lower case and case sensitive (should return false)
  QVERIFY( !t.node_matches(rootIndex, QRegExp("mynode", Qt::CaseSensitive)) );
  // 4b. lower case and case insensitive (should return true)
  QVERIFY( t.node_matches(rootIndex, QRegExp("mynode", Qt::CaseInsensitive)) );
  // 4c. correct case and case sensitive (should return true)
  QVERIFY( t.node_matches(rootIndex, QRegExp("MyNode", Qt::CaseSensitive)) );
}

////////////////////////////////////////////////////////////////////////////

void NTreeTest::test_indexIsVisible()
{
  NTree t;
  NGeneric::Ptr node(new NGeneric("Node", "MyType"));
  MyNode::Ptr myNode(new MyNode("AnotherNode"));

  t.tree_root()->add_node(node);
  t.tree_root()->add_node(myNode);

  QModelIndex nodeIndex = t.index_from_path( node->uri() );
  QModelIndex myNodeIndex = t.index_from_path( myNode->uri() );

  // 1. invalid index
  QVERIFY( !t.check_index_visible( QModelIndex() ) );

  // 2. check with the root (should always be visible)
  QVERIFY( t.check_index_visible( t.index(0,0) ) );

  //
  // 3. checks with a non-local but advanced component
  //
  // 3a. in basic mode (components are advanced by default)
  QVERIFY( !t.check_index_visible( nodeIndex ) );
  // 3b. in advanced mode
  t.set_advanced_mode(true);
  QVERIFY( t.check_index_visible( nodeIndex ) );
  // 3c. in advanced mode with the component marked as basic
  node->mark_basic();
  QVERIFY( t.check_index_visible( nodeIndex ) );

  t.set_advanced_mode(true);

  //
  // 4. checks with a non-local but advanced component
  //
  // 4a. in basic mode (components are advanced by default)
  QVERIFY( !t.check_index_visible( myNodeIndex ) );
  // 4b. in advanced mode
  t.set_advanced_mode(true);
  QVERIFY( !t.check_index_visible( myNodeIndex ) );
  // 4c. in advanced mode with the component marked as basic
  myNode->mark_basic();
  QVERIFY( !t.check_index_visible( myNodeIndex ) );
  // 4d. in debug mode
  t.set_debug_mode_enabled(true);
  QVERIFY( t.check_index_visible( myNodeIndex ) );
}

////////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // cf3
