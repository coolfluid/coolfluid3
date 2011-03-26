// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Timer_hpp
#define CF_Common_Timer_hpp

#include <boost/scoped_ptr.hpp>

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

///////////////////////////////////////////////////////////////////////////////////////

#ifdef CF_OS_LINUX

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

#else

#include <boost/timer.hpp>

typedef boost::timer Timer;

#endif

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Timer_hpp
