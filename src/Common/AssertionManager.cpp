#include <sstream>
#include <iostream>

#include "Common/CommonAPI.hpp"
#include "Common/AssertionManager.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/OSystemLayer.hpp"

#include "Common/OSystem.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace CF::Common;

namespace CF {

////////////////////////////////////////////////////////////////////////////////

AssertionManager::AssertionManager() :
  DoAssertions    ( true ),
  AssertionDumps  ( false ),
  AssertionThrows ( false ) {}

////////////////////////////////////////////////////////////////////////////////

AssertionManager& AssertionManager::instance()
{
  static AssertionManager assertion_manager;
  return assertion_manager;
}

////////////////////////////////////////////////////////////////////////////////

void AssertionManager::do_assert ( bool condition,
                                   const char * cond_str,
                                   const char * file,
                                   int line,
                                   const char * func,
                                   const char * desc )
{
  if ( (!condition) && AssertionManager::instance().DoAssertions )
  {

    CodeLocation code_position (file,line,func);

    std::ostringstream out;
    out << "Assertion failed: [" << cond_str << "] ";

    if (desc)
      out << "'" << desc << "' ";

    out << "at " << code_position.str();

    if ( AssertionManager::instance().AssertionDumps )
      out << "\n" << OSystem::instance().OSystemLayer()->getBackTrace();

    if ( AssertionManager::instance().AssertionThrows )
    {
      throw FailedAssertion ( code_position, out.str());
    }
    else
    {
      std::cerr << out << std::endl;
      std::cerr << "aborting..." << std::endl;
      cerr.flush ();
      abort ();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace CF
