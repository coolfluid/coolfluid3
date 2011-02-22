// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "rapidxml/rapidxml.hpp"

#include "Common/Component.hpp"
#include "Common/Log.hpp"

#include "Common/XML/Protocol.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"

#include "GUI/Client/Core/ProcessingThread.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::GUI::ClientCore;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////

  ProcessingThread::ProcessingThread(XmlDoc::Ptr node)
  : m_node(node)
{
  cf_assert(node.get() != nullptr);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ProcessingThread::run()
{
  const char * tag = Protocol::Tags::node_frame();
  XmlNode nodedoc = Protocol::goto_doc_node(*m_node.get());
  rapidxml::xml_node<>* nodeToProcess = nodedoc.content->first_node(tag);

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
      NRoot::Ptr root = ClientRoot::instance().root();
      CRoot::Ptr realRoot = root->root();
      SignalFrame frame(nodeToProcess);

      if(realRoot->full_path().path() == receiver)
        root->call_signal(type, frame);
      else
        realRoot->access_component(receiver)->call_signal(type, frame);
    }
    catch(CF::Common::Exception & cfe)
    {
      ClientRoot::instance().log()->addException(cfe.what());
    }
    catch(std::exception & stde)
    {
      ClientRoot::instance().log()->addException(stde.what());
    }
    catch(...)
    {
      CFerror << "Unknown exception thrown during execution of action [" << type
          << "] on component " << " [" << receiver << "]." << CFendl;
    }

  }

//    m_receiver->call_signal(m_target, *m_node.first_node() );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

XmlNode & ProcessingThread::getNode() const
{
  return *m_node.get();
//  return m_node;
}

////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF
