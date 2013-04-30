// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_OSystemLayer_hpp
#define cf3_common_OSystemLayer_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Exception.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

class Common_API OSystemError : public common::Exception {
public:
  /// Constructor
  OSystemError ( const common::CodeLocation& where, const std::string& what);
}; // end class OSystem

////////////////////////////////////////////////////////////////////////////////

/// This class represents the current information on the memory usage.
/// @author Tiago Quintino
class Common_API OSystemLayer : public boost::noncopyable {

public: // functions

  /// signal function type
  typedef void (*sighandler_t)(int);

  /// Constructor without arguments
  OSystemLayer();

  /// Destructor
  virtual ~OSystemLayer();

  /// @returns string with platform name
  virtual std::string platform_name () const = 0;

  /// Dump backtrace
  /// The format of the backtrace is operating system dependent
  /// @returns a string with the backtrace dump
  virtual std::string back_trace () const = 0;

  /// Gets the current process ID
  /// @return a integer witht he current process ID
  virtual cf3::Uint process_id () const = 0;

  /// Regists the signal handlers that will be handled by this class
  virtual void regist_os_signal_handlers() = 0;

  /// Gets the memory usage
  /// @return a double with the memory usage in bytes
  virtual cf3::Real memory_usage () const = 0;

  /// @returns a string with the memory usage
  /// @post adds the unit of memory (B, KB, MB or GB)
  /// @post  no end of line added
  /// @param out the output stream
  std::string memory_usage_str () const;

  /// Executes the command passed in the string
  /// @todo should return the output of the command but not yet implemented.
  void execute_command (const std::string& call);

  /// Gets the Class name
  static std::string type_name() { return "OSystemLayer"; }

}; // OSystemLayer

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_OSystemLayer_hpp
