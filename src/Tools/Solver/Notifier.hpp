// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Tools_Solver_Notifier_hpp
#define CF_Tools_Solver_Notifier_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/signals2.hpp>

#include "Common/NotificationQueue.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Common { namespace PE { class CPEManager; } }

namespace Tools {
namespace Solver {

//////////////////////////////////////////////////////////////////////////////

  class Notifier
  {
  public:

    Notifier( boost::shared_ptr<Common::PE::CPEManager> manager );

    ~Notifier();

    void listen_to_event(const std::string & name, bool notifyOnce);

    void begin_notify();

    void new_event(const std::string & name, const CF::Common::URI & raiserPath);

    boost::signals2::signal< void (const std::string &, const Common::URI &) > event_occured;

//  signals:

//    void eventOccured(const std::string & name, const CF::Common::URI & raiserPath);

  private:

    Common::NotificationQueue * m_observed_queue;

    std::map<std::string, bool> m_once_notifying_events;

    boost::shared_ptr<Common::PE::CPEManager> m_manager;

  }; // class Notifier

  //////////////////////////////////////////////////////////////////////////////

} // Solver
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_Solver_Notifier_hpp
