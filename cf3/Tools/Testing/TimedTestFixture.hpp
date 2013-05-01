// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Tools_Tests_Timer_hpp
#define cf3_Tools_Tests_Timer_hpp

#include <iostream>

#include <boost/test/framework.hpp>
#include <boost/test/test_observer.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/timer.hpp>

#include "common/Timer.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {
namespace Testing {

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
  };

  /// Stop timing when a test ends
  void test_unit_finish( boost::unit_test::test_unit const& unit ) {
    // TODO: Provide more generic support for output in CDash format
    std::cout << "<DartMeasurement name=\"" << unit.p_name.get() << " time\" type=\"numeric/double\">" << m_timer.elapsed() << "</DartMeasurement>" << std::endl;
  }
private:
  common::Timer m_timer;
};

////////////////////////////////////////////////////////////////////////////////

} // Testing
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Tools_Tests_TimedTestFixture_hpp
