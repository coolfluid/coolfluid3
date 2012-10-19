// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include <common/EventHandler.hpp>

#include "math/VariableManager.hpp"
#include "math/VariablesDescriptor.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include <mesh/Space.hpp>

#include "solver/Tags.hpp"
#include "solver/actions/Probe.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/History.hpp"

#include "physics/PhysModel.hpp"

#include "InitialConditions.hpp"
#include "Solver.hpp"
#include "SparsityBuilder.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace math;
using namespace mesh;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

common::ComponentBuilder < UFEM::Solver, solver::Solver, LibUFEM > UFEMSolver_Builder;

Solver::Solver(const std::string& name) :
  SimpleSolver(name),
  m_need_field_creation(true)
{
  regist_signal( "add_direct_solver" )
    .connect( boost::bind( &Solver::signal_add_direct_solver, this, _1 ) )
    .description("Create a solver needing only one LSS solve")
    .pretty_name("Create Direct Solver")
    .signature( boost::bind ( &Solver::signature_add_solver, this, _1) );

  regist_signal( "add_unsteady_solver" )
    .connect( boost::bind( &Solver::signal_add_unsteady_solver, this, _1 ) )
    .description("Create an unsteady solver, solving a linear system once every time step")
    .pretty_name("Create Unsteady Solver")
    .signature( boost::bind ( &Solver::signature_add_solver, this, _1) );

  regist_signal( "add_unsteady_advance_solver" )
    .connect( boost::bind( &Solver::signal_add_unsteady_solver, this, _1 ) )
    .description("Create an unsteady solver, solving a linear system once for several time steps")
    .pretty_name("Create Advance Unsteady Solver")
    .signature( boost::bind ( &Solver::signature_add_solver, this, _1) );

  regist_signal( "add_iteration_solver" )
    .connect( boost::bind( &Solver::signal_add_iteration_solver, this, _1 ) )
    .description("Create an iteration solver, solving a linear system more than once every time step")
    .pretty_name("Create iteration Solver")
    .signature( boost::bind ( &Solver::signature_add_solver, this, _1) );

  regist_signal( "create_initial_conditions" )
    .connect( boost::bind( &Solver::signal_create_initial_conditions, this, _1 ) )
    .description("Create initial conditions.")
    .pretty_name("Create Initial Conditions");

  regist_signal( "create_fields" )
    .connect( boost::bind( &Solver::signal_create_fields, this, _1 ) )
    .description("Create the fields required for the solver.")
    .pretty_name("Create Fields");

  regist_signal( "add_probe" )
    .connect( boost::bind( &Solver::signal_add_probe, this, _1 ) )
    .description("Add a probe to log data")
    .pretty_name("Add Probe")
    .signature( boost::bind ( &Solver::signature_add_probe, this, _1) );

  Core::instance().event_handler().connect_to_event("ufem_variables_added", this, &Solver::on_variables_added_event);
}

Solver::~Solver()
{
}

Handle< common::Action > Solver::add_direct_solver(const std::string& builder_name)
{
  if(is_null(m_initial_conditions))
  {
    create_initial_conditions();
  }

  return add_solver(builder_name, *this);
}

Handle< common::Action > Solver::add_unsteady_solver(const std::string& builder_name)
{
  if(is_null(m_initial_conditions))
  {
    create_initial_conditions();
  }

  Handle<Component> timeloop = get_child("TimeLoop");
  if(is_null(timeloop))
  {
    timeloop = create_component("TimeLoop", "cf3.solver.actions.Iterate");
    timeloop->create_component("CriterionTime", "cf3.solver.actions.CriterionTime");
  }
  else
  {
    timeloop->remove_component("AdvanceTime");
  }

  Handle<common::Action> result = add_solver(builder_name, *timeloop);

  timeloop->create_component("AdvanceTime", "cf3.solver.actions.AdvanceTime");

  return result;
}

Handle< common::Action > Solver::add_unsteady_advance_solver(const std::string& builder_name)
{
  if(is_null(m_initial_conditions))
  {
    create_initial_conditions();
  }

  Handle<Component> timeloop = get_child("TimeLoop");
  if(is_null(timeloop))
  {
    timeloop = create_component("TimeLoop", "cf3.solver.actions.Iterate");
    timeloop->mark_basic();
    timeloop->create_component("CriterionTime", "cf3.solver.actions.CriterionTime");
    timeloop->create_component("SelfIteration","cf3.solver.actions.Iterate")->mark_basic();
  }
  else
  {
    timeloop->remove_component("AdvanceTime");
  }

  Handle<Component> selfiteration = timeloop->get_child("SelfIteration");
  cf3_assert(is_not_null(selfiteration));

  Handle<common::Action> result = add_solver(builder_name, *timeloop);

  timeloop->create_component("AdvanceTime", "cf3.solver.actions.AdvanceTime");

  return result;
}

Handle< common::Action > Solver::add_iteration_solver(const std::string& builder_name)    // the add_iteration_solver for inner itarations in case of a coupling between two solvers
{
  if(is_null(m_initial_conditions))
  {
    create_initial_conditions();
  }

  Handle<Component> timeloop = get_child("TimeLoop");
  if(is_null(timeloop))
  {
      timeloop = create_component("TimeLoop", "cf3.solver.actions.Iterate");
      timeloop->mark_basic();                                                          //mark_basic to make it visible in python; there, a dot replaces the get child method
      timeloop->create_component("CriterionTime", "cf3.solver.actions.CriterionTime");
      timeloop->create_component("CouplingIteration","cf3.solver.actions.Iterate")->mark_basic();
      timeloop->create_component("AdvanceTime", "cf3.solver.actions.AdvanceTime");
  }

  Handle<Component> coupling = timeloop->get_child("CouplingIteration");
  if(is_null(coupling))
  {
    boost::shared_ptr<Component> advance_time = timeloop->remove_component("AdvanceTime");
    coupling = timeloop->create_component("CouplingIteration","cf3.solver.actions.Iterate");
    coupling->mark_basic();
    timeloop->add_component(advance_time);
  }
  cf3_assert(is_not_null(coupling));

  Handle<common::Action> result = add_solver(builder_name, *coupling);

  return result;
}

Handle<InitialConditions> Solver::create_initial_conditions()
{
  if(is_not_null(m_initial_conditions))
  {
    CFwarn << "InitialConditions were created already, returning handle to previously created component" << CFendl;
    return m_initial_conditions;
  }

  m_initial_conditions = create_component<InitialConditions>("InitialConditions");
  if(is_not_null(m_physics))
    m_initial_conditions->configure_option_recursively(solver::Tags::physical_model(), m_physics);

  m_initial_conditions->mark_basic();

  return m_initial_conditions;
}

void Solver::signature_add_solver(SignalArgs& args)
{
  SignalOptions options(args);
  options.add("builder_name", "")
    .pretty_name("Builder Names")
    .description("List the names of the builders that should be used to construct inner actions");
}

void Solver::signal_add_direct_solver(SignalArgs& args)
{
  SignalOptions options(args);
  Handle<common::Action> result = add_direct_solver(options.option("builder_name").value<std::string>());

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", result->uri());
}

void Solver::signal_add_unsteady_solver(SignalArgs& args)
{
  SignalOptions options(args);
  Handle<common::Action> result = add_unsteady_solver(options.option("builder_name").value<std::string>());

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", result->uri());
}

void Solver::signal_add_unsteady_advance_solver(SignalArgs& args)
{
  SignalOptions options(args);
  Handle<common::Action> result = add_unsteady_advance_solver(options.option("builder_name").value<std::string>());

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", result->uri());
}

void Solver::signal_add_iteration_solver(SignalArgs& args)
{
  SignalOptions options(args);
  Handle<common::Action> result = add_iteration_solver(options.option("builder_name").value<std::string>());

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", result->uri());
}

void Solver::signal_create_initial_conditions(SignalArgs& args)
{
  Handle<common::ActionDirector> ic = create_initial_conditions();

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", ic->uri());
}

void Solver::signal_create_fields(SignalArgs& args)
{
  create_fields();
}


void Solver::mesh_loaded(mesh::Mesh& mesh)
{
  cf3::solver::SimpleSolver::mesh_loaded(mesh);
  mesh_changed(mesh);
}


void Solver::mesh_changed(Mesh& mesh)
{
  CFdebug << "UFEM::Solver: Reacting to mesh_changed signal" << CFendl;
  configure_option_recursively("dictionary", mesh.geometry_fields().handle<Dictionary>());
  m_need_field_creation = true;
}

void Solver::on_variables_added_event(SignalArgs& args)
{
  // TODO: Check if the event comes from one of our children
  CFdebug << "UFEM::Solver: Reacting to ufem_variables_added event" << CFendl;
  m_need_field_creation = true;
}

void Solver::create_fields()
{
  if(!m_need_field_creation)
    return;

  if(is_null(m_mesh))
    return;

  // Reset comm patterns in case they became invalid
  BOOST_FOREACH(Dictionary& dict, find_components_recursively<Dictionary>(*m_mesh))
  {
    if(is_not_null(dict.get_child("CommPattern")))
    {
      dict.remove_component("CommPattern");
    }
  }

  // Find out what tags are used
  std::map<std::string, std::string> tags;
  BOOST_FOREACH(const ProtoAction& action, find_components_recursively<ProtoAction>(*this))
  {
    action.insert_field_info(tags);
  }

  // Create fields as needed
  for(std::map<std::string, std::string>::const_iterator it = tags.begin(); it != tags.end(); ++it)
  {
    const std::string& tag = it->first;
    const std::string& space_lib_name = it->second;

    // Find the dictionary
    Handle<Dictionary> dict;
    if(space_lib_name == "geometry")
    {
      dict = mesh().geometry_fields().handle<Dictionary>();
    }
    else
    {
      BOOST_FOREACH(Dictionary& tagged_dict, find_components_recursively_with_tag<Dictionary>(mesh(), "ufem_dict"))
      {
        if(tagged_dict.spaces().empty())
        {
          CFwarn << "Found empty dict while looking for dictionaries with tag ufem_dict" << CFendl;
          continue;
        }
        const std::string sf_name = tagged_dict.spaces().front()->shape_function().derived_type_name();
        if(boost::algorithm::starts_with(sf_name, space_lib_name))
        {
          if(is_null(dict))
          {
            CFinfo << "Found ufem_dict " << tagged_dict.uri().path() << CFendl;
            dict = tagged_dict.handle<Dictionary>();
          }
          else
          {
            CFwarn << "Duplicate ufem_dict " << tagged_dict.uri().path() << " ignored." << CFendl;
          }
        }
      }
    }

    // If the dictionary is not found, create it
    if(is_null(dict))
    {
      // Special case of P0: create a discontinuous space
      if(boost::ends_with(space_lib_name, "P0"))
      {
        dict = mesh().create_discontinuous_space(space_lib_name, space_lib_name).handle<Dictionary>();
      }
      else
      {
        dict = mesh().create_continuous_space(space_lib_name, space_lib_name).handle<Dictionary>();
      }
      dict->add_tag("ufem_dict");
      CFinfo << "Created ufem_dict " << dict->uri().path() << CFendl;
    }

    cf3_assert(is_not_null(dict));

    // We also tag the dict now, so the proto code can find it
    if(!dict->has_tag(tag))
      dict->add_tag(tag);

    Handle< Field > field = find_component_ptr_with_tag<Field>(*dict, tag);

    // Create the field
    field_manager().create_field(tag, *dict);
    field = find_component_ptr_with_tag<Field>(*dict, tag);
    cf3_assert(is_not_null(field));

    // Parallelize
    if(common::PE::Comm::instance().is_active())
    {
      CFdebug << "parallelizing field " << field->uri().path() << CFendl;
      field->parallelize_with(dict->comm_pattern());
    }
  }

  m_need_field_creation = false;
}

Handle< common::Action > Solver::add_solver(const std::string& builder_name, Component& parent)
{
  std::vector<std::string> builder_parts;
  boost::split(builder_parts, builder_name, boost::is_any_of("."));
  Handle< common::Action > result(parent.create_component(builder_parts.back(), builder_name));

  if(is_not_null(m_physics))
    result->configure_option_recursively(solver::Tags::physical_model(), m_physics);

  result->configure_option_recursively("initial_conditions", m_initial_conditions);

  return result;
}

Handle< Probe > Solver::add_probe ( const std::string& name, Component& parent, const Handle<Dictionary>& dict)
{
  Handle<Probe> probe = parent.create_component<Probe>(name);
  if(is_null(dict))
  {
    probe->options().set("dict", mesh().geometry_fields().handle<Dictionary>());
  }
  else
  {
    probe->options().set("dict", dict);
  }
  Handle<ProbePostProcessor> pp = probe->create_post_processor("Log","cf3.solver.actions.ProbePostProcHistory");
  pp->mark_basic();

  Handle<History> hist(get_child("History"));
  if(is_null(hist))
  {
    hist = create_component<History>("History");
    hist->options().set("dimension",1u);
    hist->mark_basic();
  }
  pp->options().set("history", hist);

  return probe;
}

void Solver::signal_add_probe ( SignalArgs& args )
{
  SignalOptions options(args);
  Handle<Dictionary> dict;
  if(options.check("dict"))
    dict = options.option("dict").value< Handle<Dictionary> >();

  const std::string name = options.option("name").value<std::string>();

  Handle<Component> parent = options.option("parent").value< Handle<Component> >();
  if(is_null(parent))
    throw common::SetupError(FromHere(), "Invalid parent component supplied when adding probe " + name);

  Handle<Probe> probe = add_probe(name, *parent, dict);

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", probe->uri());
}

void Solver::signature_add_probe ( SignalArgs& args )
{
  SignalOptions options(args);
  options.add("name", "SomeProbe").pretty_name("Name").description("Name of the probe to add").mark_basic();
  options.add("parent", Handle<Component>()).pretty_name("Parent").description("Component that will be the parent of the probe").mark_basic();
}



void Solver::execute()
{
  create_fields();
  solver::SimpleSolver::execute();
}


} // UFEM
} // cf3
