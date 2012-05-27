// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "common/BoostFilesystem.hpp"

#include "common/EventHandler.hpp"
#include "common/Log.hpp"
#include "common/Factory.hpp"
#include "common/Builder.hpp"
#include "common/Signal.hpp"
#include "common/Core.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/FindComponents.hpp"
#include "common/Group.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

/// @todo remove when ready
#include "mesh/Space.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Region.hpp"

#include "physics/PhysModel.hpp"

#include "solver/Solver.hpp"
#include "solver/Model.hpp"
#include "solver/Tags.hpp"

namespace cf3 {
namespace solver {

using namespace common;
using namespace common::XML;
using namespace mesh;

common::ComponentBuilder < Model, Model, LibSolver > Model_Builder;

////////////////////////////////////////////////////////////////////////////////

struct Model::Implementation
{
  Implementation(Component& component) :
    m_component(component),
    m_tools(*m_component.create_static_component<Group>("tools"))
  {
    m_tools.mark_basic();
  }

  Component& m_component;
  Group& m_tools;
  Handle<Domain> m_domain;
  Handle<physics::PhysModel> m_physics;
};

////////////////////////////////////////////////////////////////////////////////

Model::Model( const std::string& name  ) :
  Component ( name ),
  m_implementation(new Implementation(*this))
{
  mark_basic();

  // options

  std::string cwd = boost::filesystem::current_path().string();

   options().add("WorkingDir", URI( cwd ) )
       .description("Your working directory")
       .mark_basic();

   options().add("ResultsDir", URI( cwd ) )
       .description("Directory to store the output files")
       .mark_basic();

   options().add("CPUs", 1u )
       .description("Number of cpus to use in simulation")
       .mark_basic();

  // properties

  properties()["steady"] = bool(true);

  // signals
  regist_signal( "create_physics" )
    .connect( boost::bind( &Model::signal_create_physics, this, _1 ) )
    .description("Create the physical model")
    .pretty_name("Create Physics")
    .signature( boost::bind ( &Model::signature_create_physics, this, _1) );

  regist_signal( "create_domain" )
    .connect( boost::bind( &Model::signal_create_domain, this, _1 ) )
    .description("Create the domain and load a mesh")
    .pretty_name("Create Domain")
    .signature( boost::bind ( &Model::signature_create_domain, this, _1) );

  regist_signal( "create_solver" )
    .connect( boost::bind( &Model::signal_create_solver, this, _1 ) )
    .description("Create the solver")
    .pretty_name("Create Solver")
    .signature( boost::bind ( &Model::signature_create_solver, this, _1) );

  regist_signal( "simulate" )
    .connect( boost::bind( &Model::signal_simulate, this, _1 ) )
    .description("Simulates this model")
    .pretty_name("Simulate");

  regist_signal ( "setup" )
    .connect( boost::bind ( &Model::signal_setup, this, _1 ) )
    .description( "Set up the model using a specific solver" )
    .pretty_name( "Setup" )
    .signature( boost::bind ( &Model::signature_setup, this, _1 ) );

  // Listen to mesh_updated events, emitted by the domain
  Core::instance().event_handler().connect_to_event(mesh::Tags::event_mesh_loaded(), this, &Model::on_mesh_loaded_event);
  Core::instance().event_handler().connect_to_event(mesh::Tags::event_mesh_changed(), this, &Model::on_mesh_changed_event);
}

Model::~Model() {}

////////////////////////////////////////////////////////////////////////////////

void Model::simulate()
{
  CFinfo << "\n" << name() << ": start simulation" << CFendl;

  // call all the solvers
//  try
  {
    boost_foreach(Solver& solver, find_components<Solver>(*this))
    {
      solver.execute();
    }
    CFinfo << name() << ": end simulation\n" << CFendl;
  }
//  catch (common::FailedToConverge& e)
//  {
//    CFerror << "simulation failed\n" << e.what() << CFendl;
//  }

}

////////////////////////////////////////////////////////////////////////////////

physics::PhysModel& Model::physics()
{
  return find_component<physics::PhysModel>(*this);
}

////////////////////////////////////////////////////////////////////////////////

Domain& Model::domain()
{
  return find_component<Domain>(*this);
}

////////////////////////////////////////////////////////////////////////////////

Solver& Model::solver()
{
  return find_component<Solver>(*this);
}

////////////////////////////////////////////////////////////////////////////////

Group& Model::tools()
{
  return m_implementation->m_tools;
}

////////////////////////////////////////////////////////////////////////////////

physics::PhysModel& Model::create_physics( const std::string& builder )
{
  std::string pm_name = Builder::extract_reduced_name(builder);

  boost::shared_ptr< physics::PhysModel > pm = boost::algorithm::contains( builder, "." ) ?
        build_component_abstract_type< physics::PhysModel >( builder, pm_name ) :
        build_component_abstract_type_reduced< physics::PhysModel >( builder, pm_name );

  add_component(pm);
  m_implementation->m_physics = Handle<physics::PhysModel>(pm);

  configure_option_recursively(Tags::physical_model(), m_implementation->m_physics);

  return *pm;
}

////////////////////////////////////////////////////////////////////////////////

Domain& Model::create_domain( const std::string& name )
{
  Handle<Domain> dom = create_component<Domain>( name );
  m_implementation->m_domain = dom;

  return *dom;
}

////////////////////////////////////////////////////////////////////////////////

Solver& Model::create_solver( const std::string& builder)
{
  std::string solver_name = Builder::extract_reduced_name(builder);

  boost::shared_ptr< Solver > solver = boost::algorithm::contains( builder, "." ) ?
              build_component_abstract_type< solver::Solver >( builder, solver_name ) :
              build_component_abstract_type_reduced< solver::Solver >( builder, solver_name );

  add_component(solver);

  if(is_not_null(m_implementation->m_physics))
    solver->configure_option_recursively(Tags::physical_model(), m_implementation->m_physics);

  return *solver;
}

////////////////////////////////////////////////////////////////////////////////

void Model::signature_create_physics ( common::SignalArgs& node )
{
  SignalOptions options( node );

  Handle< Factory > pm_factory = Core::instance().factories().get_factory<physics::PhysModel>();
  std::vector<boost::any> pms;

  // build the restricted list
  boost_foreach(Builder& bdr, find_components_recursively<Builder>( *pm_factory ) )
  {
    pms.push_back(bdr.name());
  }

  // create de value and add the restricted list
  options.add( "builder", std::string() )
      .description("Choose physical model")
      .restricted_list() = pms;
}

////////////////////////////////////////////////////////////////////////////////

void Model::signal_create_physics ( common::SignalArgs& node )
{
  SignalOptions options( node );
  std::string builder = options.value<std::string>( "builder" );
  physics::PhysModel& phys_model = create_physics( builder );

  SignalFrame reply = node.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", phys_model.uri());
}

////////////////////////////////////////////////////////////////////////////////

void Model::signature_create_domain ( common::SignalArgs& node )
{
  // no signature parameters needed
}

////////////////////////////////////////////////////////////////////////////////

void Model::signal_create_domain ( common::SignalArgs& node )
{
  SignalFrame& options = node.map( Protocol::Tags::key_options() );

  Domain& domain = create_domain("Domain"); // dispatch to virtual function

  SignalFrame reply = node.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", domain.uri());
}

////////////////////////////////////////////////////////////////////////////////

void Model::signature_create_solver ( common::SignalArgs& node )
{
  SignalOptions options( node );

  Handle< Factory > solver_factory = Core::instance().factories().get_factory<Solver>();
  std::vector<boost::any> solvers;

  // build the restricted list
  boost_foreach(Builder& bdr, find_components_recursively<Builder>( *solver_factory ) )
  {
    solvers.push_back(bdr.name());
  }

  // create de value and add the restricted list
  options.add( "builder", std::string() )
      .description("Choose solver")
      .restricted_list() = solvers;
}

////////////////////////////////////////////////////////////////////////////////

void Model::signal_create_solver ( common::SignalArgs& node )
{
  SignalOptions options( node );
  std::string builder_name = options.value<std::string>( "builder" );
  Solver& solver = create_solver(builder_name);

  SignalFrame reply = node.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", solver.uri());
}

////////////////////////////////////////////////////////////////////////////////

void Model::setup(const std::string& solver_builder_name, const std::string& physics_builder_name)
{
  create_domain("Domain");
  create_solver(solver_builder_name);
  create_physics(physics_builder_name);
}

void Model::signature_setup(SignalArgs& node)
{
  SignalOptions options( node );

  options.add<std::string>("solver_builder")
    .pretty_name("Solver Builder")
    .description("Builder name");

  options.add<std::string>("physics_builder")
    .pretty_name("Physics Builder")
    .description("Builder name for the physics");
}

void Model::signal_setup(SignalArgs& node)
{
  SignalOptions options( node );
  const std::string solver_builder_name = options.value<std::string>( "solver_builder" );
  const std::string physics_builder_name = options.value<std::string>( "physics_builder" );
  setup(solver_builder_name, physics_builder_name);
}


////////////////////////////////////////////////////////////////////////////////

void Model::signal_simulate ( common::SignalArgs& node )
{
  this->simulate(); // dispatch to virtual function
}

////////////////////////////////////////////////////////////////////////////////

void Model::on_mesh_loaded_event(SignalArgs& args)
{
  // If we have no domain, the event can't be for us
  if(is_null(m_implementation->m_domain))
    return;

  SignalOptions options(args);

  URI mesh_uri = options.value<URI>("mesh_uri");

  if(mesh_uri.base_path() == domain().uri()) // Only handle events coming from our own domain
  {
    // Get a reference to the mesh that changed
    Handle<Mesh> mesh(access_component(mesh_uri));

    // Inform the solvers of the change
    boost_foreach(Solver& solver, find_components<Solver>(*this))
    {
      solver.mesh_loaded(*mesh);
    }
  }
}

void Model::on_mesh_changed_event(SignalArgs& args)
{
  // If we have no domain, the event can't be for us
  if(is_null(m_implementation->m_domain))
    return;

  SignalOptions options(args);

  URI mesh_uri = options.value<URI>("mesh_uri");

  if(mesh_uri.base_path() == domain().uri()) // Only handle events coming from our own domain
  {
    // Get a reference to the mesh that changed
    Handle<Mesh> mesh(access_component(mesh_uri));

    // Inform the solvers of the change
    boost_foreach(Solver& solver, find_components<Solver>(*this))
    {
      solver.mesh_changed(*mesh);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
