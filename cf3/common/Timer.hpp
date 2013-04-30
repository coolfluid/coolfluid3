// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Timer_hpp
#define cf3_common_Timer_hpp

#include "common/CommonAPI.hpp"

/////////////////////////////////////////////////////////////////////////////////////

#ifdef CF3_OS_LINUX

#include <boost/scoped_ptr.hpp>

namespace cf3 {
namespace common {

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

} // common
} // cf3

#else

#include <boost/timer.hpp>

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////////////

typedef boost::timer Timer;

/////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

#endif

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Timer_hpp
