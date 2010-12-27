// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Tools_Tests_Timer_hpp
#define CF_Tools_Tests_Timer_hpp

#include <boost/test/framework.hpp>
#include <boost/test/test_observer.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/timer.hpp>

#include <boost/mpi/timer.hpp>

#include "Common/BasicExceptions.hpp"
#include "Common/MPI/PE.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace Testing {

////////////////////////////////////////////////////////////////////////////////

#ifdef CF_OS_LINUX

extern "C"
{
  #include <time.h>
}

/// Based on boost::timer and spirit/optimization/high_res_timer. Uses high resolution
class Timer
{
public:
  Timer()
  { 
    restart();
  }
  
  void restart()
  {
    if (-1 == clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &m_start_time))
      throw Common::NotSupported(FromHere(), "Couldn't initialize high resolution timer");
  }
  
  double elapsed() const
  {
    timespec now;
    if (-1 == clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now))
      throw Common::NotSupported(FromHere(), "Couldn't get current time from high resolution timer");

    if (now.tv_sec == m_start_time.tv_sec)
      return double(now.tv_nsec - m_start_time.tv_nsec) * 1e-9;

    return double(now.tv_sec - m_start_time.tv_sec) + 
      (double(now.tv_nsec - m_start_time.tv_nsec) * 1e-9);
  }

private:
  timespec m_start_time;
}; // timer

#else

typedef boost::timer Timer;

#endif

////////////////////////////////////////////////////////////////////////////////

/// Any test using this fixture (or a derivative) will be timed
class TimedTestFixture {
public:

  TimedTestFixture() {
    test_unit_start(boost::unit_test::framework::current_test_case());
  }

  ~TimedTestFixture() {
    test_unit_finish(boost::unit_test::framework::current_test_case());
  }

  /// Start timing when a test starts
  void test_unit_start( boost::unit_test::test_unit const& unit)
  {
    restart_timer();
  }
  
  void restart_timer()
  {
    m_timer.restart();
    if(Common::mpi::PE::instance().is_init())
      m_mpi_timer.restart();
  };

  /// Stop timing when a test ends
  void test_unit_finish( boost::unit_test::test_unit const& unit ) {
    if(Common::mpi::PE::instance().rank() > 0)
      return;
    // TODO: Provide more generic support for output in CDash format
    std::cout << "<DartMeasurement name=\"" << unit.p_name.get() << " time\" type=\"numeric/double\">" << (Common::mpi::PE::instance().is_init() ? m_mpi_timer.elapsed() : m_timer.elapsed()) << "</DartMeasurement>" << std::endl;
  }
private:
  Timer m_timer;
  boost::mpi::timer m_mpi_timer;
};

////////////////////////////////////////////////////////////////////////////////

} // Testing
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_Tests_TimedTestFixture_hpp
