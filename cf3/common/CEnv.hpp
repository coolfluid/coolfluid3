// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_CEnv_hpp
#define cf3_common_CEnv_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

/// Component that defines global environment
/// @author Quentin Gasper
class Common_API CEnv : public Component {

public: //typedefs

  typedef boost::shared_ptr<CEnv> Ptr;
  typedef boost::shared_ptr<CEnv const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CEnv ( const std::string& name );

  /// Virtual destructor
  virtual ~CEnv();

  /// Get the class name
  static std::string type_name () { return "CEnv"; }

private: // functions

  void trigger_only_cpu0_writes();

  void trigger_assertion_throws();

  void trigger_assertion_backtrace();

  void trigger_disable_assertions();

  void trigger_exception_outputs();

  void trigger_exception_backtrace();

  void trigger_exception_aborts();

  void trigger_log_level();

}; // CEnv

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_CEnv_hpp
