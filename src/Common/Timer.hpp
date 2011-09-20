// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Timer_hpp
#define CF_Common_Timer_hpp

#include <boost/scoped_ptr.hpp>

#include "Common/CommonAPI.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

class Component;

/// Store accumulated timings in properties for readout
void store_timings(Component& root);

/// Print timing tree based on the existing properties
void print_timing_tree(Component& root, const bool print_untimed = false, const std::string& prefix="");

}
}

#ifdef CF_OS_LINUX

namespace CF {
namespace Common {

///////////////////////////////////////////////////////////////////////////////////////

/// Based on boost::timer and spirit/optimization/high_res_timer. Uses high resolution
class Common_API Timer
{
public:
  Timer();
  ~Timer();
  
  void restart();
  
  double elapsed() const;

private:
  /// Contains implementation details
  struct implementation;
  boost::scoped_ptr<implementation> m_implementation;
}; // timer

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

#else

#include <boost/timer.hpp>

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

typedef boost::timer Timer;

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

#endif

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Timer_hpp

class C;
