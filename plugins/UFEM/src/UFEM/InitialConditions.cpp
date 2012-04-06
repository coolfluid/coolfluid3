// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionArray.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"

#include "common/XML/SignalOptions.hpp"

#include "math/VariableManager.hpp"
#include "math/VariablesDescriptor.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"

#include "solver/Tags.hpp"

#include "InitialConditions.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace common::XML;
using namespace math;
using namespace mesh;
using namespace solver;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < InitialConditions, ActionDirector, LibUFEM > InitialConditions_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

InitialConditions::InitialConditions ( const std::string& name ) : ActionDirector ( name )
{
}

InitialConditions::~InitialConditions()
{
}

Handle<InitialCondition> InitialConditions::create_initial_condition(const std::string& tag)
{
  Handle<InitialCondition> ic = create_component<InitialCondition>(tag);

  ic->options().configure_option(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
  ic->options().configure_option("field_tag", tag);

  return ic;
}

void InitialConditions::signature_create_initial_condition(SignalArgs& args)
{
  SignalOptions options(args);
  options.add_option("field_tag", "").pretty_name("Field Tag").description("Tag of the field for which the initial condition is to be set");
}


void InitialConditions::signal_create_initial_condition(SignalArgs& args)
{
  SignalOptions options(args);
  Handle<InitialCondition> ic = create_initial_condition(options["field_tag"].value<std::string>());
  
  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("created_component", ic->uri());
}


} // UFEM
} // cf3
