#ifndef COOLFluiD_Common_ProcessInfo_hh
#define COOLFluiD_Common_ProcessInfo_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hh"
#include "Common/NonCopyable.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

//////////////////////////////////////////////////////////////////////////////

/// This class represents the current information on the memory usage.
/// @author Tiago Quintino
class Common_API ProcessInfo : public Common::NonCopyable<ProcessInfo> {

public: // functions

  /// Constructor without arguments
  ProcessInfo();

  /// Destructor
  virtual ~ProcessInfo();

  /// @returns string with platform name
  virtual std::string getPlatformName () const = 0;

  /// Dump backtrace
  /// The format of the backtrace is operating system dependent
  /// @returns a string with the backtrace dump
  virtual std::string getBackTrace () const = 0;

  /// Gets the current process ID
  /// @return a integer witht he current process ID
  virtual CF::Uint getPID () const = 0;

  /// Gets the memory usage
  /// @return a double with the memory usage in bytes
  virtual CF::Real memoryUsageBytes () const = 0;

  /// @returns a string with the memory usage
  /// @post adds the unit of memory (B, KB, MB or GB)
  /// @post  no end of line added
  /// @param out the output stream
  std::string memoryUsage () const;

  /// Gets the Class name
  static std::string getClassName() { return "ProcessInfo"; }

}; // end of class ProcessInfo

//////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_Common_ProcessInfo_hh
