// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"
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

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////////
