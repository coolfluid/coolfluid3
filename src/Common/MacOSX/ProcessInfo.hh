#ifndef COOLFluiD_Common_ProcessInfoMacOSX_hh
#define COOLFluiD_Common_ProcessInfoMacOSX_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/ProcessInfo.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Common {

//////////////////////////////////////////////////////////////////////////////

/// This class represents the current information on the memory usage.
/// Is is an implementation for the MacOSX operating system
/// @author Tiago Quintino
class Common_API ProcessInfoMacOSX :
  public ProcessInfo {

public:

  /// Constructor without arguments
  ProcessInfoMacOSX();

  /// Destructor
  virtual ~ProcessInfoMacOSX();

  /// @returns string with platform name
  virtual std::string getPlatformName () const { return "MacOSX"; };

  /// Dump backtrace
  static std::string dumpBackTrace ();
  
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

#endif // COOLFluiD_Common_ProcessInfoMacOSX_hh
