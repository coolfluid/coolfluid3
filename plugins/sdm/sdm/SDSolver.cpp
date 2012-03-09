// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/EventHandler.hpp"
#include "common/Group.hpp"
#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionArray.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"

//#include "RiemannSolvers/RiemannSolvers/RiemannSolver.hpp"

#include "solver/Time.hpp"
#include "solver/actions/SynchronizeFields.hpp"
#include "solver/actions/ComputeLNorm.hpp"

#include "sdm/Tags.hpp"
#include "sdm/SDSolver.hpp"
#include "sdm/IterativeSolver.hpp"
#include "sdm/RungeKuttaLowStorage2.hpp"
#include "sdm/TimeStepping.hpp"
#include "sdm/ComputeUpdateCoefficient.hpp"
#include "sdm/ElementCaching.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::physics;
using namespace cf3::solver;
using namespace cf3::solver::actions;


namespace cf3 {
namespace sdm {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < SDSolver, Solver, LibSDM > SDSolver_Builder;

////////////////////////////////////////////////////////////////////////////////

SDSolver::SDSolver ( const std::string& name  ) :
  Solver ( name ),
  m_mesh_configured(false)
{
  // properties

  properties()["brief"] = std::string("Spectral Finite Difference Method");
  properties()["description"] = std::string("Long description not available");

  // options
  options().add_option( sdm::Tags::solution_vars(), "")
      .attach_trigger ( boost::bind ( &SDSolver::config_physics, this ) )
      .mark_basic();

  options().add_option( sdm::Tags::solution_order(),2u )
      .pretty_name("Solution Order")
      .description("Setting this will create the appropriate spaces in the mesh")
      .mark_basic();

  options().add_option(sdm::Tags::mesh(), m_mesh)
      .description("Mesh the Discretization Method will be applied to")
      .pretty_name("Mesh")
      .attach_trigger ( boost::bind ( &SDSolver::config_mesh,   this ) )
      .mark_basic()
      .link_to(&m_mesh);

  options().add_option(sdm::Tags::regions(), std::vector<URI>(1,mesh::Tags::topology()))
      .description("The regions this solver will be applied to (if relative URI, it is relative to mesh/topology)")
      .pretty_name("Regions")
      .attach_trigger ( boost::bind ( &SDSolver::config_regions,   this ) )
      .mark_basic();

  options().add_option("iterative_solver",("cf3.sdm.RungeKuttaLowStorage2"))
      .description("Iterative solver to solve for the solution each time step")
      .pretty_name("Iterative Solver")
      .attach_trigger( boost::bind ( &SDSolver::config_iterative_solver, this ))
      .mark_basic();

  m_time = create_component<Time>("Time");
  options().add_option(Tags::time(), m_time)
      .description("Time tracking component")
      .pretty_name("Time")
      .attach_trigger( boost::bind( &SDSolver::config_time, this) )
      .mark_basic();

 options().option(sdm::Tags::physical_model()).attach_trigger ( boost::bind ( &SDSolver::config_physics, this ) );

  m_shared_caches = create_component<SharedCaches>(Tags::shared_caches());

  // Shared actions by the solver
  m_actions = create_static_component< Group >( sdm::Tags::actions() );
  ComputeLNorm& L2norm = *m_actions->create_static_component<ComputeLNorm>(sdm::Tags::L2norm());
  L2norm.options().configure_option("order",2u);
  L2norm.options().configure_option("scale",true);
  L2norm.options().configure_option("field",URI("../../FieldManager/")/Tags::residual());
  ComputeUpdateCoefficient& compute_update_coefficient = *m_actions->create_static_component<ComputeUpdateCoefficient>("compute_update_coefficient");

  // listen to mesh_updated events, emitted by the domain
  Core::instance().event_handler().connect_to_event("mesh_changed", this, &SDSolver::on_mesh_changed_event);

  // for setting up the mesh
  m_prepare_mesh  = create_static_component< PrepareMesh >( PrepareMesh::type_name() );
  m_initial_conditions  = create_static_component< InitialConditions >( InitialConditions::type_name() );

  m_time_stepping    = create_static_component< TimeStepping >( TimeStepping::type_name() );
  m_time_stepping->options().configure_option(Tags::time(),m_time);

  Handle< Action > conditional( m_time_stepping->post_actions().create_component("Periodic", "cf3.solver.actions.Conditional") );
  conditional->create_component("time_step","cf3.solver.actions.CriterionMilestoneTime");
  conditional->create_component("write_mesh","cf3.mesh.WriteMesh");
  m_time_stepping->post_actions().add_link(L2norm);

  m_boundary_conditions= create_static_component< BoundaryConditions > ( BoundaryConditions::type_name() );
  m_domain_discretization= create_static_component< DomainDiscretization > ( DomainDiscretization::type_name() );

  config_iterative_solver();
}

////////////////////////////////////////////////////////////////////////////////

SDSolver::~SDSolver()
{
}

////////////////////////////////////////////////////////////////////////////////

void SDSolver::execute()
{
  // Perform boundary condition before solving, to be sure it is applied correctly
  m_boundary_conditions->execute();
  // Start time stepping
  m_time_stepping->execute();
  Component& solution_space = *mesh().get_child("solution_space");
  boost_foreach(mesh::Field& field,  find_components_recursively<Field>(solution_space))
    field.synchronize();
}

////////////////////////////////////////////////////////////////////////////////

void SDSolver::config_iterative_solver()
{
  if ( is_not_null(m_iterative_solver) )
  {
    m_time_stepping->remove_component(*m_iterative_solver);
  }
  m_iterative_solver = m_time_stepping->create_component("IterativeSolver",options().option("iterative_solver").value_str())->handle<IterativeSolver>();
  m_iterative_solver->pre_update().add_link(*m_domain_discretization);
  m_iterative_solver->post_update().add_link(*m_boundary_conditions);
}

////////////////////////////////////////////////////////////////////////////////

void SDSolver::config_time()
{
  Handle<Time> new_time_component = options().option(Tags::time()).value< Handle<Time> >();
  if (is_null(new_time_component))
    throw SetupError(FromHere(),"Time is not setup correctly");
  if (new_time_component != m_time)
  {
    if (Handle<Component> owned_child = get_child(m_time->name()))
      if (owned_child == m_time->handle<Component>())
        remove_component(*m_time);
    m_time = new_time_component;
    m_time_stepping->options().configure_option(solver::Tags::time(),m_time);
  }

}

////////////////////////////////////////////////////////////////////////////////

void SDSolver::config_physics()
{
  try
  {
    PhysModel& pm = physics();

    std::string user_vars = options().option( sdm::Tags::solution_vars() ).value<std::string>();
    if( user_vars.empty() )
      return;

    Handle< Component > solution_vars =
        find_component_ptr_with_tag(pm, sdm::Tags::solution_vars());

    if( is_not_null(solution_vars) ) // if exits, ensure it is the good one
    {
      Variables& vars = *solution_vars->handle<Variables>();

      if( vars.type() != user_vars )
      {
        pm.remove_component(vars);
        solution_vars.reset();
      }
    }

    if( is_null(solution_vars) )
    {
      solution_vars = pm.create_variables( user_vars, sdm::Tags::solution_vars() )->handle<Component>();
      solution_vars->add_tag(sdm::Tags::solution_vars());
    }

    boost_foreach( Component& child, *this )
    {
      child.configure_option_recursively( sdm::Tags::physical_model(), pm.handle<Component>() );
      child.configure_option_recursively( sdm::Tags::solver(), handle<Component>() );
    }
  }
  catch(SetupError&)
  {
    // Do nothing if physmodel is not configured
  }
}

////////////////////////////////////////////////////////////////////////////////

void SDSolver::config_regions()
{
  if ( is_null(m_mesh) )
    throw SetupError(FromHere(), "First configure the mesh");
  m_regions.clear();
  const std::vector<URI> regions = options().option("regions").value< std::vector<URI> >();
  boost_foreach(const URI& region_uri, regions)
  {
    Handle<Component> comp = m_mesh->access_component(region_uri);

    if ( Handle< Region > region = comp->handle<Region>() )
      m_regions.push_back( region );
    else
      throw common::ValueNotFound ( FromHere(),
                           "Could not find region with path [" + region_uri.path() +"]" );
  }
}

////////////////////////////////////////////////////////////////////////////////

void SDSolver::config_mesh()
{
  if( is_null(m_mesh) ) return;

  physics::PhysModel& pm = physics(); // physcial model must have already been configured
  mesh::Mesh& mesh = *m_mesh;

  if( physics().ndim() != mesh.dimension() )
    throw SetupError( FromHere(), "Dimensionality mismatch. Loaded mesh ndim " + to_str(mesh.dimension()) + " and physical model dimension " + to_str(pm.ndim()) );

  boost_foreach( Component& child, *this)
  {
    child.configure_option_recursively( sdm::Tags::mesh(), m_mesh );
    child.configure_option_recursively( sdm::Tags::solver(), handle<Component>() );
  }
}

////////////////////////////////////////////////////////////////////////////////

void SDSolver::on_mesh_changed_event( SignalArgs& args )
{
  if( is_null(m_mesh) ) return;

  SignalOptions options( args );

  URI mesh_uri = options.value<URI>("mesh_uri");


  if (mesh().uri().string() != mesh_uri.string())
    return;

  // Carefully see what needs to be changed!!!
  throw NotSupported(FromHere(),"Mesh may not be changed once configured!!! (yet)");

  //  options().configure_option( sdm::Tags::mesh(), mesh_uri ); // trigger config_mesh()
}

////////////////////////////////////////////////////////////////////////////////

//void SDSolver::build_riemann_solver()
//{
//  if (is_not_null(m_riemann_solver))
//    remove_component(*m_riemann_solver);
//  m_riemann_solver = create_component("riemann_solver",options().option("riemann_solver").value<std::string>())->handle<RiemannSolvers::RiemannSolver>();
//  m_riemann_solver->options().configure_option("physical_model",physics().handle<PhysModel>());
//}

/////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3
