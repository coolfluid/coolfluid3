// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "Common/BasicExceptions.hpp"
#include "Common/Component.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/Timer.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

///////////////////////////////////////////////////////////////////////////////////////

#ifdef CF_OS_LINUX

extern "C"
{
  #include <time.h>
}

struct Timer::implementation
{
  timespec start_time;
};

Timer::Timer()
{
  m_implementation.reset(new implementation());
  restart();
}

Timer::~Timer()
{
}

void Timer::restart()
{
  if (-1 == clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &m_implementation->start_time))
    throw Common::NotSupported(FromHere(), "Couldn't initialize high resolution timer");
}

double Timer::elapsed() const
{
  timespec now;
  if (-1 == clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now))
    throw Common::NotSupported(FromHere(), "Couldn't get current time from high resolution timer");

  if (now.tv_sec == m_implementation->start_time.tv_sec)
    return double(now.tv_nsec - m_implementation->start_time.tv_nsec) * 1e-9;

  return double(now.tv_sec - m_implementation->start_time.tv_sec) + 
    (double(now.tv_nsec - m_implementation->start_time.tv_nsec) * 1e-9);
}

#endif

/////////////////////////////////////////////////////////////////////////////////////

void store_timings(Component& root)
{
  BOOST_FOREACH(Component& component, find_components_recursively(root))
  {
    TimedComponent* timed_comp = dynamic_cast<TimedComponent*>(component.as_ptr<Component>().get());
    if(is_not_null(timed_comp))
    {
      timed_comp->store_timings();
    }
  }
}

void print_timing_tree(CF::Common::Component& root, const bool print_untimed, const std::string& prefix)
{
  if(root.properties().check("timer_mean"))
  {
    std::cout << prefix << root.name() << ": mean: " << root.properties().value_str("timer_mean") << ", max: " << root.properties().value_str("timer_maximum") << ", min: " << root.properties().value_str("timer_minimum") << std::endl;
  }
  else if(print_untimed)
  {
    std::cout << prefix << root.name() << ": no timing info" << std::endl;
  }
  BOOST_FOREACH(Component& component, root)
  {
    print_timing_tree(component, print_untimed, prefix + "  ");
  }
}


/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////////
