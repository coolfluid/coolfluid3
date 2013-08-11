// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_TestSignals_hpp
#define cf3_common_TestSignals_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

#include "LibPython.hpp"

namespace cf3 {
namespace python {

////////////////////////////////////////////////////////////////////////////////

/// @brief Exposes certain signals for testing purposes
///
/// @author Bart Janssens
class Python_API TestSignals : public common::Component
{

public: // functions

  /// Contructor
  /// @param name of the component
  TestSignals ( const std::string& name );

  /// Virtual destructor
  virtual ~TestSignals();

  /// Get the class name
  static std::string type_name () { return "TestSignals"; }
  
private:
  void signal_set_real(common::SignalArgs& args);
  void signature_set_real(common::SignalArgs& args);
  

}; // TestSignals

////////////////////////////////////////////////////////////////////////////////

} // python
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_TestSignals_hpp
