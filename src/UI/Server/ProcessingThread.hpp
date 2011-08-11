// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Server_ProcessingThread_hpp
#define CF_GUI_Server_ProcessingThread_hpp

///////////////////////////////////////////////////////////////////////////////

#include <QThread>

#include "Common/Component.hpp"

///////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Server {

  class Notifier;

///////////////////////////////////////////////////////////////////////////////

  class ProcessingThread : public QThread
  {

  public:

    ProcessingThread(Common::SignalArgs & signal, const std::string & target,
                     Common::Component::Ptr receiver);

    void run();

    bool success() const { return m_success; }

    std::string message() const { return m_message; }

  private:

    Common::SignalArgs m_signal;

    std::string m_target;

    Common::Component::Ptr m_receiver;

    bool m_success;

    std::string m_message;

};

///////////////////////////////////////////////////////////////////////////////

} // Server
} // UI
} // CF

///////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Server_ProcessingThread_hpp
