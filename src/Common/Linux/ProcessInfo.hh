#ifndef COOLFluiD_Common_ProcessInfoLinux_hh
#define COOLFluiD_Common_ProcessInfoLinux_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/ProcessInfo.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Common {

//////////////////////////////////////////////////////////////////////////////

/// This class represents the current information on the memory usage.
/// Is is an implementation for the Linux operating system
/// @author Tiago Quintino
class Common_API ProcessInfoLinux :  public ProcessInfo {

public:

  /// Constructor without arguments
  ProcessInfoLinux();

  /// Destructor
  virtual ~ProcessInfoLinux();

  /// @returns string with platform name
  virtual std::string getPlatformName () const { return "Linux"; };

  /// @returns string with platform name
  static std::string dumpBacktrace ();

  /// Dump backtrace
  /// @returns a string with the backtrace dump
  virtual std::string getBackTrace () const;

  /// Gets the current process ID
  /// @return a integer witht he current process ID
  virtual CFuint getPID () const;

  /// Gets the memory usage
  /// @return a double with the memory usage
  virtual CFdouble memoryUsageBytes() const;

}; // end of class ProcessInfo

//////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_Common_ProcessInfoLinux_hh
