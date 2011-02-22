// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtCore>
#include <QtTest>

#include "rapidxml/rapidxml.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/CNode.hpp"
#include "GUI/Client/Core/NBrowser.hpp"
#include "GUI/Client/Core/NGeneric.hpp"
#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/Core/NLink.hpp"
#include "GUI/Client/Core/NRoot.hpp"
#include "GUI/Client/Core/NTree.hpp"

#include "test/GUI/Client/CommonFunctions.hpp"
#include "test/GUI/Client/ExceptionThrowHandler.hpp"
#include "test/GUI/Client/MyNode.hpp"
#include "test/GUI/Client/TreeHandler.hpp"

#include "test/GUI/Client/CNodeTest.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientTest;

///////////////////////////////////////////////////////////////////////////

void CNodeTest::test_getComponentType()
{
  MyNode node("Node");

  QCOMPARE(node.getComponentType(), QString("MyNode"));
}

///////////////////////////////////////////////////////////////////////////

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

  QVERIFY(browser.isClientComponent());
  QVERIFY(!group.isClientComponent());
  QVERIFY(!link.isClientComponent());
  QVERIFY(log.isClientComponent());
  QVERIFY(!mesh.isClientComponent());
  QVERIFY(!method.isClientComponent());
  QVERIFY(!root.isClientComponent());
  QVERIFY(!node.isClientComponent());
  QVERIFY(tree.isClientComponent());
}

///////////////////////////////////////////////////////////////////////////

void CNodeTest::test_getType()
{
  MyNode node("Node");
  NBrowser browser;
  NLog log;

  QCOMPARE(node.type(), CNode::GENERIC_NODE);
  QCOMPARE(browser.type(), CNode::BROWSER_NODE);
  QCOMPARE(log.type(), CNode::LOG_NODE);
}

///////////////////////////////////////////////////////////////////////////

void CNodeTest::test_checkType()
{
  NGeneric mesh("Mesh", "MyType");
  NGeneric method("Method", "MyType");
  NRoot root("Root");
  NTree tree;

  QVERIFY(mesh.checkType(CNode::GENERIC_NODE));
  QVERIFY(!mesh.checkType(CNode::ROOT_NODE));
  QVERIFY(!mesh.checkType(CNode::LINK_NODE));
  QVERIFY(method.checkType(CNode::GENERIC_NODE));
  QVERIFY(root.checkType(CNode::ROOT_NODE));
  QVERIFY(tree.checkType(CNode::TREE_NODE));
}

///////////////////////////////////////////////////////////////////////////

void CNodeTest::test_setOptions()
{
  MyNode node("Node");

  XmlDoc::Ptr wrongOpt = XmlDoc::parse_string("<node><map>"
      "<value key=\"properties\">"
      " <map>"
      " 	<value key=\"pi\" descr=\"Pi value\">"
      "   	<real>3.141592</real>"
      "  </value>"
      "	 <value key=\"fakePi\" descr=\"Pi value in an unknown type\">"
      "   	<type>3.141592</type>"
      "  </value>"
      " </map>"
      "</value>"
      "</map></node>");

  XmlDoc::Ptr correctOpt = XmlDoc::parse_string("<node><map>"
      "<value key=\"properties\">"
      " <map>"
      " 	<value key=\"pi\" descr=\"Pi value\">"
      "   	<real>3.141592</real>"
      "  </value>"
      "	 <value key=\"fakePi\" descr=\"Pi value in an unknown type\">"
      "   	<string>Hello world!</string>"
      "  </value>"
      " </map>"
      "</value>"
      "</map></node>");

    GUI_CHECK_THROW(MyNode("Node").setProperties(XmlNode(wrongOpt->content->first_node())), ShouldNotBeHere);

    GUI_CHECK_NO_THROW(node.setProperties(XmlNode(correctOpt->content->first_node())));
}

///////////////////////////////////////////////////////////////////////////

void CNodeTest::test_getOptions()
{
  TreeHandler th;
  MyNode::Ptr node(new MyNode("Node"));

  QList<Option::ConstPtr> nodeOptList;

  XmlDoc::Ptr options = XmlDoc::parse_string("<node><map>"
      "<value key=\"options\">"
      " <map>"
      " 	<value key=\"pi\" descr=\"Pi value\">"
      "   	<real>3.141592</real>"
      "  </value>"
      "	 <value key=\"hello\" descr=\"Some bool\">"
      "   	<bool>false</bool>"
      "  </value>"
      " </map>"
      "</value>"
      "</map></node>");

  th.add(node);

  node->setProperties(XmlNode(options->content->first_node()));

  GUI_CHECK_NO_THROW(node->options(nodeOptList));

  // there were already 2 options in MyNode + 2 new options => 4 options
  QCOMPARE(nodeOptList.count(), 0);
}

///////////////////////////////////////////////////////////////////////////

void CNodeTest::test_createFromXml()
{
  CNode::Ptr node;
  NRoot::Ptr root;
  NGeneric::Ptr group;
  QList<Option::ConstPtr> optList;
  Option::ConstPtr option;

 // GUI_CHECK_NO_THROW(doc = XmlOps::parse(boost::filesystem::path("./tree.xml")));
  GUI_CHECK_NO_THROW(root = makeTreeFromFile());

//  GUI_CHECK_NO_THROW(node = boost::dynamic_pointer_cast<CNode>(root->root()->get_child("Tools")));
//  GUI_CHECK_NO_THROW(group = node->castTo<NGeneric>());

//  GUI_CHECK_NO_THROW(group->options(optList));

//  QCOMPARE(optList.count(), 0);

//  option = optList.at(0);

//  QCOMPARE(option.m_paramAdv, true);
//  QCOMPARE(option.m_paramName, QString("pi"));
//  QCOMPARE(option.m_paramDescr, QString("Pi in a CGroup"));
//  QCOMPARE(option.m_paramValue, QString("3.14159"));
//  QCOMPARE(option.m_paramType, OptionType::TYPE_DOUBLE);
}

///////////////////////////////////////////////////////////////////////////

void CNodeTest::test_addNode()
{
  NRoot::Ptr root(new NRoot("Root"));
  NGeneric::Ptr node(new NGeneric("Node", "NGeneric"));
  QSignalSpy spy(root->notifier(), SIGNAL(childCountChanged()));

  GUI_CHECK_NO_THROW(root->addNode(node));

  QCOMPARE(spy.count(), 1);
}

//////////////////////////////////////////////////////////////////////////

void CNodeTest::test_removeNode()
{

}
