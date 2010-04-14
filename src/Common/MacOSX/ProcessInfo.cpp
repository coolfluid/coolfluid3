#include <execinfo.h>    // for backtrace() from glibc
#include <sys/types.h>   // for getting the PID of the process

#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/task.h>

#include "Common/MacOSX/ProcessInfo.hpp"
#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace CF {
  namespace Common {
    namespace MacOSX {

////////////////////////////////////////////////////////////////////////////////

ProcessInfo::ProcessInfo()
{
}

////////////////////////////////////////////////////////////////////////////////

ProcessInfo::~ProcessInfo()
{
}

////////////////////////////////////////////////////////////////////////////////

std::string ProcessInfo::getBackTrace () const
{
  printf ("getBackTrace ...\n");
  return dumpBackTrace ();
}

////////////////////////////////////////////////////////////////////////////////

std::string ProcessInfo::dumpBackTrace ()
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

Uint ProcessInfo::getPID() const
{
  pid_t pid = getpid();
  return static_cast<Uint> ( pid );
}

////////////////////////////////////////////////////////////////////////////////

double ProcessInfo::memoryUsageBytes() const
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

    } // MacOSX
  } // namespace Common
} // namespace CF

