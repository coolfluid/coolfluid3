// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file tut3.cpp
/// @brief Tutorial 3
///
/// This tutorial shows how to create a component, with
/// the same functionality as in Tutorial 2, but implemented
/// using a dynamic function or "signal". Functions implemented
/// in this way can be called in dynamic languages such as
/// our GUI, our python interface.

#include "common/Log.hpp"
#include "common/Component.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionT.hpp"

#include "common/Signal.hpp"
#include "common/SignalHandler.hpp"
#include "common/XML/SignalOptions.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

/// Component class Hello
class Hello : public Component
{
public:

  /// type name, mandatory to be there!
  static string type_name() { return "Hello"; }

  /// Constructor, with the name of the component as argument
  Hello(const string& name) : Component(name)
  {

    // Register a dynamic function, that can be called from a GUI, or script
    regist_signal( "print" )
        .connect(   boost::bind( &Hello::signal_print,    this, _1 ) )
        .signature( boost::bind( &Hello::signature_print, this, _1 ) )
        .description("Print 'Hello <person>!'");

  }

  /// Destructor, must always be declared virtual
  virtual ~Hello() {}




  /// Signal for to print "Hello <person>!", where <person> is configurable.
  void signal_print(SignalArgs& args)
  {
    // Get the options from the args
    SignalOptions options( args );

    // Print
    CFinfo << "Hello " << options["person"].value<string>() << "!" << CFendl;
  }

  /// Signature for signal_print()
  void signature_print(SignalArgs& args)
  {
    // Get the options from the args
    SignalOptions options( args );

    // Add the option "person"
    options.add_option("person", string("nobody") )
        .description("Person's name to say Hello to");
  }

};

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
  // Allocate new component
  boost::shared_ptr<Hello> hello = allocate_component<Hello>("hello");

  // Following is just to check if signals are implemented correctly.
  // If this works, the signal can now be called in a dynamic language such as our GUI,
  // or our python interface

  // The default behaviour, with the signature, containing default arguments,
  // and explains what is expected from the user.
  SignalArgs args;
  hello->signature_print(args);
  hello->signal_print(args);
  // output:    Hello nobody!

  // The behaviour where the arguments are given by a user. Note that a signal_signature
  // is not strictly necessary, but good practise.
  SignalOptions options(args);
  options["person"].change_value( string("You") );
  options.flush();
  hello->signal_print(args);
  // output:    Hello You!

  return 0;
}
