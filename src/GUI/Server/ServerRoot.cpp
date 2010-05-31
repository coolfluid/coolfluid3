#include "GUI/Network/ComponentNames.hpp"

#include "Common/CRoot.hpp"

#include "GUI/Server/ServerRoot.hpp"

using namespace CF::Common;
using namespace CF::GUI::Server;

CRoot::Ptr & ServerRoot::getRoot()
{
  static bool rootCreated = false;
  static CRoot::Ptr root = CRoot::create(SERVER_ROOT);
  static CCore::Ptr core(new CCore());

  if(!rootCreated)
  {
    root->add_component(core);

    rootCreated = true;
  }

  return root;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerRoot::processSignal(const QDomDocument & signal)
{
 QDomElement elt = signal.firstChildElement("Signal");

 if(!elt.isNull())
 {
   std::string type = elt.attribute("key").toStdString();
   std::string receiver = elt.attribute("receiver").toStdString();

   getRoot()->access_component(receiver)->call_signal(type, signal.toString().toStdString());
 }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CCore::Ptr ServerRoot::getCore()
{
  return boost::dynamic_pointer_cast<CCore>(getRoot()->access_component(SERVER_CORE_PATH));
}
