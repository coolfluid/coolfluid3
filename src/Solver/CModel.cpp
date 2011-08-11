// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "Common/BoostFilesystem.hpp"

#include "Common/EventHandler.hpp"
#include "Common/Log.hpp"
#include "Common/CFactory.hpp"
#include "Common/CBuilder.hpp"
#include "Common/Signal.hpp"
#include "Common/Core.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/FindComponents.hpp"
#include "Common/CGroup.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CRegion.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/VariableManager.hpp"

#include "Solver/CSolver.hpp"
#include "Solver/CModel.hpp"
#include "Solver/Tags.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Common::XML;
using namespace Mesh;

Common::ComponentBuilder < CModel, Component, LibSolver > CModel_Builder;

////////////////////////////////////////////////////////////////////////////////

struct CModel::Implementation
{
  Implementation(Component& component) :
    m_component(component),
    m_tools(m_component.create_static_component<CGroup>("tools"))
  {
    m_tools.mark_basic();
  }

  Component& m_component;
  CGroup& m_tools;
  boost::weak_ptr<CDomain> m_domain;
  boost::weak_ptr<Physics::PhysModel> m_physics;
};

////////////////////////////////////////////////////////////////////////////////

CModel::CModel( const std::string& name  ) :
  Component ( name ),
  m_implementation(new Implementation(*this))
{
  mark_basic();

  // options

  std::string cwd = boost::filesystem::current_path().string();

   m_options.add_option< OptionURI >("WorkingDir", URI( cwd ) )
       ->description("Your working directory")
       ->mark_basic();

   m_options.add_option< OptionURI >("ResultsDir", URI( cwd ) )
       ->description("Directory to store the output files")
       ->mark_basic();

   m_options.add_option< OptionT<Uint> >("CPUs", 1u )
       ->description("Number of cpus to use in simulation")
       ->mark_basic();

  // properties

  m_properties["steady"] = bool(true);

  // signals
  regist_signal( "create_physics" )
    ->connect( boost::bind( &CModel::signal_create_physics, this, _1 ) )
    ->description("Create the physical model")
    ->pretty_name("Create Physics")
    ->signature( boost::bind ( &CModel::signature_create_physics, this, _1) );

  regist_signal( "create_domain" )
    ->connect( boost::bind( &CModel::signal_create_domain, this, _1 ) )
    ->description("Create the domain and load a mesh")
    ->pretty_name("Create Domain")
    ->signature( boost::bind ( &CModel::signature_create_domain, this, _1) );

  regist_signal( "create_solver" )
    ->connect( boost::bind( &CModel::signal_create_solver, this, _1 ) )
    ->description("Create the solver")
    ->pretty_name("Create Solver")
    ->signature( boost::bind ( &CModel::signature_create_solver, this, _1) );

  regist_signal( "simulate" )
    ->connect( boost::bind( &CModel::signal_simulate, this, _1 ) )
    ->description("Simulates this model")
    ->pretty_name("Simulate");

  regist_signal ( "setup" )
    ->connect( boost::bind ( &CModel::signal_setup, this, _1 ) )
    ->description( "Set up the model using a specific solver" )
    ->pretty_name( "Setup" )
    ->signature( boost::bind ( &CModel::signature_setup, this, _1 ) );

  // Listen to mesh_updated events, emitted by the domain
  Core::instance().event_handler().connect_to_event("mesh_loaded", this, &CModel::on_mesh_loaded_event);
}

CModel::~CModel() {}

////////////////////////////////////////////////////////////////////////////////

void CModel::simulate()
{
  CFinfo << "\n" << name() << ": start simulation" << CFendl;

  // call all the solvers
  boost_foreach(CSolver& solver, find_components<CSolver>(*this))
  {
    solver.execute();
  }

  CFinfo << name() << ": end simulation\n" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

Physics::PhysModel& CModel::physics()
{
  return find_component<Physics::PhysModel>(*this);
}

////////////////////////////////////////////////////////////////////////////////

CDomain& CModel::domain()
{
  return find_component<CDomain>(*this);
}

////////////////////////////////////////////////////////////////////////////////

CSolver& CModel::solver()
{
  return find_component<CSolver>(*this);
}

////////////////////////////////////////////////////////////////////////////////

CGroup& CModel::tools()
{
  return m_implementation->m_tools;
}

////////////////////////////////////////////////////////////////////////////////

Physics::PhysModel& CModel::create_physics( const std::string& builder )
{
  std::string pm_name = CBuilder::extract_reduced_name(builder);

  Physics::PhysModel::Ptr pm = boost::algorithm::contains( builder, "." ) ?
        build_component_abstract_type< Physics::PhysModel >( builder, pm_name ) :
        build_component_abstract_type_reduced< Physics::PhysModel >( builder, pm_name );

  add_component(pm);
  m_implementation->m_physics = pm;

  configure_option_recursively(Tags::physical_model(), pm->uri());

  return *pm;
}

////////////////////////////////////////////////////////////////////////////////

CDomain& CModel::create_domain( const std::string& name )
{
  CDomain::Ptr dom = create_component_ptr<CDomain>( name );
  m_implementation->m_domain = dom;

  return *dom;
}

////////////////////////////////////////////////////////////////////////////////

CSolver& CModel::create_solver( const std::string& builder)
{
  std::string solver_name = CBuilder::extract_reduced_name(builder);

  CSolver::Ptr solver = boost::algorithm::contains( builder, "." ) ?
              build_component_abstract_type< Solver::CSolver >( builder, solver_name ) :
              build_component_abstract_type_reduced< Solver::CSolver >( builder, solver_name );

  add_component(solver);

  if(!m_implementation->m_physics.expired())
    solver->configure_option_recursively(Tags::physical_model(), m_implementation->m_physics.lock()->uri());

  return *solver;
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signature_create_physics ( Common::SignalArgs& node )
{
  SignalOptions options( node );

  CFactory::Ptr pm_factory = Core::instance().factories().get_factory<Physics::PhysModel>();
  std::vector<boost::any> pms;

  // build the restricted list
  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *pm_factory ) )
  {
    pms.push_back(bdr.name());
  }

  // create de value and add the restricted list
  options.add_option< OptionT<std::string> >( "builder", std::string() )
      ->description("Choose solver")
      ->restricted_list() = pms;
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signal_create_physics ( Common::SignalArgs& node )
{
  SignalOptions options( node );
  std::string builder = options.value<std::string>( "builder" );
  create_physics( builder );
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signature_create_domain ( Common::SignalArgs& node )
{
  // no signature parameters needed
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signal_create_domain ( Common::SignalArgs& node )
{
  SignalFrame& options = node.map( Protocol::Tags::key_options() );

  CDomain& domain = create_domain("Domain"); // dispatch to virtual function
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signature_create_solver ( Common::SignalArgs& node )
{
  SignalOptions options( node );

  CFactory::Ptr solver_factory = Core::instance().factories().get_factory<CSolver>();
  std::vector<boost::any> solvers;

  // build the restricted list
  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *solver_factory ) )
  {
    solvers.push_back(bdr.name());
  }

  // create de value and add the restricted list
  options.add_option< OptionT<std::string> >( "builder", std::string() )
      ->description("Choose solver")
      ->restricted_list() = solvers;
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signal_create_solver ( Common::SignalArgs& node )
{
  SignalOptions options( node );
  std::string builder_name = options.value<std::string>( "builder" );
  create_solver(builder_name);
}

////////////////////////////////////////////////////////////////////////////////

void CModel::setup(const std::string& solver_builder_name, const std::string& physics_builder_name)
{
  create_domain("Domain");
  create_solver(solver_builder_name);
  create_physics(physics_builder_name);
}

void CModel::signature_setup(SignalArgs& node)
{
  SignalOptions options( node );

  options.add_option< OptionT<std::string> >("solver_builder")
    ->pretty_name("Solver Builder")
    ->description("Builder name");

  options.add_option< OptionT<std::string> >("physics_builder")
    ->pretty_name("Physics Builder")
    ->description("Builder name for the physics");
}

void CModel::signal_setup(SignalArgs& node)
{
  SignalOptions options( node );
  const std::string solver_builder_name = options.value<std::string>( "solver_builder" );
  const std::string physics_builder_name = options.value<std::string>( "physics_builder" );
  setup(solver_builder_name, physics_builder_name);
}


////////////////////////////////////////////////////////////////////////////////

void CModel::signal_simulate ( Common::SignalArgs& node )
{
  this->simulate(); // dispatch to virtual function
}

////////////////////////////////////////////////////////////////////////////////

void CModel::on_mesh_loaded_event(SignalArgs& args)
{
  // If we have no domain, the event can't be for us
  if(m_implementation->m_domain.expired())
    return;

  SignalOptions options(args);

  URI mesh_uri = options.value<URI>("mesh_uri");

  if(mesh_uri.base_path() == domain().uri()) // Only handle events coming from our own domain
  {
    // Get a reference to the mesh that changed
    CMesh& mesh = access_component(mesh_uri).as_type<CMesh>();

    // Inform the solvers of the change
    boost_foreach(CSolver& solver, find_components<CSolver>(*this))
    {
      solver.mesh_loaded(mesh);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
