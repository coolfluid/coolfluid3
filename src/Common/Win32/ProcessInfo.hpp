#ifndef CF_Common_Win32_ProcessInfo_hpp
#define CF_Common_Win32_ProcessInfo_hpp

/////////////////////////////////ProcessInfo/////////////////////////////////////////////

#include "Common/ProcessInfo.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {
namespace Win32 {

////////////////////////////////////////////////////////////////////////////////

/// This class represents the current information on the memory usage.
/// Is is an implementation for the Win32 operating system
/// @author Tiago Quintino
class Common_API ProcessInfo : public Common::ProcessInfo {

public:

  /// Constructor without arguments
  ProcessInfo();

  /// Destructor
  virtual ~ProcessInfo();

  /// @returns string with platform name
  virtual std::string getPlatformName () const { return "Win32"; };

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

////////////////////////////////////////////////////////////////////////////////

} // Win32
} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Win32_ProcessInfo_hpp
