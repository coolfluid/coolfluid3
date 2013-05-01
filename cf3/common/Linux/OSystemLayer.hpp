// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Linux_OSystemLayer_hpp
#define CF_Common_Linux_OSystemLayer_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/OSystemLayer.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/// @brief Specialized classes for interacting with Linux operating system
namespace Linux {

////////////////////////////////////////////////////////////////////////////////

/// This class represents the current information on the memory usage.
/// Is is an implementation for the Linux operating system
/// @author Tiago Quintino
class Common_API OSystemLayer :  public common::OSystemLayer {

public:

  /// Constructor without arguments
  OSystemLayer();

  /// Destructor
  virtual ~OSystemLayer();

  /// @returns string with platform name
  virtual std::string platform_name () const { return "Linux"; };

  /// @returns string with platform name
  static std::string dump_back_trace ();

  /// Dump backtrace
  /// @returns a string with the backtrace dump
  virtual std::string back_trace () const;

  /// Gets the current process ID
  /// @return a integer witht he current process ID
  virtual Uint process_id () const;

  /// Gets the memory usage
  /// @return a double with the memory usage
  virtual double memory_usage() const;

  /// Regists the signal handlers that will be handled by this class
  virtual void regist_os_signal_handlers();

protected: // methods

  /// SIGFPE signal handler
  static int handleSIGFPE(int signal);

  /// SIGSEGV signal handler
  static int handleSIGSEGV(int signal);

}; // end of class OSystemLayer

////////////////////////////////////////////////////////////////////////////////

} // Linux
} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Linux_OSystemLayer_hpp
