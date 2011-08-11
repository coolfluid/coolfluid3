// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/MPI/CPEManager.hpp"

#include "Tools/Solver/Notifier.hpp"

using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace Solver {

//////////////////////////////////////////////////////////////////////////////

Notifier::Notifier( boost::shared_ptr<Common::Comm::CPEManager> manager )
  : m_manager(manager)
{
  cf_assert( is_not_null(manager) );

  m_observed_queue = m_manager->notification_queue();
}

//////////////////////////////////////////////////////////////////////////////

Notifier::~Notifier()
{

}

//////////////////////////////////////////////////////////////////////////////

void Notifier::listen_to_event(const std::string & name, bool notifyOnce)
{
  m_observed_queue->add_notifier(name, &Notifier::new_event, this);

  if(notifyOnce)
    m_once_notifying_events[name] = false;
}

//////////////////////////////////////////////////////////////////////////////

void Notifier::begin_notify()
{
  std::map<std::string, bool>::iterator it = m_once_notifying_events.begin();

  for( ; it != m_once_notifying_events.end() ; it++)
    it->second = false;
}

//////////////////////////////////////////////////////////////////////////////

void Notifier::new_event(const std::string & name, const URI & raiserPath)
{
  std::map<std::string, bool>::iterator it = m_once_notifying_events.find(name);

  if( it == m_once_notifying_events.end() || !it->second )
  {
//    event_occured(name, raiserPath);

    /// @todo Ugly!!! should use a boost::signal2
    m_manager->new_event(name, raiserPath);

    if(it != m_once_notifying_events.end())
      it->second = true;
  }
}

//////////////////////////////////////////////////////////////////////////////

} // Solver
} // Tools
} // CF
