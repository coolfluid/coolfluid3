// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Component.hpp"

#include "Common/Log.hpp"

#include "GUI/Server/ServerRoot.hpp"

#include "GUI/Server/ProcessingThread.hpp"

using namespace CF::Common;
using namespace CF::GUI::Server;

ProcessingThread::ProcessingThread(XmlNode & node, const std::string & target,
                                   Component::Ptr receiver)
  : m_node(node),
    m_target(target),
    m_receiver(receiver)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ProcessingThread::run()
{
  try
  {
    m_receiver->call_signal(m_target, *m_node.first_node() );
  }
  catch(CF::Common::Exception & cfe)
  {
    CFerror << cfe.what() << CFendl;
  }
  catch(std::exception & stde)
  {
    CFerror << stde.what() << CFendl;
  }
  catch(...)
  {
    CFerror << "Unknown exception thrown..." << CFendl;
  }
}

XmlNode & ProcessingThread::getNode() const
{
  return m_node;
}
