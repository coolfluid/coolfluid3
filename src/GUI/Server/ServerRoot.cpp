#include "GUI/Network/ComponentNames.hpp"

#include "Common/CRoot.hpp"
#include "Common/XmlHelpers.hpp"

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
  boost::shared_ptr<XmlDoc> xmldoc = XmlOps::parse ( signal.toString().toStdString() );

  XmlNode& nodedoc = *XmlOps::goto_doc_node(*xmldoc.get());

  std::string type = nodedoc.first_node()->first_attribute("target")->value();
  std::string receiver = nodedoc.first_node()->first_attribute("receiver")->value();

  getRoot()->access_component(receiver)->call_signal( type, *nodedoc.first_node() );

  std::string str;
  XmlOps::xml_to_string(*xmldoc.get(), str);
  //CFinfo << "Sending back " <<  str << CFendl;

  getCore()->sendSignal(*xmldoc.get());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CCore::Ptr ServerRoot::getCore()
{
  return boost::dynamic_pointer_cast<CCore>(getRoot()->access_component(SERVER_CORE_PATH));
}
