#include <QtCore>
#include <string>
#include <cstring>

#include "Common/XmlHelpers.hpp"

#include "GUI/Network/ComponentNames.hpp"
#include "GUI/Network/SignalInfo.hpp"

#include "GUI/Client/ClientRoot.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;
using namespace CF::GUI::Network;

NRoot::Ptr ClientRoot::getRoot()
{
  static bool rootCreated = false;
  static NRoot::Ptr root(new NRoot(CLIENT_ROOT));

  static NLog::Ptr log(new NLog());
  static NBrowser::Ptr browser(new NBrowser());
  static NTree::Ptr tree(new NTree(root));

  // if the function is called for the first time, we add the components
  if(!rootCreated)
  {
    root->root()->add_component(log);
    root->root()->add_component(browser);
    root->root()->add_component(tree);
    rootCreated = true;

    tree->setRoot(root);
  }

  return root;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientRoot::processSignal(const QDomDocument & signal)
{
  boost::shared_ptr<XmlDoc> xmldoc = XmlOps::parse ( signal.toString().toStdString() );

  XmlNode& nodedoc = *XmlOps::goto_doc_node(*xmldoc.get());
  XmlNode * nodeToProcess = nodedoc.first_node(XmlParams::tag_node_frame());


  if(nodeToProcess != CFNULL)
  {
    XmlNode * tmpNode = nodeToProcess->next_sibling();

    if(tmpNode != CFNULL && std::strcmp(tmpNode->first_attribute("type")->value(), "reply") == 0)
      nodeToProcess = tmpNode;

    std::string type = nodeToProcess->first_attribute("target")->value();
    std::string receiver = nodeToProcess->first_attribute("receiver")->value();

    try
    {
      getRoot()->root()->access_component(receiver)->call_signal(type, *nodeToProcess);
    }
    catch(InvalidPath ip)
    {
      getLog()->addException(ip.what());
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientRoot::processSignalString(const QString & signal)
{
  QDomDocument doc;

  doc.setContent(signal);
  processSignal(doc);
}


