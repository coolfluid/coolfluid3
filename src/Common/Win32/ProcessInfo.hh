#ifndef COOLFluiD_Common_ProcessInfoWin32_hh
#define COOLFluiD_Common_ProcessInfoWin32_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/ProcessInfo.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Common {

//////////////////////////////////////////////////////////////////////////////

/// This class represents the current information on the memory usage.
/// Is is an implementation for the Win32 operating system
/// @author Tiago Quintino
class Common_API ProcessInfoWin32 :
  public ProcessInfo {

public:

  /// Constructor without arguments
  ProcessInfoWin32();

  /// Destructor
  virtual ~ProcessInfoWin32();

  /// @returns string with platform name
  virtual std::string getPlatformName () const { return "Win32"; };

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

#endif // COOLFluiD_Common_ProcessInfoWin32_hh
