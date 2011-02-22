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
using namespace CF::Common::XML;
using namespace CF::GUI::Server;

ProcessingThread::ProcessingThread(Signal::arg_t & signal, const std::string & target,
                                   Component::Ptr receiver)
  : m_signal(signal),
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
    m_receiver->call_signal( m_target, m_signal );
  }
  catch(Exception & cfe)
  {
    ServerRoot::core()->sendException(cfe.what());
  }
  catch(std::exception & stde)
  {
    ServerRoot::core()->sendException(stde.what());
  }
  catch(...)
  {
    CFerror << "Unknown exception thrown during execution of action [" << m_target
        << "] on component " << " [" << m_receiver->full_path().path()
        << "]." << CFendl;
  }
}

