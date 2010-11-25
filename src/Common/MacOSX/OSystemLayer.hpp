// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_MacOSX_OSystemLayer_hpp
#define CF_Common_MacOSX_OSystemLayer_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/OSystemLayer.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace MacOSX {

////////////////////////////////////////////////////////////////////////////////

/// This class represents the current information on the memory usage.
/// Is is an implementation for the MacOSX operating system
/// @author Tiago Quintino
class Common_API OSystemLayer : public Common::OSystemLayer {

public:

  /// Constructor without arguments
  OSystemLayer();

  /// Destructor
  virtual ~OSystemLayer();

  /// @returns string with platform name
  virtual std::string getPlatformName () const { return "MacOSX"; }

  /// Dump backtrace
  static std::string dumpBackTrace ();

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

protected:

  /// SIGFPE signal handler
  static int handleSIGFPE(int signal);

  /// SIGSEGV signal handler
  static int handleSIGSEGV(int signal);

}; // OSystemLayer

////////////////////////////////////////////////////////////////////////////////

} // MacOSX
} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MacOSX_OSystemLayer_hpp
