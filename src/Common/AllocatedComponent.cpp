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

TimedActionImpl::TimedActionImpl(CAction& action) : m_action(action)
{
}

void TimedActionImpl::execute()
{
  std::cout << "Running timed action " << m_action.uri().string() << std::endl;
  m_action.execute();
}


////////////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF
