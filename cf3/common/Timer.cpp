// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "common/BasicExceptions.hpp"
#include "common/Component.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Timer.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

///////////////////////////////////////////////////////////////////////////////////////

#ifdef CF3_OS_LINUX

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
    throw common::NotSupported(FromHere(), "Couldn't initialize high resolution timer");
}

double Timer::elapsed() const
{
  timespec now;
  if (-1 == clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now))
    throw common::NotSupported(FromHere(), "Couldn't get current time from high resolution timer");

  if (now.tv_sec == m_implementation->start_time.tv_sec)
    return double(now.tv_nsec - m_implementation->start_time.tv_nsec) * 1e-9;

  return double(now.tv_sec - m_implementation->start_time.tv_sec) + 
    (double(now.tv_nsec - m_implementation->start_time.tv_nsec) * 1e-9);
}

#endif

/////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////////
