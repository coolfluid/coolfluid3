// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtCore>
#include <QtTest>

#include <boost/assign/list_of.hpp>

#include "rapidxml/rapidxml.hpp"
#include "Common/XML/FileOperations.hpp"

#include "Common/OptionURI.hpp"

#include "Common/Log.hpp"

#include "Common/OptionArray.hpp"

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

  QCOMPARE(node.getComponentType(), QString("MyNode"));
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

  QCOMPARE( node.type(),    CNode::LOG_NODE     );
  QCOMPARE( browser.type(), CNode::BROWSER_NODE );
  QCOMPARE( tree.type(),    CNode::TREE_NODE    );
}

///////////////////////////////////////////////////////////////////////////////

void CNodeTest::test_checkType()
{
  NGeneric mesh("Mesh", "MyType");
  NGeneric method("Method", "MyType");
  NRoot root("Root");
  NTree tree;

  QVERIFY( mesh.checkType(CNode::GENERIC_NODE)   );
  QVERIFY( !mesh.checkType(CNode::ROOT_NODE)     );
  QVERIFY( !mesh.checkType(CNode::LINK_NODE)     );
  QVERIFY( method.checkType(CNode::GENERIC_NODE) );
  QVERIFY( root.checkType(CNode::ROOT_NODE)      );
  QVERIFY( tree.checkType(CNode::TREE_NODE)      );
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

  GUI_CHECK_THROW(MyNode("Node").setProperties(XmlNode(wrongOpt->content->first_node())), ShouldNotBeHere);

  GUI_CHECK_NO_THROW(node.setProperties(XmlNode(correctOpt->content->first_node())));

  Property * prop;

  //
  // Checks for "prop"
  //
  // 1. should exist
  GUI_CHECK_NO_THROW( prop = &node.properties()["prop"] );
  // 2. should be of type "std::string"
  QCOMPARE( QString(prop->type().c_str()), QString(Protocol::Tags::type<std::string>()) );
  // 3. should be a property
  QVERIFY( !prop->is_option() );
  // 4. should have the value "Hello, World!"
  QCOMPARE( QString(prop->value<std::string>().c_str()), QString("Hello, World!") );

  //
  // Checks for "anotherProp"
  //
  // 1. should exist
  GUI_CHECK_NO_THROW( prop = &node.properties()["anotherProp"] );
  // 2. should be of type "bool"
  QCOMPARE( QString(prop->type().c_str()), QString(Protocol::Tags::type<bool>()) );
  // 3. should be a property
  QVERIFY( !prop->is_option() );
  // 4. should have the value false
  QVERIFY( !prop->value<bool>() );

  //
  // Checks for "pi"
  // Note: we only check that it exists and was treated as an option. The option
  // parsing is fully tested in test_makeOption()
  //
  // 1. should exist
  GUI_CHECK_NO_THROW( prop = &node.properties()["pi"] );
  // 2. should be an option
  QVERIFY( prop->is_option() );

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

  SignalFrame frame(sigs->content->first_node());
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

  SignalFrame frame2(sigs->content->first_node());
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

  SignalFrame frame3(sigs->content->first_node());
  GUI_CHECK_THROW( node.setSignals(frame3), FailedAssertion );
}

///////////////////////////////////////////////////////////////////////////////

void CNodeTest::test_modifyOptions()
{
  MyNode node("MyNode");
  QMap<QString, QString> map;

  // call with an empty map, nothing should change
  GUI_CHECK_NO_THROW( node.modifyOptions(map) );
  QCOMPARE( node.property("theAnswer").value<int>(), int(42) );
  QCOMPARE( node.property("someBool").value<bool>(), true );
  QCOMPARE( node.property("myString").value<std::string>(), std::string("This is a string") );
  QCOMPARE( node.property("someProp").value<Real>(), Real(3.14) );

  // modify some options
  map["someBool"] = QVariant(false).toString();
  map["theAnswer"] = QString::number(-45782446);
  GUI_CHECK_NO_THROW( node.modifyOptions(map) );
  QCOMPARE( node.property("theAnswer").value<int>(), int(-45782446) );
  QCOMPARE( node.property("someBool").value<bool>(), false );
  QCOMPARE( node.property("myString").value<std::string>(), std::string("This is a string") );
  QCOMPARE( node.property("someProp").value<Real>(), Real(3.14) );

  // try to modify a property (should fail)
  map["someProp"] = QString::number(2.71);
  GUI_CHECK_THROW( node.modifyOptions(map), FailedAssertion );

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
  MyNode node("MyNode");
  PropertyList& list = node.properties();
  QMap<QString, QString> map;
  PropertyList::PropertyStorage_t::iterator it = list.begin();

  node.listProperties(map);

  QCOMPARE( list.store.size(), size_t(map.size()) );

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

void CNodeTest::test_makeOption()
{
  XmlDoc::Ptr xmldoc;
  Option::Ptr option;
  XmlNode node;

  //
  // 1. a Real option with a description
  //
  xmldoc = XML::parse_cstring(
      "<value key=\"theOption\" is_option=\"true\" descr=\"This is a description\">"
      "  <real>2.71</real>"
      "</value>");
  node.content = xmldoc->content->first_node();
  GUI_CHECK_NO_THROW( option = MyNode::makeOption(node) );
  // 1a. check the name
  QCOMPARE( QString(option->name().c_str()), QString("theOption"));
  // 1b. check the type
  QCOMPARE( QString(option->type().c_str()), QString("real") );
  // 1c. check the value
  QCOMPARE( option->value<Real>(), Real(2.71) );
  // 1d. check the description
  QCOMPARE( QString(option->description().c_str()), QString("This is a description") );

  //
  // 2. an Uint option with a missing description
  //
  xmldoc = XML::parse_cstring(
      "<value key=\"theAnswer\" is_option=\"true\">"
      "  <integer>42</integer>"
      "</value>");
  node.content = xmldoc->content->first_node();
  GUI_CHECK_NO_THROW( option = MyNode::makeOption(node) );
  // 2a. check the name
  QCOMPARE( QString(option->name().c_str()), QString("theAnswer"));
  // 2b. check the type
  QCOMPARE( QString(option->type().c_str()), QString("integer") );
  // 2c. check the value
  QCOMPARE( option->value<int>(), int(42) );
  // 2d. check the description
  QCOMPARE( QString(option->description().c_str()), QString("") );

  //
  // 3. no key attribute
  //
  xmldoc = XML::parse_cstring(
      "<value is_option=\"true\">"
      "  <integer>42</integer>"
      "</value>");
  node.content = xmldoc->content->first_node();
  QCOMPARE( MyNode::makeOption(node).get(), (Option*)nullptr );

  //
  // 4. option mode (basic/adv)
  //
  XmlDoc doc;
  Map map( doc.add_node( Protocol::Tags::node_map() ) );

  XmlNode theAnswer = map.set_value("theAnswer", int(42) );
  XmlNode pi = map.set_value("pi", Real(3.14159) );
  XmlNode euler = map.set_value("euler", Real(2.71) );

  // theAnswer is not marked as basic
  pi.set_attribute("mode", "basic");
  euler.set_attribute("mode", "adv");

  QVERIFY( !MyNode::makeOption(theAnswer)->has_tag("basic") );
  QVERIFY( MyNode::makeOption(pi)->has_tag("basic") );
  QVERIFY( !MyNode::makeOption(euler)->has_tag("basic") );
}

//////////////////////////////////////////////////////////////////////////

void CNodeTest::test_makeOptionTypes()
{
  XmlDoc::Ptr xmldoc(new XmlDoc());
  Option::Ptr option;

  Map map(xmldoc->add_node("map"));

  XmlNode optBool = map.set_value("optBool", true);
  XmlNode optInt = map.set_value("optInt", int(-15468));
  XmlNode optUint = map.set_value("optUint", Uint(17513214));
  XmlNode optReal = map.set_value("optReal", Real(3.14159));
  XmlNode optString = map.set_value("optString", std::string("I am a string value"));
  XmlNode optURI = map.set_value("optURI", URI("cpath://Root"));

  XmlNode wrongOpt = map.content.add_node( Protocol::Tags::node_value() );

  wrongOpt.set_attribute( Protocol::Tags::attr_key(), "optOfUnknownType");
  wrongOpt.add_node("unknown_type", "Don't know how to interpret this.");

  // 1. bool
  GUI_CHECK_NO_THROW( option = MyNode::makeOption(optBool) );
  QCOMPARE( QString(option->type().c_str()), QString("bool") );
  QCOMPARE( option->value<bool>(), true);

  // 2. int
  GUI_CHECK_NO_THROW( option = MyNode::makeOption(optInt) );
  QCOMPARE( QString(option->type().c_str()), QString("integer") );
  QCOMPARE( option->value<int>(), -15468 );

  // 3. Uint
  GUI_CHECK_NO_THROW( option = MyNode::makeOption(optUint) );
  QCOMPARE( QString(option->type().c_str()), QString("unsigned") );
  QCOMPARE( option->value<Uint>(), Uint(17513214) );

  // 4. Real
  GUI_CHECK_NO_THROW( option = MyNode::makeOption(optReal) );
  QCOMPARE( QString(option->type().c_str()), QString("real") );
  QCOMPARE( option->value<Real>(), Real(3.14159) );

  // 5. string
  GUI_CHECK_NO_THROW( option = MyNode::makeOption(optString) );
  QCOMPARE( QString(option->type().c_str()), QString("string") );
  QCOMPARE( QString(option->value<std::string>().c_str()), QString("I am a string value") );

  // 6. uri
  GUI_CHECK_NO_THROW( option = MyNode::makeOption(optURI) );
  QCOMPARE( QString(option->type().c_str()), QString("uri") );
  QCOMPARE( option->value<URI>(), URI("cpath://Root") );

  // 7. unknown type
  GUI_CHECK_THROW( MyNode::makeOption(wrongOpt), ShouldNotBeHere );
}

//////////////////////////////////////////////////////////////////////////

void CNodeTest::test_makeOptionUriSchemes()
{
  XmlDoc::Ptr xmldoc(new XmlDoc());
  Map map(xmldoc->add_node("map"));
  const char * tag = Protocol::Tags::attr_uri_protocols();

  XmlNode opt1 = map.set_value("opt1", URI());
  XmlNode opt2 = map.set_value("opt2", URI());
  XmlNode opt3 = map.set_value("opt3", URI());
  XmlNode opt4 = map.set_value("opt4", URI());
  XmlNode opt5 = map.set_value("opt5", URI());

  // opt1 has no scheme attribute defined
  opt2.set_attribute(tag, "");                  // no scheme defined
  opt3.set_attribute(tag, "http");              // one scheme
  opt4.set_attribute(tag, "cpath,https,file");  // several schemes
  opt5.set_attribute(tag, "cpath,scheme");      // a wrong scheme

  OptionURI::Ptr opt;
  std::vector<URI::Scheme::Type> vect;

  // 1. check opt1
  opt = boost::dynamic_pointer_cast<OptionURI>( MyNode::makeOption(opt1) );

  QVERIFY( opt->supported_protocols().empty() );

  // 2. check opt2
  opt = boost::dynamic_pointer_cast<OptionURI>( MyNode::makeOption(opt2) );

  QVERIFY( opt->supported_protocols().empty() );

  // 3. check opt3
  opt = boost::dynamic_pointer_cast<OptionURI>( MyNode::makeOption(opt3) );

  vect = opt->supported_protocols();

  QCOMPARE( vect.size() , size_t(1) );
  QCOMPARE( vect[0], URI::Scheme::HTTP );

  // 4. check opt4
  opt = boost::dynamic_pointer_cast<OptionURI>( MyNode::makeOption(opt4) );

  vect = opt->supported_protocols();

  QCOMPARE( vect.size() , size_t(3) );
  QCOMPARE( vect[0], URI::Scheme::CPATH );
  QCOMPARE( vect[1], URI::Scheme::HTTPS );
  QCOMPARE( vect[2], URI::Scheme::FILE  );

  // 5. check opt5
  GUI_CHECK_THROW( MyNode::makeOption( opt5 ), CastingFailed);
}

//////////////////////////////////////////////////////////////////////////

void CNodeTest::test_makeOptionRestrictedLists()
{
  XmlDoc::Ptr xmldoc(new XmlDoc());
  Map map(xmldoc->add_node("map"));
  Option::Ptr option;
  std::vector<int> vectInt = list_of<int>(344646)(544684)(446454)
                                                        (878764)(646316);

  XmlNode optInt = map.set_value("optInt", int(13));
  XmlNode optIntRestrList = map.set_value("optIntRestrList", int(-15468));

  Map(optIntRestrList).set_array(Protocol::Tags::key_restricted_values(), vectInt, " ; ");

  // test without restricted list
  GUI_CHECK_NO_THROW(option = MyNode::makeOption( optInt ) );
  QVERIFY( !option->has_restricted_list() );

  // test with restricted list
  GUI_CHECK_NO_THROW(option = MyNode::makeOption( optIntRestrList ) );

  QVERIFY( option->has_restricted_list() );

  QCOMPARE( option->restricted_list().size(), size_t( vectInt.size() + 1 ) );
  QCOMPARE( boost::any_cast<int>(option->restricted_list()[0]), int(-15468) );
  QCOMPARE( boost::any_cast<int>(option->restricted_list()[1]), vectInt[0] );
  QCOMPARE( boost::any_cast<int>(option->restricted_list()[2]), vectInt[1] );
  QCOMPARE( boost::any_cast<int>(option->restricted_list()[3]), vectInt[2] );
  QCOMPARE( boost::any_cast<int>(option->restricted_list()[4]), vectInt[3] );
  QCOMPARE( boost::any_cast<int>(option->restricted_list()[5]), vectInt[4] );
}

//////////////////////////////////////////////////////////////////////////

void CNodeTest::test_makeOptionArrayTypes()
{
  XmlDoc::Ptr xmldoc(new XmlDoc());
  Option::Ptr option;
  OptionArray::Ptr optionArray;
  std::vector<bool> readVectBool;
  std::vector<int> readVectInt;
  std::vector<Uint> readVectUint;
  std::vector<Real> readVectReal;
  std::vector<std::string> readVectString;
  std::vector<URI> readVectURI;

  Map map(xmldoc->add_node("map"));

  std::vector<bool> vectBool = list_of<bool>(false)(true)(true)(false);
  std::vector<int> vectInt = list_of<int>(5464)(-345646)(343468)(454646);
  std::vector<Uint> vectUint = list_of<Uint>(344646)(544684)(446454)(878764)(646316);
  std::vector<Real> vectReal = list_of<Real>(3.14159)(2.71)(-125431)(12.146);
  std::vector<std::string> vectString = list_of<std::string>("The first string")
                                                            ("The second string")
                                                            ("The third string")
                                                            ("The fourth string")
                                                            ("The fifth string");
  std::vector<URI> vectURI = list_of<URI>("cpath://Root")
                                         ("http://www.google.com")
                                         ("https://coolfluidsrv.vki.ac.be")
                                         ("file:/etc/fstab");


  XmlNode optBool = map.set_array("optBool", vectBool, " ; ");
  XmlNode optInt = map.set_array("optInt", vectInt, " ; ");
  XmlNode optUint = map.set_array("optUint", vectUint, " ; ");
  XmlNode optReal = map.set_array("optReal", vectReal, " ; ");
  XmlNode optString = map.set_array("optString", vectString, " ; ");
  XmlNode optURI = map.set_array("optURI", vectURI, " ; ");

//  XmlNode wrongOpt = map.content.add_node( Protocol::Tags::node_value() );

//  wrongOpt.set_attribute( Protocol::Tags::attr_key(), "optOfUnknownType");
//  wrongOpt.add_node("unknown_type", "Don't know how to interpret this.");

  //
  // 1. bool
  //
//  std::string str;
//  optBool.to_string(str);
//  qDebug() << "building phase" << str.c_str();
//  GUI_CHECK_NO_THROW( option = MyNode::makeOption(optBool) );

//  {
//    Option * option = new OptionArrayT<bool>("bla", "", std::vector<bool>());

//    CFinfo << "pointer " << option << " " <<
//        static_cast< OptionArray* >(option) << " " <<
//        static_cast< OptionArrayT<bool>* >(option) << CFendl;

//    std::vector<bool> vect;
//    Option::Ptr opt(new OptionArrayT<bool>("bla", "", vect));

//    CFinfo << "OptionArrayT " << opt.get() << " " <<
//        boost::dynamic_pointer_cast<OptionArray>(opt).get() << " " <<
//        boost::dynamic_pointer_cast<OptionArrayT<bool> >(opt).get() << CFendl;

//    Property::Ptr optT(new OptionT<bool>("bla", "", true));

//    CFinfo << "OptionT " << optT.get() << " " <<
//        boost::dynamic_pointer_cast<Option>(optT).get() << " " <<
//        boost::dynamic_pointer_cast<OptionT<bool> >(boost::dynamic_pointer_cast<Option>(optT)).get() << CFendl;


//    Property::Ptr optURI(new OptionURI("bla", "", URI()));

//    CFinfo << "OptionURI " << optURI.get() << " " <<
//        boost::dynamic_pointer_cast<Option>(optURI).get() << " " <<
//        boost::dynamic_pointer_cast<OptionURI >(optURI).get() << CFendl;

//  Component::Ptr cmp(new NLink("link"));

//  CFinfo << "NLink " << cmp.get() << " " <<
//      boost::dynamic_pointer_cast<CNode>(cmp).get() << " " <<
//      boost::dynamic_pointer_cast<NLink>(cmp).get() << CFendl;
//}
  // 1a. should be an array
//  GUI_CHECK_NO_THROW(QCOMPARE( QString(option->tag()), QString("array") ));

//  // 1b. element type should be bool
//  GUI_CHECK_NO_THROW(optionArray = boost::dynamic_pointer_cast<OptionArray>(option));
//  QCOMPARE( QString(optionArray->elem_type()), QString("bool"));

//  // get the array value
//  QCOMPARE( boost::dynamic_pointer_cast< OptionArrayT<bool> >(optionArray).get(), (OptionArrayT<bool>*)0x1564865);
//  GUI_CHECK_NO_THROW(readVectBool = boost::dynamic_pointer_cast< OptionArrayT<bool> >(optionArray)->value_vect());

  // 1c. vectors should be identical
//  QVERIFY( compareVectors(readVectBool, vectBool) );

  // 2. int
//  GUI_CHECK_NO_THROW( option = MyNode::makeOption(optInt) );
//  QCOMPARE( QString(option->type().c_str()), QString("integer") );
//  QCOMPARE( option->value<int>(), -15468 );

//  // 3. Uint
//  GUI_CHECK_NO_THROW( option = MyNode::makeOption(optUint) );
//  QCOMPARE( QString(option->type().c_str()), QString("unsigned") );
//  QCOMPARE( option->value<Uint>(), Uint(17513214) );

//  // 4. Real
//  GUI_CHECK_NO_THROW( option = MyNode::makeOption(optReal) );
//  QCOMPARE( QString(option->type().c_str()), QString("real") );
//  QCOMPARE( option->value<Real>(), Real(3.14159) );

//  // 5. string
//  GUI_CHECK_NO_THROW( option = MyNode::makeOption(optString) );
//  QCOMPARE( QString(option->type().c_str()), QString("string") );
//  QCOMPARE( QString(option->value<std::string>().c_str()), QString("I am a string value") );

//  // 6. uri
//  GUI_CHECK_NO_THROW( option = MyNode::makeOption(optURI) );
//  QCOMPARE( QString(option->type().c_str()), QString("uri") );
//  QCOMPARE( option->value<URI>(), URI("cpath://Root") );

  // 7. unknown type
//  GUI_CHECK_THROW( MyNode::makeOption(wrongOpt), ShouldNotBeHere );
}

//////////////////////////////////////////////////////////////////////////

void CNodeTest::test_makeOptionArrayRestrictedLists()
{

}

//////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // CF
