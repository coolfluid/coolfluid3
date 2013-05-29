// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Environment_hpp
#define cf3_common_Environment_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

/// Component that defines global environment
/// @author Quentin Gasper
class Common_API Environment : public Component
{

public: // functions

  /// Contructor
  /// @param name of the component
  Environment ( const std::string& name );

  /// Virtual destructor
  virtual ~Environment();

  /// Get the class name
  static std::string type_name () { return "Environment"; }

private: // functions

  void trigger_only_cpu0_writes();

  void trigger_assertion_throws();

  void trigger_assertion_backtrace();

  void trigger_disable_assertions();

  void trigger_exception_outputs();

  void trigger_exception_backtrace();

  void trigger_exception_aborts();

  void trigger_log_level();

}; // Environment

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Environment_hpp
