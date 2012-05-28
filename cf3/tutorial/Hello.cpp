// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/Component.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionT.hpp"
#include "common/Log.hpp"

#include "common/Signal.hpp"
#include "common/SignalHandler.hpp"
#include "common/XML/SignalOptions.hpp"

#include "tutorial/Hello.hpp"

using namespace cf3::common;

namespace cf3 {
namespace tutorial {

////////////////////////////////////////////////////////////////////////////////////////////

/// Builder, registering this component "Hello" in the LibTutorial library,
/// so it can be built using a string "cf3.tutorial.Hello"
common::ComponentBuilder < Hello, Component, LibTutorial > Hello_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

Hello::Hello ( const std::string& name ) : Component ( name )
{
  // Create a configurable 
  options().add("message",std::string("Hello world!"));
  
  properties().add("print_count",0);
  
  // Register a dynamic function, that can be called from a GUI, or script
  regist_signal( "print" )
      .connect(   boost::bind( &Hello::signal_print,    this, _1 ) )
      .signature( boost::bind( &Hello::signature_print, this, _1 ) )
      .description("Print '<message> <towards>!'");

}

////////////////////////////////////////////////////////////////////////////////

Hello::~Hello()
{
}

////////////////////////////////////////////////////////////////////////////////////////////

void Hello::print()
{
  // Print configurable option message
  CFinfo << options()["message"].value<std::string>() << CFendl;
  
  // increase print_count
  properties()["print_count"] = properties().value<int>("print_count") + 1;
}

////////////////////////////////////////////////////////////////////////////////////////////

void Hello::signal_print(SignalArgs& args)
{
  // Get the options from the args
  SignalOptions sig_opts( args );

  // Print now NOT the configurable option, 
  // but a passed argument to the signal.
  CFinfo << sig_opts["message"].value<std::string>() << CFendl;
}

/// Signature for signal_print()
void Hello::signature_print(SignalArgs& args)
{
  // Get the options from the args
  SignalOptions sig_opts( args );

  // Add the option "person"
  sig_opts.add("message", std::string("Hello world!") )
      .description("Message to print");
}

////////////////////////////////////////////////////////////////////////////////

} // tutorial
} // cf3
