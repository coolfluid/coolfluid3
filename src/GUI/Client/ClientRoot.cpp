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

CRoot::Ptr ClientRoot::getRoot()
{
  static bool rootCreated = false;
  static CRoot::Ptr root = CRoot::create(CLIENT_ROOT);
  static CLog::Ptr log(new CLog());
  static CBrowser::Ptr browser(new CBrowser());


  // if the function is called for the first time, we add the components
  if(!rootCreated)
  {
    root->add_component(log);
    root->add_component(browser);

    rootCreated = true;
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

    getRoot()->access_component(receiver)->call_signal( type, *nodeToProcess );
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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CLog::Ptr ClientRoot::getLog()
{
  return getRoot()->access_component< CLog >(CLIENT_LOG_PATH);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CBrowser::Ptr ClientRoot::getBrowser()
{
  return boost::dynamic_pointer_cast< CBrowser >(getRoot()->access_component(CLIENT_BROWSERS_PATH));
}
