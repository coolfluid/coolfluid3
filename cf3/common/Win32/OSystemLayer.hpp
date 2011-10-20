// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Win32_OSystemLayer_hpp
#define CF_Common_Win32_OSystemLayer_hpp

/////////////////////////////////OSystemLayer///////////////////////////////////

#include "common/OSystemLayer.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
  
/// @brief Specialized classes for interacting with Win32 operating system
namespace Win32 {

////////////////////////////////////////////////////////////////////////////////

/// This class represents the current information on the memory usage.
/// Is is an implementation for the Win32 operating system
/// @author Tiago Quintino
class Common_API OSystemLayer : public common::OSystemLayer {

public:

  /// Constructor without arguments
  OSystemLayer();

  /// Destructor
  virtual ~OSystemLayer();

  /// @returns string with platform name
  virtual std::string platform_name () const { return "Win32"; };

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

}; // end of class OSystemLayer

////////////////////////////////////////////////////////////////////////////////

} // Win32
} // namespace common
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Win32_OSystemLayer_hpp
