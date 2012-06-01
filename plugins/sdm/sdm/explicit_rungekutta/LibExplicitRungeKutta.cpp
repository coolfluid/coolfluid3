// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "sdm/explicit_rungekutta/LibExplicitRungeKutta.hpp"

namespace cf3 {
namespace sdm {
namespace explicit_rungekutta {
using namespace common;

cf3::common::RegistLibrary<LibExplicitRungeKutta> libExplicitRungeKutta;

////////////////////////////////////////////////////////////////////////////////

void LibExplicitRungeKutta::initiate()
{
  if(m_is_initiated)
    return;

  Handle<Component> lib = Core::instance().libraries().get_child("cf3.sdm.explicit_rungekutta");
  cf3_assert(lib);

  m_is_initiated = true;
}

////////////////////////////////////////////////////////////////////////////////

} // explicit_rungekutta
} // sdm
} // cf3
