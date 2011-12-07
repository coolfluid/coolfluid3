// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the ui NTree class"

#include <QModelIndex>
#include <QSignalSpy>

#include "rapidxml/rapidxml.hpp"

#include "common/Group.hpp"
#include "common/Link.hpp"
#include "common/OptionT.hpp"

#include "common/XML/FileOperations.hpp"
#include "common/XML/Protocol.hpp"
#include "common/XML/SignalFrame.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include "ui/core/NGeneric.hpp"
#include "ui/core/NLog.hpp"
#include "ui/core/NRoot.hpp"
#include "ui/core/NTree.hpp"
#include "ui/core/ThreadManager.hpp"
#include "ui/core/TreeNode.hpp"
#include "ui/core/TreeThread.hpp"

#include "test/ui/CoreApplication.hpp"
#include "test/ui/MyNode.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::core;
using namespace cf3::ui::CoreTest;

Q_DECLARE_METATYPE(QModelIndex)

Handle< NRoot > makeTreeFromFile()
{
  static boost::shared_ptr< XmlDoc > doc = XML::parse_file(URI("./tree.xml"));

  static boost::shared_ptr< NRoot > root = boost::dynamic_pointer_cast<NRoot>(CNode::create_from_xml(doc->content->first_node("node")));
  return root->handle<NRoot>();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( uiCoreNBrowserSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  application();

  ThreadManager::instance().tree();

  AssertionManager::instance().AssertionDumps = false;
  AssertionManager::instance().AssertionThrows = true;
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionOutputs = false;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor )
{
  NTree t(ThreadManager::instance().tree().root());
  NTree t2(makeTreeFromFile());

  // the root must be the same as the client root
  //BOOST_CHECK_EQUAL(t.tree_root().get(), ThreadManager::instance().tree().root().get());
  BOOST_CHECK_EQUAL(makeTreeFromFile().get(), t2.tree_root().get());

  // the root must be different from nullptr
  BOOST_CHECK(t2.tree_root().get() != nullptr);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_root )
{
  NTree t(ThreadManager::instance().tree().root());
  boost::shared_ptr< NRoot > newRoot(new NRoot("Root"));
  QSignalSpy spy(&t, SIGNAL(layoutChanged()));

  newRoot->create_component<Link>("link");
  newRoot->create_component<Group>("Group1");
  newRoot->create_component<Group>("Group2");
  newRoot->create_component<Group>("Group3");
  newRoot->create_component<Group>("Group4");

  t.set_tree_root(newRoot->handle<NRoot>());

  // the tree must have emitted a layoutChanged signal exactly once
  BOOST_CHECK_EQUAL(spy.count(), 1);

  // newRoot must be the tree root now
  BOOST_CHECK_EQUAL(t.tree_root(), newRoot->handle<NRoot>());

  // the tree root should have 5 children now
  BOOST_CHECK_EQUAL( t.tree_root()->count_children(), std::size_t(5));
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_current_index )
{
  // QModelIndex needs to be registered. See QSignalSpy class doc.
  qRegisterMetaType<QModelIndex>("QModelIndex");
  QList<QVariant> arguments;
  NTree t(ThreadManager::instance().tree().root());
  QModelIndex index = t.current_index();
  QSignalSpy spy(&t, SIGNAL(current_index_changed(QModelIndex,QModelIndex)));

  //
  // 1. setting a correct index
  //
  t.set_current_index(t.index(0, 0));

  BOOST_CHECK(index != t.current_index());

  // the tree must have emitted a current_index_changed signal exactly once
  BOOST_CHECK_EQUAL(spy.count(), 1);

  // check signal parameters
  arguments = spy.takeFirst();
  BOOST_CHECK_EQUAL(qvariant_cast<QModelIndex>(arguments.at(0)).internalPointer(), t.current_index().internalPointer());
  BOOST_CHECK_EQUAL(qvariant_cast<QModelIndex>(arguments.at(1)).internalPointer(), index.internalPointer());

  //
  // 2. setting the same index as the current one (no signal should be emitted)
  //
  index = t.current_index();
  spy.clear();
  t.set_current_index( t.index(0,0) );

  BOOST_CHECK_EQUAL(spy.count(), 0);

  //
  // 3. setting an invalid index (should work as well)
  //
  index = t.current_index();
  spy.clear();
  t.set_current_index(QModelIndex());

  BOOST_CHECK_EQUAL(spy.count(), 1);

  // check signal parameters
  arguments = spy.takeFirst();
  BOOST_CHECK_EQUAL(qvariant_cast<QModelIndex>(arguments.at(0)).internalPointer(), QModelIndex().internalPointer());
  BOOST_CHECK_EQUAL(qvariant_cast<QModelIndex>(arguments.at(1)).internalPointer(), index.internalPointer());
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( current_path )
{
  NTree t(ThreadManager::instance().tree().root());
  QModelIndex rootIndex = t.index(0, 0);

  // 1. when the current index is not valid
  BOOST_CHECK_EQUAL( t.current_path().string(), std::string() );

  // 2. when the current index is the root
  t.set_current_index( rootIndex );
  BOOST_CHECK_EQUAL( t.current_path().string(), std::string("cpath:/") );

  // 3. when the current index is not the root (i.e the ui group)
  t.set_current_index( t.index(0, 0, rootIndex) );
  BOOST_CHECK_EQUAL( t.current_path().string(), std::string("cpath:/UI") );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( node_path )
{
  NTree t(ThreadManager::instance().tree().root());
  QModelIndex rootIndex = t.index(0, 0);

  // 1. when the index is not valid
  BOOST_CHECK_EQUAL( t.node_path( QModelIndex() ).toStdString(), std::string() );

  // 2. when the index is the root
  BOOST_CHECK_EQUAL( t.node_path( rootIndex ).toStdString(), std::string("Root/") );

  // 3. when the index is not the root (i.e the ui group)
  BOOST_CHECK_EQUAL( t.node_path( t.index(0, 0, rootIndex) ).toStdString(), std::string("Root/UI/") );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( path_from_index )
{
  NTree t(ThreadManager::instance().tree().root());
  QModelIndex rootIndex = t.index(0, 0);

  // 1. when the current index is not valid
  BOOST_CHECK_EQUAL( t.pathFromIndex( QModelIndex() ).string(), std::string() );

  // 2. when the current index is the root
  BOOST_CHECK_EQUAL( t.pathFromIndex( rootIndex ).string(), std::string("cpath:/") );

  // 3. when the current index is not the root (i.e the ui group)
  BOOST_CHECK_EQUAL( t.pathFromIndex( t.index(0, 0, rootIndex) ).string(), std::string("cpath:/UI") );

}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( list_node_options )
{
  NTree t(ThreadManager::instance().tree().root());
  boost::shared_ptr< MyNode > node(new MyNode("UselessNode"));
  QModelIndex index;
  QList<boost::shared_ptr< Option > > options;
  bool ok = false;

  //
  // 1. index is not valid
  //
  t.list_node_options(QModelIndex(), options, &ok);
  BOOST_CHECK(!ok);
  BOOST_CHECK_EQUAL(options.count(), 0);

  //
  // 2. the list is not empty
  //
  options.append( boost::shared_ptr<Option>(new OptionT<bool>("opt1", true)) );
  options.append( boost::shared_ptr<Option>(new OptionT<int>("opt2", 42)) );
  options.append( boost::shared_ptr<Option>(new OptionT<std::string>("opt3", std::string())) );
  t.list_node_options(QModelIndex(), options, &ok);
  BOOST_CHECK(!ok);
  BOOST_CHECK_EQUAL(options.count(), 0);

  //
  // 3. everything is OK
  //
  t.tree_root()->add_node(node);

  index = t.index_from_path( node->uri() );

  BOOST_CHECK(index.isValid());

  t.list_node_options(index, options, &ok);

  BOOST_CHECK(ok);
  BOOST_CHECK_EQUAL(options.count(), 3);

  t.tree_root()->remove_component(node->name());
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_advanced_mode )
{
  NTree t(ThreadManager::instance().tree().root());
  QSignalSpy spy(&t, SIGNAL(advanced_mode_changed(bool)));
  QList<QVariant> arguments;

  //
  // 1. default value
  //

  // by default, advanced is disabled
  BOOST_CHECK(!t.is_advanced_mode());

  //
  // 2. enable advanced mode
  //
  t.set_advanced_mode(true);

  // the tree must have emitted a advanced_mode_changed signal exactly once
  BOOST_CHECK_EQUAL(spy.count(), 1);

  // check signal parameter
  arguments = spy.takeFirst();
  BOOST_CHECK_EQUAL(arguments.at(0).toBool(), true);

  //
  // 3. disable advanced mode
  //
  spy.clear();
  t.set_advanced_mode(false);

  // the tree must have emitted a advanced_mode_changed signal exactly once
  BOOST_CHECK_EQUAL(spy.count(), 1);

  // check signal parameter
  arguments = spy.takeFirst();
  BOOST_CHECK_EQUAL(arguments.at(0).toBool(), false);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( are_from_same_node )
{
  NTree t(ThreadManager::instance().tree().root());
  QModelIndex index = t.index(0, 0);
  QModelIndex anotherIndex = t.index(0, 0, index);

  t.set_current_index(index);

  BOOST_CHECK( t.are_from_same_node(t.current_index(), index) );
  BOOST_CHECK( !t.are_from_same_node(t.current_index(), anotherIndex) );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( node_by_path )
{
  NTree t(ThreadManager::instance().tree().root());
  Handle< CNode > log_node = t.node_by_path("cpath:/Path/That/Does/Not/Exist") ;
  Handle<CNode> root = t.node_by_path("cpath:/");

  BOOST_CHECK(log_node.get() == nullptr);

  log_node = t.node_by_path(CLIENT_LOG_PATH);

  BOOST_REQUIRE_EQUAL(log_node.get(), NLog::global().get());

  // note: we can freely use logNode here, even if the previous BOOST_REQUIRE_EQUAL() failed,
  // since a failing BOOST_REQUIRE_EQUAL() interrupts the test case execution
  BOOST_CHECK_EQUAL(log_node->uri().path(), std::string(CLIENT_LOG_PATH));

  // check the root
  BOOST_REQUIRE_EQUAL(root.get(), ThreadManager::instance().tree().root().get());
  BOOST_CHECK_EQUAL(root->uri().path(), std::string(CLIENT_ROOT_PATH));

}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( index_from_path )
{
  NTree t(ThreadManager::instance().tree().root());
  QModelIndex rootIndex = t.index(0, 0);
  QModelIndex index = t.index(0, 0, rootIndex);

  Handle< CNode > node = static_cast<TreeNode*>(index.internalPointer())->node();


  // 1. get the root
  QModelIndex foundRootIndex = t.index_from_path("cpath:/");
  BOOST_CHECK( foundRootIndex.isValid() );
  BOOST_CHECK_EQUAL( foundRootIndex.internalPointer(), rootIndex.internalPointer() );

  // 2. get another node
  QModelIndex foundIndex = t.index_from_path(node->uri());
  BOOST_CHECK( foundIndex.isValid() );
  BOOST_CHECK_EQUAL( foundIndex.internalPointer(), index.internalPointer() );

  // 3. unexisting path
  QModelIndex badIndex = t.index_from_path("cpath:/Unexisting/Path");
  BOOST_CHECK( !badIndex.isValid() );

  // 4. unexisting path (bis, no path but just a name)
  QModelIndex badIndexBis = t.index_from_path("cpath:UnexistingPath");
  BOOST_CHECK( !badIndexBis.isValid() );

  // 5. path is not a CPATH
  BOOST_CHECK_THROW( t.index_from_path("http://www.google.com"), FailedAssertion);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( data )
{
  NTree t(ThreadManager::instance().tree().root());
  QModelIndex logIndex = t.index_from_path(CLIENT_LOG_PATH);
  QModelIndex logScndCol = t.index(logIndex.row(), 1, logIndex.parent());

  //
  // 1. index is not valid
  //
  BOOST_CHECK( !t.data(QModelIndex(), Qt::DisplayRole).isValid() );

  //
  // 2. try to get the log (local component) while in non-debug mode
  //    (should return invalid data)
  //
  BOOST_CHECK( !t.data(logIndex, Qt::DisplayRole).isValid()   );
  BOOST_CHECK( !t.data(logIndex, Qt::ToolTip).isValid()       );
  BOOST_CHECK( !t.data(logScndCol, Qt::DisplayRole).isValid() );
  BOOST_CHECK( !t.data(logScndCol, Qt::ToolTip).isValid()     );

  t.set_debug_mode_enabled(true);

  //
  // 3. try to get the log (local component) while in debug mode
  //    (should return correct data)
  //
  QVariant logName = t.data(logIndex, Qt::DisplayRole);
  QVariant logToolTip = t.data(logIndex, Qt::ToolTipRole);
  QVariant logToolTipScndCol = t.data(logScndCol, Qt::ToolTipRole);

  // verify data
  BOOST_CHECK_EQUAL( logName.toString().toStdString(), std::string(CLIENT_LOG) );
  BOOST_CHECK_EQUAL( logToolTip.toString().toStdString(), NLog().tool_tip().toStdString() );
  BOOST_CHECK_EQUAL( logToolTipScndCol.toString().toStdString(), NLog().tool_tip().toStdString() );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( index )
{
  NTree t(ThreadManager::instance().tree().root());
  QModelIndex index = t.index(0, 0);

  // 1. get the first item (the root), 1st column. Should be valid.
  BOOST_CHECK( index.isValid() );

  // 2. get the first item (the root), 2nd column. Should be valid.
  BOOST_CHECK( t.index(0, 1).isValid() );

  // 3. get the 1st child under the root. *Should* be valid.
  BOOST_CHECK( t.index(0, 0, index).isValid() );

  // 4. get the 12th child under the root. Should *not* be valid.
  BOOST_CHECK( !t.index(12, 0, index).isValid() );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( parent )
{
  NTree t(ThreadManager::instance().tree().root());
  QModelIndex rootIndex = t.index(0, 0);
  QModelIndex childIndex = t.index(0, 0, rootIndex);

  BOOST_CHECK( !t.parent(rootIndex).isValid() );
  BOOST_CHECK_EQUAL( t.parent(childIndex).internalPointer(), rootIndex.internalPointer() );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( row_count )
{
  NTree t(ThreadManager::instance().tree().root());
  TreeThread & tree = ThreadManager::instance().tree();

  BOOST_CHECK_EQUAL(t.rowCount(), 1);
  BOOST_CHECK_EQUAL(t.rowCount(t.index(0, 0)), (int) tree.root()->count_children());
  BOOST_CHECK_EQUAL(t.rowCount(t.index(0, 1)), 0);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( header_data )
{
  NTree t(ThreadManager::instance().tree().root());

  BOOST_CHECK_EQUAL(t.headerData(0, Qt::Horizontal).toString().toStdString(), std::string("Name"));
  BOOST_CHECK_EQUAL(t.headerData(1, Qt::Horizontal).toString().toStdString(), std::string("Type"));

  BOOST_CHECK(!t.headerData(0, Qt::Vertical).isValid());
  BOOST_CHECK(!t.headerData(0, Qt::Horizontal, Qt::DecorationRole).isValid());
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_debug_mode_enabled )
{
  NTree t(ThreadManager::instance().tree().root());
  QSignalSpy spy(&t, SIGNAL(layoutChanged()));

  // by default, debug mode is disabled
  BOOST_CHECK(!t.is_debug_mode_enabled());

  t.set_debug_mode_enabled(true);

  // the tree must have emitted a layout changed signal exactly once
  BOOST_CHECK_EQUAL(spy.count(), 1);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( options_changed )
{
  Handle< NTree > t = NTree::global();
  boost::shared_ptr< NGeneric > node(new NGeneric("ThisNodeShouldDisappear", "MyType"));
  Handle< XmlDoc > doc;
  SignalFrame frame;
  Handle< NRoot > root = t->tree_root();
  boost::shared_ptr<Component> newRoot = allocate_component<Group>("Root");

  newRoot->create_component<Link>("Environment");
  newRoot->create_component<Group>("Tools");

  newRoot->signal_list_tree( frame );

  SignalFrame replyFrame = frame.get_reply();

  // add a node
  root->add_node(node);

  // set the new tree
  BOOST_CHECK_NO_THROW ( t->list_tree_reply( replyFrame ) );

  // check that the previously added node has been removed
  BOOST_CHECK_THROW( root->get_child_checked("ThisNodeShouldDisappear"),  ValueNotFound);

  // check that new nodes have been added
  BOOST_CHECK_NO_THROW( root->get_child("Environment") );
  BOOST_CHECK_NO_THROW( root->get_child("Tools") );

  // check that the local components are still there
  Handle< Component > uidir;

  BOOST_CHECK_NO_THROW( uidir = root->get_child("UI") );
  BOOST_CHECK_NO_THROW( uidir->get_child("Browsers") );
  BOOST_CHECK_NO_THROW( uidir->get_child("Log") );
  BOOST_CHECK_NO_THROW( uidir->get_child("Plugins") );
  BOOST_CHECK_NO_THROW( uidir->get_child("Tree") );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( node_matches )
{
  NTree t(ThreadManager::instance().tree().root());
  QSignalSpy spy(&t, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
  QModelIndex index = t.index_from_path( CLIENT_LOG_PATH );
  QList<QVariant> args;

  t.options_changed("//A/Path/That/Does/Not/Exist");

  BOOST_CHECK_EQUAL( spy.count(), 0 );

  t.options_changed( CLIENT_LOG_PATH );

  BOOST_CHECK_EQUAL( spy.count(), 1 );
  args = spy.takeFirst();
  BOOST_CHECK_EQUAL( qvariant_cast<QModelIndex>(args.at(0)).internalPointer(), index.internalPointer() );
  BOOST_CHECK_EQUAL( qvariant_cast<QModelIndex>(args.at(1)).internalPointer(), index.internalPointer() );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( signal_list_tree )
{
  NTree t(ThreadManager::instance().tree().root());
  boost::shared_ptr< NGeneric > node(new NGeneric("MyNode", "MyType"));
  QModelIndex rootIndex = t.index_from_path( CLIENT_ROOT_PATH );
  QModelIndex treeIndex = t.index_from_path( CLIENT_TREE_PATH );
  QModelIndex logIndex = t.index_from_path( CLIENT_LOG_PATH );

  t.tree_root()->get_child("UI")->get_child("Log")->handle<NLog>()->add_node( node );

  QModelIndex nodeIndex = t.index_from_path( CLIENT_TREE_PATH "/MyNode" );

  //
  // 1. check with an invalid index
  //
  BOOST_CHECK( !t.node_matches(QModelIndex(), QRegExp("(nothing)")));

  //
  // 2. check with a direct local component child while not in debug mode
  //
  BOOST_CHECK( !t.node_matches(rootIndex, QRegExp("Tree", Qt::CaseSensitive)) );

  t.set_debug_mode_enabled(true);

  //
  // 3. check with a direct child
  //
  // 3a. lower case and case sensitive (should return false)
  BOOST_CHECK( !t.node_matches(rootIndex, QRegExp("tree", Qt::CaseSensitive)) );
  // 3b. lower case and case insensitive (should return true)
  BOOST_CHECK( t.node_matches(rootIndex, QRegExp("tree", Qt::CaseInsensitive)) );
  // 3c. correct case and case sensitive (should return true)
  BOOST_CHECK( t.node_matches(rootIndex, QRegExp("Tree", Qt::CaseSensitive)) );

  //
  // 4. check with a non-direct child
  //
  // 4a. lower case and case sensitive (should return false)
  BOOST_CHECK( !t.node_matches(rootIndex, QRegExp("mynode", Qt::CaseSensitive)) );
  // 4b. lower case and case insensitive (should return true)
  BOOST_CHECK( t.node_matches(rootIndex, QRegExp("mynode", Qt::CaseInsensitive)) );
  // 4c. correct case and case sensitive (should return true)
  BOOST_CHECK( t.node_matches(rootIndex, QRegExp("MyNode", Qt::CaseSensitive)) );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( index_is_visible )
{
  NTree t(ThreadManager::instance().tree().root());
  boost::shared_ptr< NGeneric > node(new NGeneric("Node", "MyType"));
  boost::shared_ptr< MyNode > myNode(new MyNode("AnotherNode"));

  t.tree_root()->add_node(node);
  t.tree_root()->add_node(myNode);

  QModelIndex nodeIndex = t.index_from_path( node->uri() );
  QModelIndex myNodeIndex = t.index_from_path( myNode->uri() );

  // 1. invalid index
  BOOST_CHECK( !t.check_index_visible( QModelIndex() ) );

  // 2. check with the root (should always be visible)
  BOOST_CHECK( t.check_index_visible( t.index(0,0) ) );

  //
  // 3. checks with a non-local but advanced component
  //
  // 3a. in basic mode (components are advanced by default)
  BOOST_CHECK( !t.check_index_visible( nodeIndex ) );
  // 3b. in advanced mode
  t.set_advanced_mode(true);
  BOOST_CHECK( t.check_index_visible( nodeIndex ) );
  // 3c. in advanced mode with the component marked as basic
  node->mark_basic();
  BOOST_CHECK( t.check_index_visible( nodeIndex ) );

  t.set_advanced_mode(true);

  //
  // 4. checks with a non-local but advanced component
  //
  // 4a. in basic mode (components are advanced by default)
  BOOST_CHECK( !t.check_index_visible( myNodeIndex ) );
  // 4b. in advanced mode
  t.set_advanced_mode(true);
  BOOST_CHECK( !t.check_index_visible( myNodeIndex ) );
  // 4c. in advanced mode with the component marked as basic
  myNode->mark_basic();
  BOOST_CHECK( !t.check_index_visible( myNodeIndex ) );
  // 4d. in debug mode
  t.set_debug_mode_enabled(true);
  BOOST_CHECK( t.check_index_visible( myNodeIndex ) );
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
