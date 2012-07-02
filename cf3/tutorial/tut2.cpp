// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file tut2.cpp
/// @brief Tutorial 2
///
/// This tutorial shows how to create a simple component "Hello",
/// demonstrating how options and properties are used.

#include "common/Log.hpp"
#include "common/Component.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionT.hpp"
#include "common/Signal.hpp"
#include "common/SignalHandler.hpp"

#include "tutorial/Hello.hpp"

using namespace std;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::tutorial;

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
  {
    // Allocate new component
    boost::shared_ptr<Hello> hello = allocate_component<Hello>("hello");


    hello->print();
    // output:  Hello world!


    hello->options().set("message",string("Goodbye world!"));
    hello->print();
    // output:  Goodbye world!

    // Access a property from the component
    CFinfo << "Print is called " << hello->properties().value<int>("print_count") << " times." << CFendl;
  }
  // ------------------------------------------------------------------------------------------ //

  /// Tutorial 2, dynamic interface
  /// Start "cf3/Tools/Command/coolfluid-command"
  /// This opens the coolfluid shell. Put following output
  /// [/] create hello cf3.tutorial.Hello
  /// [/] call hello/print
  /// Hello world!
  /// [/] call hello/print  message:string="Goodbye world!"
  /// Goodbye world!

  // Following is just to check if signals are implemented correctly.
  // If this works, the signal can now be called in a dynamic language such as our GUI,
  // or our python interface
  {
    // Build component using the registered builder by its name
    boost::shared_ptr<Component> hello = build_component("cf3.tutorial.Hello","hello_world");
    // The default behaviour, with the signature, containing default arguments,
    // and explains what is expected from the user.
    SignalArgs args;
    (*hello->signal("print")->signature())(args);
    hello->call_signal("print",args);
    // output:    Hello world!

    // The behaviour where the arguments are given by a user. Note that a signal_signature
    // is not strictly necessary, but good practise.
    SignalOptions options(args);
    options["message"].change_value( string("Goodbye world!") );
    options.flush();
    hello->call_signal("print",args);
    // output:    Goodbye world!
  }


  return 0;
}
