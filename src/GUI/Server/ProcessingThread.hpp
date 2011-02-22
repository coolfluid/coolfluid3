// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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
namespace GUI {
namespace Server {

  class Notifier;

///////////////////////////////////////////////////////////////////////////////

  class ProcessingThread : public QThread
  {

  public:

    ProcessingThread(Common::Signal::arg_t & signal, const std::string & target,
                     Common::Component::Ptr receiver);

    void run();

  private:

    Common::Signal::arg_t m_signal;

    std::string m_target;

    Common::Component::Ptr m_receiver;

};

///////////////////////////////////////////////////////////////////////////////

} // Server
} // GUI
} // CF

///////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Server_ProcessingThread_hpp
