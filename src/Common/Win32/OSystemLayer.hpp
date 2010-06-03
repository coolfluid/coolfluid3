#ifndef CF_Common_Win32_OSystemLayer_hpp
#define CF_Common_Win32_OSystemLayer_hpp

/////////////////////////////////OSystemLayer///////////////////////////////////

#include "Common/OSystemLayer.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace Win32 {

////////////////////////////////////////////////////////////////////////////////

/// This class represents the current information on the memory usage.
/// Is is an implementation for the Win32 operating system
/// @author Tiago Quintino
class Common_API OSystemLayer : public Common::OSystemLayer {

public:

  /// Constructor without arguments
  OSystemLayer();

  /// Destructor
  virtual ~OSystemLayer();

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

  /// Regists the signal handlers that will be handled by this class
  virtual void registSignalHandlers();

}; // end of class OSystemLayer

////////////////////////////////////////////////////////////////////////////////

} // Win32
} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Win32_OSystemLayer_hpp
