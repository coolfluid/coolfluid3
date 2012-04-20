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

#include "math/VariableManager.hpp"
#include "math/VariablesDescriptor.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"

#include "solver/Tags.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"

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
  SimpleSolver(name)
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

  regist_signal( "create_initial_conditions" )
    .connect( boost::bind( &Solver::signal_create_initial_conditions, this, _1 ) )
    .description("Create initial conditions.")
    .pretty_name("Create Initial Conditions");
}

Solver::~Solver()
{
}

Handle< common::Action > Solver::add_direct_solver(const std::string& builder_name)
{
  if(is_null(get_child("InitialConditions")))
  {
    create_component<InitialConditions>("InitialConditions");
  }

  std::vector<std::string> builder_parts;
  boost::split(builder_parts, builder_name, boost::is_any_of("."));
  Handle<common::Action> result(create_component(builder_parts.back(), builder_name));

  if(is_not_null(m_physics))
    configure_option_recursively(solver::Tags::physical_model(), m_physics);
  return result;
}

Handle< common::Action > Solver::add_unsteady_solver(const std::string& builder_name)
{
  if(is_null(get_child("InitialConditions")))
  {
    create_component<InitialConditions>("InitialConditions");
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

  std::vector<std::string> builder_parts;
  boost::split(builder_parts, builder_name, boost::is_any_of("."));
  Handle< common::Action > result(timeloop->create_component(builder_parts.back(), builder_name));

  timeloop->create_component("AdvanceTime", "cf3.solver.actions.AdvanceTime");

  if(is_not_null(m_physics))
    configure_option_recursively(solver::Tags::physical_model(), m_physics);
  return result;
}

Handle< common::ActionDirector > Solver::create_initial_conditions()
{
  Handle<common::ActionDirector> result(get_child("InitialConditions"));
  if(is_not_null(result))
  {
    CFwarn << "InitialConditions were created already, returning handle to previously created component" << CFendl;
    return result;
  }

  result = create_component<InitialConditions>("InitialConditions");
  if(is_not_null(m_physics))
    result->configure_option_recursively(solver::Tags::physical_model(), m_physics);

  return result;
}

void Solver::signature_add_solver(SignalArgs& args)
{
  SignalOptions options(args);
  options.add_option("builder_name", "")
    .pretty_name("Builder Names")
    .description("List the names of the builders that should be used to construct inner actions");
}

void Solver::signal_add_direct_solver(SignalArgs& args)
{
  SignalOptions options(args);
  Handle<common::Action> result = add_direct_solver(options.option("builder_name").value<std::string>());

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("created_component", result->uri());
}

void Solver::signal_add_unsteady_solver(SignalArgs& args)
{
  SignalOptions options(args);
  Handle<common::Action> result = add_unsteady_solver(options.option("builder_name").value<std::string>());

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("created_component", result->uri());
}

void Solver::signal_create_initial_conditions(SignalArgs& args)
{
  Handle<common::ActionDirector> ic = create_initial_conditions();

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("created_component", ic->uri());
}

void Solver::mesh_loaded(Mesh& mesh)
{
  SimpleSolver::mesh_loaded(mesh);
  mesh_changed(mesh);
}

void Solver::mesh_changed(Mesh& mesh)
{
  CFdebug << "UFEM::Solver: Reacting to mesh_changed signal" << CFendl;

  // Ensure the comm pattern will be updated
  if(is_not_null(mesh.geometry_fields().get_child("CommPattern")))
  {
    mesh.geometry_fields().remove_component("CommPattern");
  }

  // Find out what tags are used
  std::set<std::string> tags;
  BOOST_FOREACH(const ProtoAction& action, find_components_recursively<ProtoAction>(*this))
  {
    action.insert_tags(tags);
  }

  // Create fields as needed
  BOOST_FOREACH(const std::string& tag, tags)
  {
    Handle< Field > field = find_component_ptr_with_tag<Field>(mesh.geometry_fields(), tag);

    // If the field was created before, destroy it
    if(is_not_null(field))
    {
      CFdebug << "Removing existing field " << field->uri().string() << CFendl;
      field->parent()->remove_component(field->name());
      field.reset();
    }

    // Create the field
    field_manager().create_field(tag, mesh.geometry_fields());
    field = find_component_ptr_with_tag<Field>(mesh.geometry_fields(), tag);
    cf3_assert(is_not_null(field));

    // Parallelize
    if(common::PE::Comm::instance().is_active())
    {
      field->parallelize_with(mesh.geometry_fields().comm_pattern());
    }
  }

  // Set the region of all children to the root region of the mesh
  std::vector<URI> root_regions;
  root_regions.push_back(mesh.topology().uri());
  configure_option_recursively(solver::Tags::regions(), root_regions);
  configure_option_recursively("dictionary", mesh.geometry_fields().handle<Dictionary>());
}

} // UFEM
} // cf3
