// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "rapidxml/rapidxml.hpp"

#include "Common/Log.hpp"

#include "GUI/Client/Core/NBrowser.hpp"
#include "GUI/Client/Core/NetworkThread.hpp"
#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/Core/NRoot.hpp"
#include "GUI/Client/Core/NTree.hpp"
#include "GUI/Client/Core/ThreadManager.hpp"

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::GUI::Network;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////

ClientRoot & ClientRoot::instance()
{
  static ClientRoot cr;
  return cr;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ClientRoot::ClientRoot() :
    m_root(new NRoot(CLIENT_ROOT))
{
  CRoot::Ptr realRoot = m_root->root();

  NLog::Ptr log(new NLog());
  NBrowser::Ptr browser(new NBrowser());
  NTree::Ptr tree(new NTree(m_root));

  // add components to the root
  realRoot->add_component(log);
  realRoot->add_component(browser);
  realRoot->add_component(tree);

  // mark all components as basic
  m_root->mark_basic();
  log->mark_basic();
  browser->mark_basic();
  tree->mark_basic();

  // set the root as model root
  tree->setRoot(m_root);

  ThreadManager::instance().network().newSignal.connect( boost::bind(&ClientRoot::newSignal, this, _1));
}

////////////////////////////////////////////////////////////////////////////

void ClientRoot::newSignal(Common::XML::XmlDoc::Ptr doc)
{
  const char * tag = Protocol::Tags::node_frame();
  XmlNode nodedoc = Protocol::goto_doc_node(*doc.get());
  rapidxml::xml_node<char>* nodeToProcess = nodedoc.content->first_node(tag);

  if(nodeToProcess != nullptr)
  {
    rapidxml::xml_node<>* tmpNode = nodeToProcess->next_sibling( tag );

    // check this is a reply
    if(tmpNode != nullptr && std::strcmp(tmpNode->first_attribute("type")->value(), "reply") == 0)
      nodeToProcess = tmpNode;

    std::string type = nodeToProcess->first_attribute("target")->value();
    std::string receiver = nodeToProcess->first_attribute("receiver")->value();

    try
    {
      CRoot::Ptr realRoot = root()->root();
      SignalFrame frame(nodeToProcess);

      if(realRoot->full_path().path() == URI(receiver).path())
        root()->call_signal(type, frame);
      else
        realRoot->retrieve_component(receiver)->call_signal(type, frame);
    }
    catch(CF::Common::Exception & cfe)
    {
      NLog::globalLog()->addException(cfe.what());
    }
    catch(std::exception & stde)
    {
      NLog::globalLog()->addException(stde.what());
    }
    catch(...)
    {
      CFerror << "Unknown exception thrown during execution of action [" << type
          << "] on component " << " [" << receiver << "]." << CFendl;
    }

  }

}

////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF
