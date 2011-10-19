// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Server_ProcessingThread_hpp
#define cf3_GUI_Server_ProcessingThread_hpp

///////////////////////////////////////////////////////////////////////////////

#include <QThread>

#include "common/Component.hpp"

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Server {

  class Notifier;

///////////////////////////////////////////////////////////////////////////////

  class ProcessingThread : public QThread
  {

  public:

    ProcessingThread(common::SignalArgs & signal, const std::string & target,
                     common::Component::Ptr receiver);

    void run();

    bool success() const { return m_success; }

    std::string message() const { return m_message; }

  private:

    common::SignalArgs m_signal;

    std::string m_target;

    common::Component::Ptr m_receiver;

    bool m_success;

    std::string m_message;

};

///////////////////////////////////////////////////////////////////////////////

} // Server
} // UI
} // cf3

///////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Server_ProcessingThread_hpp
