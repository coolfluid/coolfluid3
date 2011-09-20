// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "Common/Component.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/TimedComponent.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

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

/////////////////////////////////////////////////////////////////////////////////////

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
