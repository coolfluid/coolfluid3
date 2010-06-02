#ifndef CF_Tools_Tests_Timer_hpp
#define CF_Tools_Tests_Timer_hpp

#include <boost/timer.hpp>

#include "Common/Log.hpp"

using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace Tests {

////////////////////////////////////////////////////////////////////////////////

class Timer {
public:
  void restart_timing() { m_timer.restart(); }
  /// Write the current value of the timer as a Dart measurement with the supplied name
  void measure_time(const std::string& measurementName) {
    // TODO: Provide more generic support for output in CDash format
    CFinfo << "<DartMeasurement name=\"" << measurementName << " time\" type=\"numeric/double\">" << m_timer.elapsed() << "</DartMeasurement>\n";
  }
private:
  boost::timer m_timer;
};

////////////////////////////////////////////////////////////////////////////////

} // Tests
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_Tests_TimedTestFixture_hpp
