// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_tutorial_Hello_hpp
#define cf3_tutorial_Hello_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"
#include "tutorial/LibTutorial.hpp"

namespace cf3 {
namespace tutorial {

////////////////////////////////////////////////////////////////////////////////

/// Component class Hello
///
/// Requirements:
/// - Keyword tutorial_API (here defined in tutorial/LibTutorial.hpp)
///   is necessary for Windows dynamic library registration.
/// - derive from common::Component to get all component functionality
/// - type_name() function is needed for cross-platform type registration
/// - Constructor always needs to be of the form with name argument, to assign
///   a name to the component.
/// - Destructor must always be virtual
class tutorial_API Hello : public common::Component
{
public:

  /// type name, mandatory to be there!
  static std::string type_name() { return "Hello"; }

  /// Constructor, with the name of the component as argument
  Hello(const std::string& name);

  /// Destructor, must always be declared virtual
  virtual ~Hello();
  
  // Extra specialized functions, giving purpose to this component type
  
  /// Print "<message>", where <message> is configurable
  void print();

private:

  // For Tutorial 3
  
  /// A signal that prints "<message>", but instead of taking a
  /// configured message, takes it from the passed arguments
  void signal_print(common::SignalArgs& args);

  /// Signature for signal_print(), 
  /// giving default value for <message> to be "Hello world!"
  void signature_print(common::SignalArgs& args);

};

////////////////////////////////////////////////////////////////////////////////

} // tutorial
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_tutorial_Hello_hpp
