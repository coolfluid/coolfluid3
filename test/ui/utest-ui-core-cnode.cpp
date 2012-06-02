// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the ui CNode class"

#include <QSignalSpy>

#include <boost/assign/list_of.hpp>

#include "rapidxml/rapidxml.hpp"

#include "common/BoostAnyConversion.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Signal.hpp"

#include "common/XML/FileOperations.hpp"
#include "common/XML/Protocol.hpp"

#include "ui/core/TreeThread.hpp"
#include "ui/core/ThreadManager.hpp"
#include "ui/core/CNode.hpp"
#include "ui/core/NBrowser.hpp"
#include "ui/core/NGeneric.hpp"
#include "ui/core/NLog.hpp"
#include "ui/core/NLink.hpp"
#include "ui/core/NRoot.hpp"
#include "ui/core/NTree.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include "test/ui/CoreApplication.hpp"

//#include "test/ui/core/CommonFunctions.hpp"
#include "test/ui/MyNode.hpp"
//#include "test/ui/core/TreeHandler.hpp"

using namespace boost::assign;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::core;
using namespace cf3::ui::CoreTest;

Handle< NRoot > makeTreeFromFile()
{
  static boost::shared_ptr< XmlDoc > doc = XML::parse_file(URI("./tree.xml"));

  static boost::shared_ptr< NRoot > root = boost::dynamic_pointer_cast<NRoot>(CNode::create_from_xml(doc->content->first_node("node")));
  return Handle<NRoot>(root);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( uiCoreCNodeSuite )

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

BOOST_AUTO_TEST_CASE( component_type )
{
  MyNode node("node");

  BOOST_CHECK_EQUAL( node.component_type().toStdString(), std::string("MyNode") );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( is_client_component )
{
  boost::shared_ptr<MyNode> node(new MyNode("Node"));
  boost::shared_ptr<NBrowser> browser(new NBrowser());
  boost::shared_ptr<NGeneric> group(new NGeneric("Group", "MyType"));
  boost::shared_ptr<NLink> link(new NLink("Link"));
  boost::shared_ptr<NLog> log(new NLog());
  boost::shared_ptr<NGeneric> mesh(new NGeneric("Mesh", "MyType"));
  boost::shared_ptr<NGeneric> method(new NGeneric("Method", "MyType"));
  boost::shared_ptr<NRoot> root(new NRoot("Root"));
  boost::shared_ptr<NTree> tree(new NTree(root->handle<NRoot>()));

  BOOST_CHECK( browser->is_local_component() );
  BOOST_CHECK( !group->is_local_component()  );
  BOOST_CHECK( !link->is_local_component()   );
  BOOST_CHECK( log->is_local_component()     );
  BOOST_CHECK( !mesh->is_local_component()   );
  BOOST_CHECK( !method->is_local_component() );
  BOOST_CHECK( !root->is_local_component()   );
  BOOST_CHECK( node->is_local_component()    );
  BOOST_CHECK( tree->is_local_component()    );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_properties )
{
  MyNode node("Node");

  // an invalid tree (the type of fakePi option is unknown)
  boost::shared_ptr< XmlDoc > wrong_opt = XML::parse_cstring(
      "<node>"
      " <map>"
      "  <value key=\"properties\">"
      "   <map>"
      " 	   <value key=\"pi\" descr=\"Pi value\" is_option=\"true\">"
      "   	  <real>3.141592</real>"
      "    </value>"
      "	   <value key=\"fakePi\" descr=\"Pi value in an unknown type\" is_option=\"true\">"
      "   	  <type>3.141592</type>"
      "    </value>"
      "   </map>"
      "  </value>"
      " </map>"
      "</node>");

  // Legend for tree below:
  // (1) a string property (because "is_option" attribute is not defined)
  // (2) a bool property (because "is_option" attribute is set to false)
  // (3) a Real option
  boost::shared_ptr< XmlDoc > correct_opt = XML::parse_cstring(
      "<node>"
      " <map>"
      "  <value key=\"properties\">"
      "   <map>"
      " 	   <value key=\"prop\">"                                     // (1)
      "   	  <string>Hello, World!</string>"
      "    </value>"
      " 	   <value key=\"anotherProp\" is_option=\"false\">"          // (2)
      "    	<bool>false</bool>"
      "    </value>"
      " 	   <value key=\"pi\" descr=\"Pi value\" is_option=\"true\">" // (3)
      "    	<real>3.141592</real>"
      "    </value>"
      "   </map>"
      "  </value>"
      " </map>"
      "</node>");

  SignalArgs args_wrong(XmlNode(wrong_opt->content->first_node("node")));
  BOOST_CHECK_THROW(MyNode("Node").set_properties(args_wrong), ValueNotFound);

  SignalArgs args_correct(XmlNode(correct_opt->content->first_node("node")));
  BOOST_REQUIRE_NO_THROW(node.set_properties(args_correct));

  boost::any prop;

  //
  // Checks for "prop"
  //
  // 1. should exist
  BOOST_REQUIRE_NO_THROW( prop = node.properties()["prop"] );
  // 2. should be of type "std::string"
  BOOST_CHECK_EQUAL( any_type(prop), std::string(common::class_name<std::string>()) );
  // 3. should have the value "Hello, World!"
  BOOST_CHECK_EQUAL( any_to_value<std::string>(prop), std::string("Hello, World!") );

  //
  // Checks for "anotherProp"
  //
  // 1. should exist
  BOOST_REQUIRE_NO_THROW( prop = node.properties()["anotherProp"] );
  // 2. should be of type "bool"
  BOOST_CHECK_EQUAL( class_name_from_typeinfo(prop.type()), std::string(common::class_name<bool>()) );
  // 3. should have the value false
  BOOST_CHECK( !any_to_value<bool>(prop) );

  //
  // Checks for "pi"
  // Note: we only check that it exists and was treated as an option. The option
  // parsing is fully tested in test_makeOption()
  //
  // 1. should exist
  BOOST_REQUIRE_NO_THROW( Option & opt = node.options()["pi"] );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_signals )
{
  MyNode node("MyNode");

  // Legend for the tree below:
  // my_signal1 : with readable name, description and hidden set to false
  // my_signal2 : with readable name, description and hidden set to true
  // my_signal3 : with readable name, description and missing hidden
  // my_signal4 : with readable name and missing description and hidden
  // my_signal5 : with nothing else but the mandatory key
  boost::shared_ptr< XmlDoc > sigs = XML::parse_cstring(
      "<node>"
      " <map>"
      "  <value key=\"signals\">"
      "   <map key=\"my_signal1\" name=\"My signal 1\" descr=\"This is a 1st signal\" hidden=\"false\"/>"
      "   <map key=\"my_signal2\" name=\"My signal 2\" descr=\"This is a 2nd signal\" hidden=\"true\"/>"
      "   <map key=\"my_signal3\" name=\"My signal 3\" descr=\"This is a 3rd signal\"/>"
      "   <map key=\"my_signal4\" name=\"My signal 4\"/>"
      "   <map key=\"my_signal5\"/>"
      "  </value>"
      " </map>"
      "</node>");

  QList<ActionInfo> list;
  node.list_signals(list);
  int sigCount = list.size();

  SignalFrame frame(sigs->content->first_node("node"));
  BOOST_REQUIRE_NO_THROW( node.set_signals(frame) );

  // 4 signals should have been added (my_signal1 is hidden and should have been ignored)
  node.list_signals(list);
  BOOST_CHECK_EQUAL( list.size(), sigCount + 4 );

  // Below, the key is empty, we should have an assertion failure
  sigs = XML::parse_cstring(
      "<node>"
      " <map>"
      "  <value key=\"signals\">"
      "   <map key=\"\"/>"
      "  </value>"
      " </map>"
      "</node>");

  SignalFrame frame2(sigs->content->first_node("node"));
  BOOST_CHECK_THROW( node.set_signals(frame2), FailedAssertion );

  // remote signals list should have been cleared as well
  list.clear();
  node.list_signals( list );
  BOOST_CHECK_EQUAL( list.size(), sigCount );

  // Below, the key is missing, we should have an assertion failure
  sigs = XML::parse_cstring(
      "<node>"
      " <map>"
      "  <value key=\"signals\">"
      "   <map/>"
      "  </value>"
      " </map>"
      "</node>");

  SignalFrame frame3(sigs->content->first_node("node"));
  BOOST_CHECK_THROW( node.set_signals(frame3), FailedAssertion );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( modify_options )
{
  MyNode node("MyNode");
  QMap<QString, QString> map;

  // call with an empty map, nothing should change
  BOOST_REQUIRE_NO_THROW( node.modify_options(map) );
  BOOST_CHECK_EQUAL( node.options().value<int>("theAnswer"), int(42) );
  BOOST_CHECK_EQUAL( node.options().value<bool>("someBool"), true );
  BOOST_CHECK_EQUAL( node.options().value<std::string>("myString"), std::string("This is a string") );
  BOOST_CHECK_EQUAL( node.properties().value<Real>("someProp"), Real(3.14) );

  // modify some options
  map["someBool"] = QVariant(false).toString();
  map["theAnswer"] = QString::number(-45782446);
  BOOST_REQUIRE_NO_THROW( node.modify_options(map) );
  BOOST_CHECK_EQUAL( node.options().value<int>("theAnswer"), int(-45782446) );
  BOOST_CHECK_EQUAL( node.options().value<bool>("someBool"), false );
  BOOST_CHECK_EQUAL( node.options().value<std::string>("myString"), std::string("This is a string") );
  BOOST_CHECK_EQUAL( node.properties().value<Real>("someProp"), Real(3.14) );

  // try to modify a property (should fail)
  map["someProp"] = QString::number(2.71);
  BOOST_CHECK_THROW( node.modify_options(map), ValueNotFound );

  // option that does not exist
  map.clear();
  map["optionThatDoesNotExist"] = "Hello, World!";
  BOOST_CHECK_THROW( node.modify_options(map), ValueNotFound );

  // wrong type
  map.clear();
  map["theAnswer"] = QString::number(2.15467654);
  BOOST_CHECK_THROW( node.modify_options(map), CastingFailed );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( list_properties )
{
  boost::shared_ptr< MyNode > node( new MyNode("MyNode") );
  PropertyList& list = node->properties();
  int itemCount = list.store.size() + node->options().store.size();
  QMap<QString, QString> map;
  PropertyList::PropertyStorage_t::iterator it = list.begin();

  node->list_properties( map );

  BOOST_CHECK_EQUAL( itemCount, map.size() );

  for( ; it != list.end() ; ++it )
    BOOST_CHECK( map.contains( it->first.c_str() ) );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( list_options )
{
  MyNode node("MyNode");
  QList<boost::shared_ptr< Option > > options;

  node.list_options(options);

  // MyNode has 3 options
  BOOST_CHECK_EQUAL( options.size(), 3 );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( create_from_xml )
{
  Handle< Component > node;
  Handle< NRoot > root;
  Handle< NGeneric > group;

  BOOST_REQUIRE_NO_THROW(root = makeTreeFromFile());

  BOOST_REQUIRE_NO_THROW(node = root->get_child("Tools") );
  BOOST_REQUIRE_NO_THROW(group = node->handle<NGeneric>());
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( add_node )
{
  boost::shared_ptr< NRoot > root(new NRoot("Root"));
  boost::shared_ptr< NGeneric > node(new NGeneric("Node", "NGeneric"));
  boost::shared_ptr< NLog > log( new NLog() );
  QSignalSpy rootSpy(root->notifier(), SIGNAL(child_count_changed()));
  QSignalSpy nodeSpy(node->notifier(), SIGNAL(child_count_changed()));

  BOOST_REQUIRE_NO_THROW( root->add_node(node));
  // the component should have been added to the *real* root (Root)
  BOOST_REQUIRE_NO_THROW( root->access_component("cpath:/Node")->handle<NGeneric>() );

  BOOST_CHECK_EQUAL(rootSpy.count(), 1);

  BOOST_REQUIRE_NO_THROW( node->add_node(log) );
  BOOST_REQUIRE_NO_THROW( node->access_component("cpath:/Node/" CLIENT_LOG)->handle<NLog>() );

  BOOST_CHECK_EQUAL(nodeSpy.count(), 1);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( remove_node )
{
  boost::shared_ptr< NRoot > root(new NRoot("Root"));
  boost::shared_ptr< NGeneric > node(new NGeneric("Node", "NGeneric"));
  boost::shared_ptr< NLog > log( new NLog() );
  Component * nullComp = (Component*)nullptr;

  root->add_node(node);
  node->add_node(log);

  QSignalSpy rootSpy(root->notifier(), SIGNAL(child_count_changed()));
  QSignalSpy nodeSpy(node->notifier(), SIGNAL(child_count_changed()));

  BOOST_REQUIRE_NO_THROW( root->remove_node("Node"));
  // the component should have been removed from the REAL root (Root)
  BOOST_CHECK_EQUAL( root->access_component("cpath:/Node").get(), nullComp);

  BOOST_CHECK_EQUAL(rootSpy.count(), 1);

  BOOST_REQUIRE_NO_THROW( node->remove_node( CLIENT_LOG ) );
  BOOST_CHECK_EQUAL( root->access_component( "cpath:/Node/" CLIENT_LOG ).get(), nullComp );

  BOOST_CHECK_EQUAL( nodeSpy.count(), 1 );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( list_child_paths )
{
  /* The tree used to test:

     Root
      |---> Log (local component)
      |      |---> Node1
      |
      |---> Node2
      |      |---> Node3
      |      |---> Tree (local component)
      |      |---> Node4
      |
      |---> Node5

  */

  QStringList list;
  boost::shared_ptr< NRoot > root(new NRoot("Root"));
  boost::shared_ptr< NLog > log(new NLog());
  boost::shared_ptr< NTree > tree(new NTree(root->handle<NRoot>()));
  boost::shared_ptr< NGeneric > node1(new NGeneric("Node1", "NGeneric"));
  boost::shared_ptr< NGeneric > node2(new NGeneric("Node2", "NGeneric"));
  boost::shared_ptr< NGeneric > node3(new NGeneric("Node3", "NGeneric"));
  boost::shared_ptr< NGeneric > node4(new NGeneric("Node4", "NGeneric"));
  boost::shared_ptr< NGeneric > node5(new NGeneric("Node5", "NGeneric"));

  root->add_node(log);
  root->add_node(node2);
  root->add_node(node5);

  log->add_node(node1);

  node2->add_node(node3);
  node2->add_node(tree);
  node2->add_node(node4);

  //
  // 1. Get everything
  //
  root->list_child_paths(list, true, true);
  // should have 8 strings
  BOOST_CHECK_EQUAL( list.count(), 8);
  // check the strings
  // Nodes should be in the order they were added
  BOOST_CHECK_EQUAL( list.at(0).toStdString(), std::string("/")             );
  BOOST_CHECK_EQUAL( list.at(1).toStdString(), std::string("/Log")         );
  BOOST_CHECK_EQUAL( list.at(2).toStdString(), std::string("/Log/Node1")   );
  BOOST_CHECK_EQUAL( list.at(3).toStdString(), std::string("/Node2")       );
  BOOST_CHECK_EQUAL( list.at(4).toStdString(), std::string("/Node2/Node3") );
  BOOST_CHECK_EQUAL( list.at(5).toStdString(), std::string("/Node2/Tree") );
  BOOST_CHECK_EQUAL( list.at(6).toStdString(), std::string("/Node2/Node4")  );
  BOOST_CHECK_EQUAL( list.at(7).toStdString(), std::string("/Node5")       );

  list.clear();

  //
  // 2. Skip local components
  //
  root->list_child_paths(list, true, false);
  // should have 5 strings
  BOOST_CHECK_EQUAL( list.count(), 5);
  // check the strings
  BOOST_CHECK_EQUAL( list.at(0).toStdString(), std::string("/")             );
  BOOST_CHECK_EQUAL( list.at(1).toStdString(), std::string("/Node2")       );
  BOOST_CHECK_EQUAL( list.at(2).toStdString(), std::string("/Node2/Node3") );
  BOOST_CHECK_EQUAL( list.at(3).toStdString(), std::string("/Node2/Node4") );
  BOOST_CHECK_EQUAL( list.at(4).toStdString(), std::string("/Node5")       );

  list.clear();

  //
  // 3. Not recursive
  //
  root->list_child_paths(list, false, true);
  // should have 4 strings
  BOOST_CHECK_EQUAL( list.count(), 4);
  // check the strings
  BOOST_CHECK_EQUAL( list.at(0).toStdString(), std::string("/")       );
  BOOST_CHECK_EQUAL( list.at(1).toStdString(), std::string("/Log")   );
  BOOST_CHECK_EQUAL( list.at(2).toStdString(), std::string("/Node2") );
  BOOST_CHECK_EQUAL( list.at(3).toStdString(), std::string("/Node5") );


  list.clear();

  //
  // 4. Neither local components, nor recursive
  //
  root->list_child_paths(list, false, false);
  // should have 3 strings
  BOOST_CHECK_EQUAL( list.count(), 3);
  // check the strings
  BOOST_CHECK_EQUAL( list.at(0).toStdString(), std::string("/")       );
  BOOST_CHECK_EQUAL( list.at(1).toStdString(), std::string("/Node2") );
  BOOST_CHECK_EQUAL( list.at(2).toStdString(), std::string("/Node5") );

  list.clear();

  //
  // 5. From another component than the root
  //
  node2->list_child_paths(list, true, true);
  // should have 4 strings
  BOOST_CHECK_EQUAL( list.count(), 4);
  // check the strings
  // Nodes should be in the order they were added
  BOOST_CHECK_EQUAL( list.at(0).toStdString(), std::string("/Node2")       );
  BOOST_CHECK_EQUAL( list.at(1).toStdString(), std::string("/Node2/Node3") );
  BOOST_CHECK_EQUAL( list.at(2).toStdString(), std::string("/Node2/Tree") );
  BOOST_CHECK_EQUAL( list.at(3).toStdString(), std::string("/Node2/Node4")  );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
