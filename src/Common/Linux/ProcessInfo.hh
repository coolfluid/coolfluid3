#ifndef CF_Common_Linux_ProcessInfo_hh
#define CF_Common_Linux_ProcessInfo_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/ProcessInfo.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {
    namespace Linux {

//////////////////////////////////////////////////////////////////////////////

/// This class represents the current information on the memory usage.
/// Is is an implementation for the Linux operating system
/// @author Tiago Quintino
class Common_API ProcessInfo :  public ProcessInfo {

public:

  /// Constructor without arguments
  ProcessInfo();

  /// Destructor
  virtual ~ProcessInfo();

  /// @returns string with platform name
  virtual std::string getPlatformName () const { return "Linux"; };

  /// @returns string with platform name
  static std::string dumpBacktrace ();

  /// Dump backtrace
  /// @returns a string with the backtrace dump
  virtual std::string getBackTrace () const;

  /// Gets the current process ID
  /// @return a integer witht he current process ID
  virtual Uint getPID () const;

  /// Gets the memory usage
  /// @return a double with the memory usage
  virtual double memoryUsageBytes() const;

}; // end of class ProcessInfo

//////////////////////////////////////////////////////////////////////////////

    } // Linux
  } // namespace Common
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Linux_ProcessInfo_hh
