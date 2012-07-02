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

common::ComponentBuilder < InitialConditions, common::ActionDirector, LibUFEM > InitialConditions_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

InitialConditions::InitialConditions ( const std::string& name ) : solver::ActionDirector ( name )
{
  regist_signal( "create_initial_condition" )
    .connect( boost::bind( &InitialConditions::signal_create_initial_condition, this, _1 ) )
    .description("Create an initial condition")
    .pretty_name("Create Initial Condition")
    .signature( boost::bind ( &InitialConditions::signature_create_initial_condition, this, _1) );
}

InitialConditions::~InitialConditions()
{
}

Handle<common::Action> InitialConditions::create_initial_condition(const std::string& tag, const std::string& builder_name)
{
  Handle<common::Action> ic(create_component(tag, builder_name));

  ic->options().set(solver::Tags::physical_model(), options().option(solver::Tags::physical_model()).value());
  ic->options().set(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
  if(ic->options().check("field_tag"))
    ic->options().set("field_tag", tag);
  
  ic->mark_basic();

  return ic;
}

void InitialConditions::signature_create_initial_condition(SignalArgs& args)
{
  SignalOptions options(args);
  options.add("field_tag", "").pretty_name("Field Tag").description("Tag of the field for which the initial condition is to be set").mark_basic();
  options.add("builder_name", "cf3.UFEM.InitialConditionConstant").pretty_name("Builder Name").description("Builder to determine the initial condition type");
}


void InitialConditions::signal_create_initial_condition(SignalArgs& args)
{
  SignalOptions options(args);
  Handle<common::Action> ic;
  if(options.check("builder_name"))
  {
    ic = create_initial_condition(options["field_tag"].value<std::string>(), options["builder_name"].value<std::string>());
  }
  else
  {
    ic = create_initial_condition(options["field_tag"].value<std::string>());
  }

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", ic->uri());
}


} // UFEM
} // cf3
