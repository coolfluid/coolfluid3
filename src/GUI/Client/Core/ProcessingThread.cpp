// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Component.hpp"
#include "Common/Log.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"

#include "GUI/Client/Core/ProcessingThread.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////

ProcessingThread::ProcessingThread(boost::shared_ptr<XmlDoc> node)
  : m_node(node)
{
  cf_assert(node.get() != nullptr);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ProcessingThread::run()
{
  XmlNode& nodedoc = *XmlOps::goto_doc_node(*m_node.get());
  XmlNode * nodeToProcess = nodedoc.first_node(XmlParams::tag_node_frame());

  if(nodeToProcess != nullptr)
  {
    XmlNode * tmpNode = nodeToProcess->next_sibling(XmlParams::tag_node_frame());

    // check this is a reply
    if(tmpNode != nullptr && std::strcmp(tmpNode->first_attribute("type")->value(), "reply") == 0)
      nodeToProcess = tmpNode;

    std::string type = nodeToProcess->first_attribute("target")->value();
    std::string receiver = nodeToProcess->first_attribute("receiver")->value();

    try
    {
      NRoot::Ptr root = ClientRoot::instance().root();
      CRoot::Ptr realRoot = root->root();

      if(realRoot->full_path().path() == receiver)
        root->call_signal(type, *nodeToProcess);
      else
        realRoot->access_component(receiver)->call_signal(type, *nodeToProcess);
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
