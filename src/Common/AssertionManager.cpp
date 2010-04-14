#include <sstream>
#include <iostream>

#include "Common/CommonAPI.hpp"
#include "Common/AssertionManager.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/ProcessInfo.hpp"

#include "Common/OSystem.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace CF::Common;

namespace CF {

////////////////////////////////////////////////////////////////////////////////

AssertionManager::AssertionManager() :
  DoAssertions    ( true ),
  AssertionDumps  ( true ),
  AssertionThrows ( false ) {}

////////////////////////////////////////////////////////////////////////////////

AssertionManager& AssertionManager::getInstance()
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
  if ( (!condition) && AssertionManager::getInstance().DoAssertions )
  {
    std::ostringstream out;
    out << "Assertion failed: [" << cond_str << "] ";

    if (desc)
      out << "'" << desc << "' ";

    out << "in " << file << ":" << line;

    if (func)
      out << " [function " << func << "]";

    if ( AssertionManager::getInstance().AssertionDumps )
      out << "\n" << OSystem::getInstance().getProcessInfo()->getBackTrace();

    if ( AssertionManager::getInstance().AssertionThrows )
    {
      throw FailedAssertion (FromHere(),out.str());
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
