#include <QtCore>
#include <string>

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
  QDomElement elt = signal.firstChildElement("Signal");

  if(elt.isNull())
    elt = signal.firstChildElement("Reply");

  if(!elt.isNull())
  {
    std::string type = elt.attribute("key").toStdString();
    std::string receiver = elt.attribute("receiver").toStdString();

    getRoot()->access_component(receiver)->call_signal(type, signal.toString().toStdString());
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
  //return boost::dynamic_pointer_cast< CLog >(getRoot()->access_component(CLIENT_LOG_PATH));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CBrowser::Ptr ClientRoot::getBrowser()
{
  return boost::dynamic_pointer_cast< CBrowser >(getRoot()->access_component(CLIENT_BROWSERS_PATH));
}
