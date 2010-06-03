#ifdef CF_HAVE_UNISTD_H
  #include <unistd.h>
#endif

#include <cstdio>        // for printf()
#include <cstdlib>       // for free() and abort()
#include <csignal>       // POSIX signal(), SIGFPE and SIGSEGV
#include <fenv.h>        // floating Common access
#include <sstream>       // streamstring
#include <execinfo.h>    // for backtrace() from glibc
#include <sys/types.h>   // for getting the PID of the process
#include <malloc.h>      //  for mallinfo

#include "Common/BasicExceptions.hpp"
#include "Common/Linux/OSystemLayer.hpp"
#include "Common/Log.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace CF {
namespace Common {
namespace Linux {

////////////////////////////////////////////////////////////////////////////////

OSystemLayer::OSystemLayer()
{
}

////////////////////////////////////////////////////////////////////////////////

OSystemLayer::~OSystemLayer()
{
}

////////////////////////////////////////////////////////////////////////////////

std::string OSystemLayer::getBackTrace () const
{
  return dumpBacktrace ();
}

////////////////////////////////////////////////////////////////////////////////

std::string OSystemLayer::dumpBacktrace ()
{
#define CF_BUFFER_SIZE 256

  printf ("\n\ndumping backtrace ...\n");

  std::ostringstream oss;
  int j, nptrs;
  void *buffer[CF_BUFFER_SIZE];
  char **strings;

  nptrs = backtrace(buffer, CF_BUFFER_SIZE);
  oss << "\nbacktrace() returned " << nptrs << " addresses\n";

  strings = backtrace_symbols(buffer, nptrs);
  if (strings == NULL)
    oss << "\nno backtrace_symbols found\n";
  for (j = 0; j < nptrs; j++)
    oss << strings[j] << "\n";
  free(strings);

#undef CF_BUFFER_SIZE

  return oss.str();
}

////////////////////////////////////////////////////////////////////////////////

Uint OSystemLayer::getPID() const
{
  pid_t pid = getpid();
  return static_cast<Uint> ( pid );
}

////////////////////////////////////////////////////////////////////////////////

double OSystemLayer::memoryUsageBytes() const
{
  struct mallinfo info;

  // get current memory
  info = mallinfo();
#if 0
  std::cout
  << "### MALLINFO ###"  << std::endl
  << "  number of free chunks " <<  info.ordblks << std::endl
  << "  number of fastbin blocks " << info.smblks << std::endl
  << "  number of mmapped regions " << info.hblks << std::endl
  << "  non-mmapped space allocated from system " << info.arena /1024 << "KB" << std::endl
  << "  space in mmapped regions " << info.hblkhd /1024 << "KB" << std::endl
  << "  maximum total allocated space " << info.usmblks/1024 << "KB"<< std::endl
  << "  space available in freed fastbin blocks " << info.fsmblks/1024 << "KB"<< std::endl
  << "  total allocated space " << info.uordblks/1024 << "KB"<< std::endl
  << "  total free space " << info.fordblks/1024 << "KB" << std::endl
  << "  top-most, releasable (via malloc_trim) space " << info.keepcost/1024 << "KB"<< std::endl
  << "################" << std::endl
  << "SUM BEFORE : " << ( info.arena + info.hblkhd ) / 1024  << "KB" << std::endl
  << "SUM ALL    : " << ( info.arena + info.hblkhd + info.fsmblks + info.uordblks ) / 1024 << "KB" << std::endl
  << "################" << std::endl;

  char buf[30];
  snprintf(buf, 30, "/proc/%u/statm", (unsigned)getpid());
        FILE* pf = fopen(buf, "r");
        if (pf) {
            unsigned size; //       total program size
            unsigned resident;//   resident set size
            unsigned share;//      shared pages
            unsigned text;//       text (code)
            unsigned lib;//        library
            unsigned data;//       data/stack
            unsigned dt;//         dirty pages (unused in Linux 2.6)
            fscanf(pf, "%u %u %u %u %u %u", &size, &resident, &share, &text, &lib, &data);
            CFLog(INFO, "NEW METHOD size" << size / (1024.0) << " MB mem used\n");
            CFLog(INFO, "NEW METHOD resident" << resident / (1024.0) << " MB mem used\n");
            CFLog(INFO, "NEW METHOD share" << share / (1024.0) << " MB mem used\n");
            CFLog(INFO, "NEW METHOD text" << text / (1024.0) << " MB mem used\n");
            CFLog(INFO, "NEW METHOD lib" << lib / (1024.0) << " MB mem used\n");
            CFLog(INFO, "NEW METHOD data" << data / (1024.0) << " MB mem used\n");
        }
#endif

  return static_cast<double>(info.arena) +
         static_cast<double>(info.hblkhd);
}

////////////////////////////////////////////////////////////////////////////////

void OSystemLayer::registOSystemLayers()
{
  // register handler functions for the signals
  signal(SIGFPE,    (sighandler_t) Linux::OSystemLayer::handleSIGFPE);
  signal(SIGSEGV,   (sighandler_t) Linux::OSystemLayer::handleSIGSEGV);

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
  static std::string dump = Linux::OSystemLayer::dumpBacktrace();
  printf( "%s\n", dump.c_str() );
  throw Common::FloatingPointError (FromHere(), "Some floating point operation has given an invalid result");
}

////////////////////////////////////////////////////////////////////////////////

int OSystemLayer::handleSIGSEGV(int signal)
{
  printf("\nreceived signal SIGSEGV [%d] - 'Segmentation violation'\n",signal);
  static std::string dump = Linux::OSystemLayer::dumpBacktrace();
  printf( "%s\n", dump.c_str() );
  abort();
}

////////////////////////////////////////////////////////////////////////////////

} // Linux
} // Common
} // CF

