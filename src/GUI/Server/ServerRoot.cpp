#include <QtCore>

#include "GUI/Network/ComponentNames.hpp"

#include "Common/CRoot.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Server/ServerRoot.hpp"
#include "GUI/Server/CSimulator.hpp"

using namespace CF::Common;
using namespace CF::GUI::Server;

CRoot::Ptr & ServerRoot::getRoot()
{
  static bool rootCreated = false;
  static CRoot::Ptr root = CRoot::create(SERVER_ROOT);
  static CCore::Ptr core(new CCore());
  static CSimulator::Ptr simulator(new CSimulator());

  if(!rootCreated)
  {
    root->add_component(core);
    root->add_component(simulator);

    rootCreated = true;

    simulator->createSimulator();
  }

  return root;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerRoot::processSignal(const std::string & target,
                               const std::string & receiver,
                               const std::string & clientid,
                               XmlNode & node)
{
  getRoot()->access_component(receiver)->call_signal(target, *node.first_node() );

  getCore()->sendSignal(*node.document());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CCore::Ptr ServerRoot::getCore()
{
  return boost::dynamic_pointer_cast<CCore>(getRoot()->access_component(SERVER_CORE_PATH));
}
