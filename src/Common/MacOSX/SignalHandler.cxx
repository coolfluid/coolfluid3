#include "Common/BasicExceptions.hh"
#include "Common/CommonAPI.hh"
#include "Common/MacOSX/SignalHandler.hh"
#include "Common/MacOSX/ProcessInfo.hh"

#include <cstdio>     // for printf()
#include <cstdlib>    // for free() and abort()
#include <csignal>    // POSIX signal(), SIGFPE and SIGSEGV
#include <fenv.h>     // floating environment access
#include <sstream>    // streamstring

//////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace CF {
  namespace Common {
    namespace MacOSX {

//////////////////////////////////////////////////////////////////////////////

/// Following functions are required since they are not available for Mac OSX
/// This only works for intel architecture
/// http://www-personal.umich.edu/~williams/archive/computation/fe-handling-example.c

static int
fegetexcept (void)
{
  static fenv_t fenv;

  return fegetenv (&fenv) ? -1 : (fenv.__control & FE_ALL_EXCEPT);
}

static int
feenableexcept (unsigned int excepts)
{
  static fenv_t fenv;
  unsigned int new_excepts = excepts & FE_ALL_EXCEPT,
               old_excepts;  // previous masks

  if ( fegetenv (&fenv) ) return -1;
  old_excepts = fenv.__control & FE_ALL_EXCEPT;

  // unmask
  fenv.__control &= ~new_excepts;
  fenv.__mxcsr   &= ~(new_excepts << 7);

  return ( fesetenv (&fenv) ? -1 : old_excepts );
}

static int
fedisableexcept (unsigned int excepts)
{
  static fenv_t fenv;
  unsigned int new_excepts = excepts & FE_ALL_EXCEPT,
               old_excepts;  // all previous masks

  if ( fegetenv (&fenv) ) return -1;
  old_excepts = fenv.__control & FE_ALL_EXCEPT;

  // mask
  fenv.__control |= new_excepts;
  fenv.__mxcsr   |= new_excepts << 7;

  return ( fesetenv (&fenv) ? -1 : old_excepts );
}

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
  signal(SIGFPE,    (sighandler_t) MacOSX::SignalHandler::handleSIGFPE);
  signal(SIGSEGV,   (sighandler_t) MacOSX::SignalHandler::handleSIGSEGV);

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
  static std::string dump = MacOSX::ProcessInfo::dumpBackTrace();
  printf( "%s\n", dump.c_str() );
  throw Common::FloatingPointError (FromHere(), "Some floating point operation has given an invalid result");
}

//////////////////////////////////////////////////////////////////////////////

int SignalHandler::handleSIGSEGV(int signal)
{
  printf("\nreceived signal SIGSEGV [%d] - 'Segmentation violation'\n",signal);
  static std::string dump = MacOSX::ProcessInfo::dumpBackTrace();
  printf( "%s\n", dump.c_str() );
  abort();
}


//////////////////////////////////////////////////////////////////////////////

    } // MacOSX
  } // Common
} // CF

//////////////////////////////////////////////////////////////////////////////
