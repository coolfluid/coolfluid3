#include <QtCore>
#include <QtTest>

#include "Common/XmlHelpers.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/CNode.hpp"
#include "GUI/Client/NBrowser.hpp"
#include "GUI/Client/NGroup.hpp"
#include "GUI/Client/NLog.hpp"
#include "GUI/Client/NLink.hpp"
#include "GUI/Client/NMesh.hpp"
#include "GUI/Client/NMethod.hpp"
#include "GUI/Client/NRoot.hpp"
#include "GUI/Client/NTree.hpp"

#include "GUI/Client/uTests/ExceptionThrowHandler.hpp"
#include "GUI/Client/uTests/MyNode.hpp"

#include "GUI/Client/uTests/CNodeTest.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;
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
  NGroup group("Group");
  NLink link("Link", "//Path/to/target");
  NLog log;
  NMesh mesh("Mesh");
  NMethod method("Method");
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

  QCOMPARE(node.getType(), CNode::GROUP_NODE);
  QCOMPARE(browser.getType(), CNode::BROWSER_NODE);
  QCOMPARE(log.getType(), CNode::LOG_NODE);
}

///////////////////////////////////////////////////////////////////////////

void CNodeTest::test_checkType()
{
  NMesh mesh("Mesh");
  NMethod method("Method");
  NRoot root("Root");
  NTree tree;

  QVERIFY(mesh.checkType(CNode::MESH_NODE));
  QVERIFY(!mesh.checkType(CNode::GROUP_NODE));
  QVERIFY(!mesh.checkType(CNode::LINK_NODE));
  QVERIFY(method.checkType(CNode::METHOD_NODE));
  QVERIFY(root.checkType(CNode::ROOT_NODE));
  QVERIFY(tree.checkType(CNode::TREE_NODE));
}

///////////////////////////////////////////////////////////////////////////

void CNodeTest::test_setOptions()
{
  MyNode node("Node");

  boost::shared_ptr<XmlDoc> wrongOpt = XmlOps::parse(std::string("<valuemap>"
      "	<value key=\"pi\" descr=\"Pi value\">"
      " 	<double>3.141592</double>"
      " </value>"
      "	<value key=\"fakePi\" descr=\"Pi value in an unknown type\">"
      " 	<type>3.141592</type>"
      " </value>"
      "</valuemap>"));

  boost::shared_ptr<XmlDoc> correctOpt = XmlOps::parse(std::string("<valuemap>"
      "	<value key=\"pi\" descr=\"Pi value\">"
      " 	<double>3.141592</double>"
      " </value>"
      "	<value key=\"hello\" descr=\"Some string\">"
      " 	<string>Hello World!</type>"
      " </value>"
      "</valuemap>"));

  GUI_CHECK_THROW(MyNode("Node").setOptions(*wrongOpt->first_node()), ShouldNotBeHere);

  GUI_CHECK_NO_THROW(node.setOptions(*correctOpt->first_node()));
}

///////////////////////////////////////////////////////////////////////////

void CNodeTest::test_getOptions()
{
  NRoot::Ptr root = ClientRoot::getRoot();
  MyNode::Ptr node(new MyNode("Node"));
  NLink::Ptr link(new NLink("Link", "//Simulator/Node")) ;
  NLink::Ptr badLink(new NLink("BadLink", "//Invalid/Path"));

  QList<NodeOption> nodeOptList;
  QList<NodeOption> linkOptList;
  QList<NodeOption> badLinkOptList;

  boost::shared_ptr<XmlDoc> options = XmlOps::parse(std::string("<valuemap>"
      "	<value key=\"pi\" descr=\"Pi value\">"
      " 	<double>3.141592</double>"
      " </value>"
      "	<value key=\"hello\" descr=\"Some bool\">"
      " 	<bool>false</bool>"
      " </value>"
      "</valuemap>"));

  root->addNode(node);
  root->addNode(link);
  root->addNode(badLink);

  node->setOptions(*options->first_node());

  GUI_CHECK_NO_THROW(node->getOptions(nodeOptList));
  GUI_CHECK_NO_THROW(link->getOptions(linkOptList));
  GUI_CHECK_THROW(badLink->getOptions(badLinkOptList), InvalidPath);

  // there were already 2 options in MyNode + 2 new options => 4 options
  QCOMPARE(nodeOptList.count(), 4);
  QCOMPARE(linkOptList.count(), 4);

  // lists must be equal
  QCOMPARE(nodeOptList, linkOptList);
}

///////////////////////////////////////////////////////////////////////////

void CNodeTest::test_createFromXml()
{
  CNode::Ptr node;
  boost::shared_ptr<XmlDoc> doc = XmlOps::parse(boost::filesystem::path("./tree.xml"));
  NRoot::Ptr root;
  NGroup::Ptr group;
  QList<NodeOption> optList;
  NodeOption option;
  boost::shared_ptr<XmlDoc> tree = XmlOps::parse(
      std::string("<CRoot name=\"Simulator\">"
                  "  <SomeComponent name=\"Flow\">" // comp. type does not exist
                  "   <valuemap>"
                  "    <value key=\"pi\" descr=\"Pi in a CGroup\">"
                  "     <double>3.1415920000000002</double>"
                  "    </value>"
                  "   </valuemap>"
                  "  </SomeComponent>"
                  "</CGroup>"));

  GUI_CHECK_NO_THROW(root = CNode::createFromXml(*doc->first_node())->convertTo<NRoot>());
  GUI_CHECK_THROW(CNode::createFromXml(*tree->first_node())->convertTo<NRoot>(), XmlError);

  GUI_CHECK_NO_THROW(node = boost::dynamic_pointer_cast<CNode>(root->root()->get_child("Flow")));
  GUI_CHECK_NO_THROW(group = node->convertTo<NGroup>());

  group->getOptions(optList);

  QCOMPARE(optList.count(), 1);

  option = optList.at(0);

  QCOMPARE(option.m_paramAdv, true);
  QCOMPARE(option.m_paramName, QString("pi"));
  QCOMPARE(option.m_paramDescr, QString("Pi in a CGroup"));
  QCOMPARE(option.m_paramValue, QString("3.1415920000000002"));
  QCOMPARE(option.m_paramType, OptionType::TYPE_DOUBLE);
}

///////////////////////////////////////////////////////////////////////////

void CNodeTest::test_addNode()
{
  NRoot::Ptr root(new NRoot("Root"));
  NLog::Ptr log(new NLog());
  QSignalSpy spy(root->getNotifier(), SIGNAL(childCountChanged()));

  GUI_CHECK_NO_THROW(root->addNode(log));

  QCOMPARE(spy.count(), 1);

  spy.clear();

  GUI_CHECK_THROW(root->addNode(log), ValueExists); // try to add the log again

  QCOMPARE(spy.count(), 0);
}
