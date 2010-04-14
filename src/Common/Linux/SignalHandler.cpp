#include <cstdio>     // for printf()
#include <cstdlib>    // for free() and abort()
#include <csignal>    // POSIX signal(), SIGFPE and SIGSEGV
#include <fenv.h>     // floating Common access
#include <sstream>    // streamstring

#include "Common/BasicExceptions.hh"
#include "Common/Linux/ProcessInfo.hh"
#include "Common/Linux/SignalHandler.hh"

//////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace CF {
  namespace Common {
    namespace Linux {

//////////////////////////////////////////////////////////////////////////////

SignalHandler::SignalHandler()
{
}

//////////////////////////////////////////////////////////////////////////////

SignalHandler::~SignalHandler()
{
}

//////////////////////////////////////////////////////////////////////////////

void SignalHandler::registSignalHandlers()
{
  // register handler functions for the signals
  signal(SIGFPE,    (sighandler_t) Linux::SignalHandler::handleSIGFPE);
  signal(SIGSEGV,   (sighandler_t) Linux::SignalHandler::handleSIGSEGV);

  // enable the exceptions that will raise the SIGFPE signal
  feenableexcept ( FE_DIVBYZERO );
  feenableexcept ( FE_INVALID   );
  feenableexcept ( FE_OVERFLOW  );
  feenableexcept ( FE_UNDERFLOW );
}

//////////////////////////////////////////////////////////////////////////////

int SignalHandler::handleSIGFPE (int signal)
{
  printf("\nreceived signal SIGFPE [%d] - 'Floating Point Exception'\n",signal);
  static std::string dump = Linux::ProcessInfo::dumpBacktrace();
  printf( "%s\n", dump.c_str() );
  throw Common::FloatingPointError (FromHere(), "Some floating point operation has given an invalid result");
}

//////////////////////////////////////////////////////////////////////////////

int SignalHandler::handleSIGSEGV(int signal)
{
  printf("\nreceived signal SIGSEGV [%d] - 'Segmentation violation'\n",signal);
  static std::string dump = Linux::ProcessInfo::dumpBacktrace();
  printf( "%s\n", dump.c_str() );
  abort();
}

//////////////////////////////////////////////////////////////////////////////

    } // Linux
  } // namespace Common
} // namespace CF

//////////////////////////////////////////////////////////////////////////////
