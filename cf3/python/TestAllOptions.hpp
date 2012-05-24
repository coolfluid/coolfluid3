// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_TestAllOptions_hpp
#define cf3_common_TestAllOptions_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

namespace cf3 {
namespace python {

////////////////////////////////////////////////////////////////////////////////

/// @brief Creates an option for each type, to enable testing of python option handling
///
/// @author Bart Janssens
class Common_API TestAllOptions : public common::Component
{

public: // functions

  /// Contructor
  /// @param name of the component
  TestAllOptions ( const std::string& name );

  /// Virtual destructor
  virtual ~TestAllOptions();

  /// Get the class name
  static std::string type_name () { return "TestAllOptions"; }
  
private:
  void trigger_debug();

}; // TestAllOptions

////////////////////////////////////////////////////////////////////////////////

} // python
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_TestAllOptions_hpp
