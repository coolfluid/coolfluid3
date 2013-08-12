// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/Group.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/URI.hpp"

#include "python/LibPython.hpp"
#include "python/TestSignals.hpp"

namespace cf3 {
namespace python {

using namespace common;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < TestSignals, Component, LibPython > TestSignals_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

TestSignals::TestSignals ( const std::string& name ) : Component ( name )
{
  options().add("real", 0.).mark_basic();
  
  regist_signal( "set_real" )
    .connect( boost::bind( &TestSignals::signal_set_real, this, _1 ) )
    .description("Set real value")
    .pretty_name("Set Real")
    .signature( boost::bind( &TestSignals::signature_set_real, this, _1 ) );
}


TestSignals::~TestSignals()
{
}

void TestSignals::signal_set_real(SignalArgs& args)
{
  common::XML::SignalOptions options(args);
  this->options().set("real", options.value<Real>("real"));
}

void TestSignals::signature_set_real(SignalArgs& args)
{
  common::XML::SignalOptions options(args);
  options.add("real", 0.).description("Number to set");
}





////////////////////////////////////////////////////////////////////////////////////////////

} // python
} // cf3
