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

#include "Common/PE/Comm.hpp"

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
  if(prefix.empty()) // Top-level recursion
  {
    store_timings(root);
    if(PE::Comm::instance().rank() == 0)
      std::cout << "<DartMeasurement name=\"Timings\" type=\"text/plain\"><![CDATA[<html><body><pre>\n";
  }

  if(!root.properties().check("timer_mean") && print_untimed && PE::Comm::instance().rank() == 0)
  {
    std::cout << prefix << root.name() << ": no timing info\n";
  }


  if(root.properties().check("timer_mean"))
  {
    const Real local_mean = root.properties().value<Real>("timer_mean");
    const Real local_min = root.properties().value<Real>("timer_minimum");
    const Real local_max = root.properties().value<Real>("timer_maximum");

    // Only bother with global averages if we have more than 1 process
    if(PE::Comm::instance().is_active() && PE::Comm::instance().size() > 1)
    {
      // Average, minimum and maximum of each statistic over all CPUs
      Real mean_mean, mean_min, mean_max, max_mean, max_min, max_max, min_mean, min_min, min_max;
      PE::Comm::instance().all_reduce(PE::plus(), &local_mean, 1, &mean_mean);
      PE::Comm::instance().all_reduce(PE::min(), &local_mean, 1, &mean_min);
      PE::Comm::instance().all_reduce(PE::max(), &local_mean, 1, &mean_max);

      PE::Comm::instance().all_reduce(PE::plus(), &local_min, 1, &min_mean);
      PE::Comm::instance().all_reduce(PE::min(), &local_min, 1, &min_min);
      PE::Comm::instance().all_reduce(PE::max(), &local_min, 1, &min_max);

      PE::Comm::instance().all_reduce(PE::plus(), &local_max, 1, &max_mean);
      PE::Comm::instance().all_reduce(PE::min(), &local_max, 1, &max_min);
      PE::Comm::instance().all_reduce(PE::max(), &local_max, 1, &max_max);

      const Real nb_procs = static_cast<Real>(PE::Comm::instance().size());
      mean_mean /= nb_procs; min_mean /= nb_procs; max_mean /= nb_procs;

      if(PE::Comm::instance().rank() == 0)
      {
        if(prefix.empty()) std::cout << "Timings in seconds, with [min, mean, max] over CPUs\n";
        std::cout << prefix << root.name()
          << ": mean: "  << mean_mean
          << ", min: " << min_min
          << ", max: " << max_max << "\n";
      }
    }
    else
    {
      std::cout << prefix << root.name() << ": mean: " << local_mean << ", max: " << local_max << ", min: " << local_min << "\n";
    }
  }

  BOOST_FOREACH(Component& component, root)
  {
    print_timing_tree(component, print_untimed, prefix + "  ");
  }

  if(prefix.empty() && PE::Comm::instance().rank() == 0)
    std::cout << "</pre></body></html>]]></DartMeasurement>" << std::endl;
}


/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////////
