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

#include "mesh/Mesh.hpp"
#include "mesh/FieldManager.hpp"

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"

#include "RiemannSolvers/RiemannSolvers/RiemannSolver.hpp"

#include "solver/actions/CSynchronizeFields.hpp"
#include "solver/actions/CComputeLNorm.hpp"

#include "SFDM/Tags.hpp"
#include "SFDM/SFDSolver.hpp"
#include "SFDM/IterativeSolver.hpp"
#include "SFDM/TimeStepping.hpp"
#include "SFDM/ComputeUpdateCoefficient.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::physics;
using namespace cf3::solver;
using namespace cf3::solver::actions;


namespace cf3 {
namespace SFDM {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < SFDSolver, CSolver, LibSFDM > SFDSolver_Builder;

////////////////////////////////////////////////////////////////////////////////

SFDSolver::SFDSolver ( const std::string& name  ) :
  CSolver ( name ),
  m_mesh_configured(false)
{
  // properties

  properties()["brief"] = std::string("Spectral Finite Difference Method");
  properties()["description"] = std::string("Long description not available");

  // options
  options().add_option( SFDM::Tags::solution_vars(), "")
      .attach_trigger ( boost::bind ( &SFDSolver::config_physics, this ) )
      .mark_basic();

  options().add_option( SFDM::Tags::solution_order(),2u )
      .pretty_name("Solution Order")
      .description("Setting this will create the appropriate spaces in the mesh")
      .mark_basic();

  options().add_option(SFDM::Tags::mesh(), m_mesh)
      .description("Mesh the Discretization Method will be applied to")
      .pretty_name("Mesh")
      .attach_trigger ( boost::bind ( &SFDSolver::config_mesh,   this ) )
      .mark_basic()
      .link_to(&m_mesh);

  options().add_option("riemann_solver", "cf3.RiemannSolvers.Roe")
    .description("The component to solve the Rieman Problem on cell-faces")
    .pretty_name("Riemann Solver")
    .mark_basic()
    .attach_trigger ( boost::bind ( &SFDSolver::build_riemann_solver, this) );

  options().option(SFDM::Tags::physical_model()).attach_trigger ( boost::bind ( &SFDSolver::config_physics, this ) );

  // Shared actions by the solver
  m_actions = create_static_component< Group >( SFDM::Tags::actions() );
  CComputeLNorm& L2norm = *m_actions->create_static_component<CComputeLNorm>(SFDM::Tags::L2norm());
  L2norm.options().configure_option("order",2u);
  L2norm.options().configure_option("scale",true);
  L2norm.options().configure_option("field",URI("../../FieldManager/")/Tags::residual());

  // create the parallel synchronization action
  CSynchronizeFields& synchronize = *m_actions->create_component<CSynchronizeFields>("Synchronize");

  // listen to mesh_updated events, emitted by the domain
  Core::instance().event_handler().connect_to_event("mesh_changed", this, &SFDSolver::on_mesh_changed_event);

  // for setting up the mesh
  m_prepare_mesh  = create_static_component< PrepareMesh >( PrepareMesh::type_name() );
  m_initial_conditions  = create_static_component< InitialConditions >( InitialConditions::type_name() );

  m_time_stepping    = create_static_component< TimeStepping >( TimeStepping::type_name() );

  m_iterative_solver = m_time_stepping->create_component< IterativeSolver >( IterativeSolver::type_name() );

  Handle< Action > conditional( m_time_stepping->post_actions().create_component("Periodic", "cf3.solver.actions.Conditional") );
  conditional->create_component("milestone_dt","cf3.solver.actions.CCriterionMilestoneTime");
  conditional->create_component("write_mesh","cf3.mesh.WriteMesh");
  m_time_stepping->post_actions().add_link(L2norm);

  m_domain_discretization= create_static_component< DomainDiscretization > ( DomainDiscretization::type_name() );
  m_iterative_solver->pre_update().add_link(*m_domain_discretization);
  m_iterative_solver->pre_update().create_component<ComputeUpdateCoefficient>("compute_time_step");

  //  m_iterative_solver->pre_update().append(m_boundary_conditions);
  m_iterative_solver->post_update().add_link(synchronize);
}

////////////////////////////////////////////////////////////////////////////////

SFDSolver::~SFDSolver()
{
}

////////////////////////////////////////////////////////////////////////////////

void SFDSolver::execute()
{
  CFinfo << "Solving the amazing PDE's..."      << CFendl;
  m_time_stepping->execute();
  CFinfo << "Solving the amazing PDE's... done" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

void SFDSolver::config_physics()
{
  try
  {
    PhysModel& pm = physics();

    std::string user_vars = options().option( SFDM::Tags::solution_vars() ).value<std::string>();
    if( user_vars.empty() )
      return;

    Handle< Component > solution_vars =
        find_component_ptr_with_tag(pm, SFDM::Tags::solution_vars());

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
      solution_vars = pm.create_variables( user_vars, SFDM::Tags::solution_vars() )->handle<Component>();
      solution_vars->add_tag(SFDM::Tags::solution_vars());
    }

    boost_foreach( Component& child, *this )
    {
      child.configure_option_recursively( SFDM::Tags::physical_model(), pm.handle<Component>() );
      child.configure_option_recursively( SFDM::Tags::solver(), handle<Component>() );
    }

    build_riemann_solver();
  }
  catch(SetupError&)
  {
    // Do nothing if physmodel is not configured
  }
}

////////////////////////////////////////////////////////////////////////////////

void SFDSolver::config_mesh()
{
  if( is_null(m_mesh) ) return;

  physics::PhysModel& pm = physics(); // physcial model must have already been configured
  mesh::Mesh& mesh = *m_mesh;

  if( physics().ndim() != mesh.dimension() )
    throw SetupError( FromHere(), "Dimensionality mismatch. Loaded mesh ndim " + to_str(mesh.dimension()) + " and physical model dimension " + to_str(pm.ndim()) );

  boost_foreach( Component& child, *this)
  {
    child.configure_option_recursively( SFDM::Tags::mesh(), m_mesh );
    child.configure_option_recursively( SFDM::Tags::solver(), handle<Component>() );
  }
}

////////////////////////////////////////////////////////////////////////////////

void SFDSolver::on_mesh_changed_event( SignalArgs& args )
{
  if( is_null(m_mesh) ) return;

  SignalOptions options( args );

  URI mesh_uri = options.value<URI>("mesh_uri");


  if (mesh().uri().string() != mesh_uri.string())
    return;

  // Carefully see what needs to be changed!!!
  throw NotSupported(FromHere(),"Mesh may not be changed once configured!!! (yet)");

  //  options().configure_option( SFDM::Tags::mesh(), mesh_uri ); // trigger config_mesh()
}

////////////////////////////////////////////////////////////////////////////////

void SFDSolver::build_riemann_solver()
{
  if (is_not_null(m_riemann_solver))
    remove_component(*m_riemann_solver);
  m_riemann_solver = create_component("riemann_solver",options().option("riemann_solver").value<std::string>())->handle<RiemannSolvers::RiemannSolver>();
  m_riemann_solver->options().configure_option("physical_model",physics().handle<PhysModel>());
}

/////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3
