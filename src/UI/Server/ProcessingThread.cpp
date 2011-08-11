// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Component.hpp"
#include "Common/XML/SignalFrame.hpp"

#include "Common/Log.hpp"

#include "UI/Server/ServerRoot.hpp"

#include "UI/Server/ProcessingThread.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Server {

/////////////////////////////////////////////////////////////////////////////

ProcessingThread::ProcessingThread(SignalArgs & signal, const std::string & target,
                                   Component::Ptr receiver)
  : m_signal(signal),
    m_target(target),
    m_receiver(receiver),
    m_success(false)
{
  cf_assert( is_not_null(receiver) );
}

/////////////////////////////////////////////////////////////////////////////

void ProcessingThread::run()
{
  m_success = false;
  try
  {
    m_receiver->call_signal( m_target, m_signal );
    m_success = true;
  }
  catch(Exception & cfe)
  {
    m_message = cfe.what();
  }
  catch(std::exception & stde)
  {
    m_message = stde.what();
  }
  catch(...)
  {
    m_message = "Unknown exception thrown during execution of action [" +
        m_target + "] on component [" + m_receiver->uri().path() + "].";
  }
}

/////////////////////////////////////////////////////////////////////////////

} // Server
} // UI
} // CF
