// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cstdio>        // for printf()
#include <cstdlib>       // for free() and abort()
#include <csignal>       // POSIX signal(), SIGFPE and SIGSEGV
#include <fenv.h>        // floating environment access
#include <sstream>       // streamstring
#include <execinfo.h>    // for backtrace() from glibc
#include <sys/types.h>   // for getting the PID of the process

#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/task.h>

#include "Common/BasicExceptions.hpp"
#include "Common/CommonAPI.hpp"
#include "Common/MacOSX/OSystemLayer.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace CF {
  namespace Common {
    namespace MacOSX {

////////////////////////////////////////////////////////////////////////////////

OSystemLayer::OSystemLayer()
{
}

////////////////////////////////////////////////////////////////////////////////

OSystemLayer::~OSystemLayer()
{
}

////////////////////////////////////////////////////////////////////////////////

std::string OSystemLayer::back_trace () const
{
  printf ("getBackTrace ...\n");
  return dumpBacktrace();
}

////////////////////////////////////////////////////////////////////////////////

std::string OSystemLayer::dumpBacktrace ()
{
  #define BUFFER_SIZE 500

  static int i = 0;
  ++i;

  printf ("dumping %d backtrace ...\n", i);

  std::ostringstream oss;
  int j, nptrs;
  void *buffer[BUFFER_SIZE];
  char **strings;

  nptrs = backtrace(buffer, BUFFER_SIZE);
  oss << "\nbacktrace() returned " << nptrs << " addresses\n";

  strings = backtrace_symbols(buffer, nptrs);
  if (strings == NULL)
    oss << "\nno backtrace_symbols found\n";
  for (j = 0; j < nptrs; j++)
    oss << strings[j] << "\n";
  free(strings);

  #undef BUFFER_SIZE

  printf ("exit dumping backtrace ...\n\n");

  return oss.str();
}

////////////////////////////////////////////////////////////////////////////////

Uint OSystemLayer::process_id() const
{
  pid_t pid = getpid();
  return static_cast<Uint> ( pid );
}

////////////////////////////////////////////////////////////////////////////////

CF::Real OSystemLayer::memory_usage() const
{

  struct task_basic_info t_info;
  mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

  if (KERN_SUCCESS != task_info(mach_task_self(),
                                TASK_BASIC_INFO, (task_info_t)&t_info,
                                &t_info_count))
  {
      return -1;
  }
  // resident size is in t_info.resident_size;
  // virtual size is in t_info.virtual_size;

  return static_cast<double>(t_info.resident_size);
}

////////////////////////////////////////////////////////////////////////////////

/// Following functions are required since they are not available for Mac OSX
/// This only works for intel architecture
/// http://www-personal.umich.edu/~williams/archive/computation/fe-handling-example.c

#if 0 // not used for the moment
static int
fegetexcept (void)
{
  static fenv_t fenv;

  return fegetenv (&fenv) ? -1 : (fenv.__control & FE_ALL_EXCEPT);
}
#endif

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

#if 0 // not used for the moment
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
#endif

////////////////////////////////////////////////////////////////////////////////

void OSystemLayer::regist_os_signal_handlers()
{
  // register handler functions for the signals
  signal(SIGFPE,    (sighandler_t) MacOSX::OSystemLayer::handleSIGFPE);
  signal(SIGSEGV,   (sighandler_t) MacOSX::OSystemLayer::handleSIGSEGV);

  // enable the exceptions that will raise the SIGFPE signal
  feenableexcept ( FE_DIVBYZERO );
  feenableexcept ( FE_INVALID   );
  feenableexcept ( FE_OVERFLOW  );
  feenableexcept ( FE_UNDERFLOW );
}

////////////////////////////////////////////////////////////////////////////////

int OSystemLayer::handleSIGFPE (int signal)
{
  printf("\nreceived signal SIGFPE [%d] - 'Floating Point Exception'\n",signal);
  static std::string dump = MacOSX::OSystemLayer::dumpBacktrace();
  printf( "%s\n", dump.c_str() );
  throw Common::FloatingPointError (FromHere(), "Some floating point operation has given an invalid result");
}

////////////////////////////////////////////////////////////////////////////////

int OSystemLayer::handleSIGSEGV(int signal)
{
  printf("\nreceived signal SIGSEGV [%d] - 'Segmentation violation'\n",signal);
  static std::string dump = MacOSX::OSystemLayer::dumpBacktrace();
  printf( "%s\n", dump.c_str() );
  abort();
}

////////////////////////////////////////////////////////////////////////////////

    } // MacOSX
  } // Common
} // CF

