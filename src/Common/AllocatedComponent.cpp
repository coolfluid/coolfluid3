// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "Common/AllocatedComponent.hpp"
#include "Common/CAction.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////////////////

TimedActionImpl::TimedActionImpl(CAction& action) : m_timed_component(action)
{
}

void TimedActionImpl::start_timing()
{
  std::cout << "Starting timer on " << m_timed_component.uri().string() << std::endl;
}

void TimedActionImpl::stop_timing()
{
  std::cout << "Stopping timer on " << m_timed_component.uri().string() << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF
