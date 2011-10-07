// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtCore>
#include <QtTest>

#include <boost/assign/list_of.hpp>

#include "rapidxml/rapidxml.hpp"

#include "Common/BoostAnyConversion.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionArray.hpp"
#include "Common/Signal.hpp"

#include "Common/XML/FileOperations.hpp"
#include "Common/XML/Protocol.hpp"

#include "UI/Core/TreeThread.hpp"
#include "UI/Core/CNode.hpp"
#include "UI/Core/NBrowser.hpp"
#include "UI/Core/NGeneric.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/Core/NLink.hpp"
#include "UI/Core/NRoot.hpp"
#include "UI/Core/NTree.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "test/UI/Core/CommonFunctions.hpp"
#include "test/UI/Core/ExceptionThrowHandler.hpp"
#include "test/UI/Core/MyNode.hpp"
#include "test/UI/Core/TreeHandler.hpp"

#include "test/UI/Core/CNodeTest.hpp"

using namespace boost::assign;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::UI::Core;
using namespace CF::UI::CoreTest;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace CoreTest {

//////////////////////////////////////////////////////////////////////////

template<typename TYPE>
bool compareVectors(const std::vector<TYPE> & left, const std::vector<TYPE> & right)
{
  bool equal = left.size() == right.size();

  if(!equal)
    qDebug() << "Sizes are different. Left has" << left.size() <<
        "elements whereas right has"<< right.size() << "elements.";

  for(unsigned int i = 0 ; i < left.size() && equal ; ++i)
  {
    equal = left[i] == right[i];

    if(!equal)
      qDebug() <<  "Item" << i << ": [" << Common::to_str(left[i]).c_str() <<
          "] is different from [" << Common::to_str(right[i]).c_str() << "].";
  }

  return equal;
}

///////////////////////////////////////////////////////////////////////////////

void CNodeTest::test_getComponentType()
{
  MyNode node("Node");

  QCOMPARE(node.componentType(), QString("MyNode"));
}

///////////////////////////////////////////////////////////////////////////////

void CNodeTest::test_isClientComponent()
{
  MyNode node("Node");
  NBrowser browser;
  NGeneric group("Group", "MyType");
  NLink link("Link");
  NLog log;
  NGeneric mesh("Mesh", "MyType");
  NGeneric method("Method", "MyType");
  NRoot root("Root");
  NTree tree;

  QVERIFY( browser.isLocalComponent() );
  QVERIFY( !group.isLocalComponent()  );
  QVERIFY( !link.isLocalComponent()   );
  QVERIFY( log.isLocalComponent()     );
  QVERIFY( !mesh.isLocalComponent()   );
  QVERIFY( !method.isLocalComponent() );
  QVERIFY( !root.isLocalComponent()   );
  QVERIFY( node.isLocalComponent()    );
  QVERIFY( tree.isLocalComponent()    );
}

///////////////////////////////////////////////////////////////////////////////

void CNodeTest::test_getType()
{
  MyNode node("Node");
  NBrowser browser;
  NTree tree;

//  QCOMPARE( node.type(),    CNode::LOG_NODE     );
//  QCOMPARE( browser.type(), CNode::BROWSER_NODE );
//  QCOMPARE( tree.type(),    CNode::TREE_NODE    );
}

///////////////////////////////////////////////////////////////////////////////

void CNodeTest::test_checkType()
{
  NGeneric mesh("Mesh", "MyType");
  NGeneric method("Method", "MyType");
  NRoot root("Root");
  NTree tree;

//  QVERIFY( mesh.checkType(CNode::GENERIC_NODE)   );
//  QVERIFY( !mesh.checkType(CNode::ROOT_NODE)     );
//  QVERIFY( !mesh.checkType(CNode::LINK_NODE)     );
//  QVERIFY( method.checkType(CNode::GENERIC_NODE) );
//  QVERIFY( root.checkType(CNode::ROOT_NODE)      );
//  QVERIFY( tree.checkType(CNode::TREE_NODE)      );
}

///////////////////////////////////////////////////////////////////////////////

void CNodeTest::test_setProperties()
{
  MyNode node("Node");

  // an invalid tree (the type of fakePi option is unknown)
  XmlDoc::Ptr wrongOpt = XML::parse_cstring(
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
  XmlDoc::Ptr correctOpt = XML::parse_cstring(
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

  SignalArgs args_wrong(XmlNode(wrongOpt->content->first_node("node")));
  GUI_CHECK_THROW(MyNode("Node").setProperties(args_wrong), ShouldNotBeHere);

  SignalArgs args_correct(XmlNode(correctOpt->content->first_node("node")));
  GUI_CHECK_NO_THROW(node.setProperties(args_correct));

  boost::any prop;

  //
  // Checks for "prop"
  //
  // 1. should exist
  GUI_CHECK_NO_THROW( prop = node.properties()["prop"] );
  // 2. should be of type "std::string"
  QCOMPARE( QString( any_type(prop).c_str() ), QString(Protocol::Tags::type<std::string>()) );
  // 3. should have the value "Hello, World!"
  QCOMPARE( QString( any_to_value<std::string>(prop).c_str() ), QString("Hello, World!") );

  //
  // Checks for "anotherProp"
  //
  // 1. should exist
  GUI_CHECK_NO_THROW( prop = node.properties()["anotherProp"] );
  // 2. should be of type "bool"
  QCOMPARE( QString( class_name_from_typeinfo(prop.type()).c_str() ), QString(Protocol::Tags::type<bool>()) );
  // 3. should have the value false
  QVERIFY( !any_to_value<bool>(prop) );

  //
  // Checks for "pi"
  // Note: we only check that it exists and was treated as an option. The option
  // parsing is fully tested in test_makeOption()
  //
  // 1. should exist
  GUI_CHECK_NO_THROW( Option & opt = node.options()["pi"] );
}

///////////////////////////////////////////////////////////////////////////////

void CNodeTest::test_setSignals()
{
  MyNode node("MyNode");

  // Legend for the tree below:
  // my_signal1 : with readable name, description and hidden set to false
  // my_signal2 : with readable name, description and hidden set to true
  // my_signal3 : with readable name, description and missing hidden
  // my_signal4 : with readable name and missing description and hidden
  // my_signal5 : with nothing else but the mandatory key
  XmlDoc::Ptr sigs = XML::parse_cstring(
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
  node.listSignals(list);
  int sigCount = list.size();

  SignalFrame frame(sigs->content->first_node("node"));
  GUI_CHECK_NO_THROW( node.setSignals(frame) );

  // 4 signals should have been added (my_signal1 is hidden and should have been ignored)
  node.listSignals(list);
  QCOMPARE( list.size(), sigCount + 4 );

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
  GUI_CHECK_THROW( node.setSignals(frame2), FailedAssertion );

  // remote signals list should have been cleared as well
  list.clear();
  node.listSignals( list );
  QCOMPARE( list.size(), sigCount );

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
  GUI_CHECK_THROW( node.setSignals(frame3), FailedAssertion );
}

///////////////////////////////////////////////////////////////////////////////

void CNodeTest::test_modifyOptions()
{
  MyNode node("MyNode");
  QMap<QString, QString> map;

  // call with an empty map, nothing should change
  GUI_CHECK_NO_THROW( node.modifyOptions(map) );
  QCOMPARE( node.option("theAnswer").value<int>(), int(42) );
  QCOMPARE( node.option("someBool").value<bool>(), true );
  QCOMPARE( node.option("myString").value<std::string>(), std::string("This is a string") );
  QCOMPARE( node.properties().value<Real>("someProp"), Real(3.14) );

  // modify some options
  map["someBool"] = QVariant(false).toString();
  map["theAnswer"] = QString::number(-45782446);
  GUI_CHECK_NO_THROW( node.modifyOptions(map) );
  QCOMPARE( node.option("theAnswer").value<int>(), int(-45782446) );
  QCOMPARE( node.option("someBool").value<bool>(), false );
  QCOMPARE( node.option("myString").value<std::string>(), std::string("This is a string") );
  QCOMPARE( node.properties().value<Real>("someProp"), Real(3.14) );

  // try to modify a property (should fail)
  map["someProp"] = QString::number(2.71);
  GUI_CHECK_THROW( node.modifyOptions(map), ValueNotFound );

  // option that does not exist
  map.clear();
  map["optionThatDoesNotExist"] = "Hello, World!";
  GUI_CHECK_THROW( node.modifyOptions(map), ValueNotFound );

  // wrong type
  map.clear();
  map["theAnswer"] = QString::number(2.15467654);
  GUI_CHECK_THROW( node.modifyOptions(map), CastingFailed );
}

///////////////////////////////////////////////////////////////////////////////

void CNodeTest::test_listProperties()
{
  MyNode::Ptr node( new MyNode("MyNode") );
  PropertyList& list = node->properties();
  int itemCount = list.store.size() + node->options().store.size();
  QMap<QString, QString> map;
  PropertyList::PropertyStorage_t::iterator it = list.begin();

  node->listProperties( map );

  QCOMPARE( itemCount, map.size() );

  for( ; it != list.end() ; ++it )
    QVERIFY( map.contains( it->first.c_str() ) );
}

///////////////////////////////////////////////////////////////////////////////

void CNodeTest::test_listOptions()
{
  MyNode node("MyNode");
  QList<Option::ConstPtr> options;

  node.listOptions(options);

  // MyNode has 3 options
  QCOMPARE( options.size(), 3 );
}

///////////////////////////////////////////////////////////////////////////////

void CNodeTest::test_createFromXml()
{
  CNode::Ptr node;
  NRoot::Ptr root;
  NGeneric::Ptr group;

  GUI_CHECK_NO_THROW(root = makeTreeFromFile());

  GUI_CHECK_NO_THROW(node = boost::dynamic_pointer_cast<CNode>(root->root()->get_child_ptr("Tools")));
  GUI_CHECK_NO_THROW(group = node->castTo<NGeneric>());
}

///////////////////////////////////////////////////////////////////////////////

void CNodeTest::test_addNode()
{
  NRoot::Ptr root(new NRoot("Root"));
  NGeneric::Ptr node(new NGeneric("Node", "NGeneric"));
  NLog::Ptr log( new NLog() );
  QSignalSpy rootSpy(root->notifier(), SIGNAL(childCountChanged()));
  QSignalSpy nodeSpy(node->notifier(), SIGNAL(childCountChanged()));

  GUI_CHECK_NO_THROW( root->addNode(node));
  // the component should have been added to the *real* root (CRoot)
  GUI_CHECK_NO_THROW( root->root()->access_component_ptr("cpath://Root/Node")->as_ptr<NGeneric>() );

  QCOMPARE(rootSpy.count(), 1);

  GUI_CHECK_NO_THROW( node->addNode(log) );
  GUI_CHECK_NO_THROW( node->access_component_ptr("cpath://Root/Node/" CLIENT_LOG)->as_ptr<NLog>() );

  QCOMPARE(nodeSpy.count(), 1);
}

//////////////////////////////////////////////////////////////////////////

void CNodeTest::test_removeNode()
{
  NRoot::Ptr root(new NRoot("Root"));
  NGeneric::Ptr node(new NGeneric("Node", "NGeneric"));
  NLog::Ptr log( new NLog() );
  Component * nullComp = (Component*)nullptr;

  root->addNode(node);
  node->addNode(log);

  QSignalSpy rootSpy(root->notifier(), SIGNAL(childCountChanged()));
  QSignalSpy nodeSpy(node->notifier(), SIGNAL(childCountChanged()));

  GUI_CHECK_NO_THROW( root->removeNode("Node"));
  // the component should have been removed from the REAL root (CRoot)
  QCOMPARE( root->root()->access_component_ptr("cpath://Root/Node").get(), nullComp);

  QCOMPARE(rootSpy.count(), 1);

  GUI_CHECK_NO_THROW( node->removeNode( CLIENT_LOG ) );
  QCOMPARE( root->root()->access_component_ptr( "cpath://Root/Node/" CLIENT_LOG ).get(), nullComp );

  QCOMPARE(nodeSpy.count(), 1);
}

//////////////////////////////////////////////////////////////////////////

void CNodeTest::test_listChildPaths()
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
  NRoot::Ptr root(new NRoot("Root"));
  NLog::Ptr log(new NLog());
  NTree::Ptr tree(new NTree(root));
  NGeneric::Ptr node1(new NGeneric("Node1", "NGeneric"));
  NGeneric::Ptr node2(new NGeneric("Node2", "NGeneric"));
  NGeneric::Ptr node3(new NGeneric("Node3", "NGeneric"));
  NGeneric::Ptr node4(new NGeneric("Node4", "NGeneric"));
  NGeneric::Ptr node5(new NGeneric("Node5", "NGeneric"));

  root->addNode(log);
  root->addNode(node2);
  root->addNode(node5);

  log->addNode(node1);

  node2->addNode(node3);
  node2->addNode(tree);
  node2->addNode(node4);

  //
  // 1. Get everything
  //
  root->listChildPaths(list, true, true);
  // should have 8 strings
  QCOMPARE( list.count(), 8);
  // check the strings
  // note: Tree is after Node4 (although it was added before), because nodes
  // are read in alphabetical order
  QCOMPARE( list.at(0), QString("//Root")             );
  QCOMPARE( list.at(1), QString("//Root/Log")         );
  QCOMPARE( list.at(2), QString("//Root/Log/Node1")   );
  QCOMPARE( list.at(3), QString("//Root/Node2")       );
  QCOMPARE( list.at(4), QString("//Root/Node2/Node3") );
  QCOMPARE( list.at(5), QString("//Root/Node2/Node4") );
  QCOMPARE( list.at(6), QString("//Root/Node2/Tree")  );
  QCOMPARE( list.at(7), QString("//Root/Node5")       );

  list.clear();

  //
  // 2. Skip local components
  //
  root->listChildPaths(list, true, false);
  // should have 5 strings
  QCOMPARE( list.count(), 5);
  // check the strings
  QCOMPARE( list.at(0), QString("//Root")             );
  QCOMPARE( list.at(1), QString("//Root/Node2")       );
  QCOMPARE( list.at(2), QString("//Root/Node2/Node3") );
  QCOMPARE( list.at(3), QString("//Root/Node2/Node4") );
  QCOMPARE( list.at(4), QString("//Root/Node5")       );

  list.clear();

  //
  // 3. Not recursive
  //
  root->listChildPaths(list, false, true);
  // should have 4 strings
  QCOMPARE( list.count(), 4);
  // check the strings
  QCOMPARE( list.at(0), QString("//Root")       );
  QCOMPARE( list.at(1), QString("//Root/Log")   );
  QCOMPARE( list.at(2), QString("//Root/Node2") );
  QCOMPARE( list.at(3), QString("//Root/Node5") );


  list.clear();

  //
  // 4. Neither local components, nor recursive
  //
  root->listChildPaths(list, false, false);
  // should have 3 strings
  QCOMPARE( list.count(), 3);
  // check the strings
  QCOMPARE( list.at(0), QString("//Root")       );
  QCOMPARE( list.at(1), QString("//Root/Node2") );
  QCOMPARE( list.at(2), QString("//Root/Node5") );

  list.clear();

  //
  // 5. From another component than the root
  //
  node2->listChildPaths(list, true, true);
  // should have 4 strings
  QCOMPARE( list.count(), 4);
  // check the strings
  // note: Tree is after Node4 (although it was added before), because nodes
  // are read in alphabetical order
  QCOMPARE( list.at(0), QString("//Root/Node2")       );
  QCOMPARE( list.at(1), QString("//Root/Node2/Node3") );
  QCOMPARE( list.at(2), QString("//Root/Node2/Node4") );
  QCOMPARE( list.at(3), QString("//Root/Node2/Tree")  );
}

//////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // CF
