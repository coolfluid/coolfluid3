// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_TimedComponent_hpp
#define cf3_common_TimedComponent_hpp

#include "common/CommonAPI.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

class Component;

/// Pure virtual interface for components that store timings
class Common_API TimedComponent
{
public:
  virtual ~TimedComponent() {}

  /// Copy the stored timings from internal storage to visible properties.
  /// This avoids having expensive property updates on each timing
  virtual void store_timings() = 0;
};

/// Store accumulated timings in properties for readout
void store_timings(Component& root);

/// Print timing tree based on the existing properties
void print_timing_tree(Component& root, const bool print_untimed = false, const std::string& prefix="");

}
}

#endif // cf3_common_TimedComponent_hpp
