// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"
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
  options().add_option< OptionT<std::string> >( SFDM::Tags::solution_vars(), "")
      ->attach_trigger ( boost::bind ( &SFDSolver::config_physics, this ) )
      ->mark_basic();

  options().add_option( OptionT<Uint>::create(SFDM::Tags::solution_order(),2u) )
      ->pretty_name("Solution Order")
      ->description("Setting this will create the appropriate spaces in the mesh")
      ->mark_basic();

  options().add_option(OptionComponent<Mesh>::create( SFDM::Tags::mesh(), &m_mesh))
      ->description("Mesh the Discretization Method will be applied to")
      ->pretty_name("Mesh")
      ->attach_trigger ( boost::bind ( &SFDSolver::config_mesh,   this ) )
      ->mark_basic();

  options().add_option( OptionT<std::string>::create("riemann_solver", "cf3.RiemannSolvers.Roe") )
    ->description("The component to solve the Rieman Problem on cell-faces")
    ->pretty_name("Riemann Solver")
    ->mark_basic()
    ->attach_trigger ( boost::bind ( &SFDSolver::build_riemann_solver, this) );

  option(SFDM::Tags::physical_model()).attach_trigger ( boost::bind ( &SFDSolver::config_physics, this ) );

  // Shared actions by the solver
  m_actions = create_static_component_ptr< Group >( SFDM::Tags::actions() );
  CComputeLNorm& L2norm = m_actions->create_static_component<CComputeLNorm>(SFDM::Tags::L2norm());
  L2norm.configure_option("order",2u);
  L2norm.configure_option("scale",true);
  L2norm.configure_option("field",URI("../../FieldManager/")/Tags::residual());

  // create the parallel synchronization action
  CSynchronizeFields& synchronize = m_actions->create_component<CSynchronizeFields>("Synchronize");

  // listen to mesh_updated events, emitted by the domain
  Core::instance().event_handler().connect_to_event("mesh_changed", this, &SFDSolver::on_mesh_changed_event);

  // for setting up the mesh
  m_prepare_mesh  = create_static_component_ptr< PrepareMesh >( PrepareMesh::type_name() );
  m_initial_conditions  = create_static_component_ptr< InitialConditions >( InitialConditions::type_name() );

  m_time_stepping    = create_static_component_ptr< TimeStepping >( TimeStepping::type_name() );

  m_iterative_solver = allocate_component< IterativeSolver >( IterativeSolver::type_name() );
  m_time_stepping->append(m_iterative_solver);

  Action::Ptr conditional ( build_component("cf3.solver.actions.Conditional","Periodic")->as_ptr<Action>() );
  m_time_stepping->post_actions().append(conditional);
  conditional->create_component("milestone_dt","cf3.solver.actions.CCriterionMilestoneTime");
  conditional->create_component("write_mesh","cf3.mesh.WriteMesh");
  m_time_stepping->post_actions().append(L2norm);

  m_domain_discretization= create_static_component_ptr< DomainDiscretization > ( DomainDiscretization::type_name() );
  m_iterative_solver->pre_update().append(m_domain_discretization);
  m_iterative_solver->pre_update().append(allocate_component<ComputeUpdateCoefficient>("compute_time_step"));

  //  m_iterative_solver->pre_update().append(m_boundary_conditions);
  m_iterative_solver->post_update().append(synchronize);
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

    std::string user_vars = option( SFDM::Tags::solution_vars() ).value<std::string>();
    if( user_vars.empty() )
      return;

    Component::Ptr solution_vars =
        find_component_ptr_with_tag(pm, SFDM::Tags::solution_vars());

    if( is_not_null(solution_vars) ) // if exits, ensure it is the good one
    {
      Variables& vars = solution_vars->as_type<Variables>();

      if( vars.type() != user_vars )
      {
        pm.remove_component(vars);
        solution_vars.reset();
      }
    }

    if( is_null(solution_vars) )
    {
      solution_vars = pm.create_variables( user_vars, SFDM::Tags::solution_vars() );
      solution_vars->add_tag(SFDM::Tags::solution_vars());
    }

    boost_foreach( Component& child, children() )
    {
      child.configure_option_recursively( SFDM::Tags::physical_model(), pm.uri() );
      child.configure_option_recursively( SFDM::Tags::solver(), this->uri() );
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
  if( is_null(m_mesh.lock()) ) return;

  physics::PhysModel& pm = physics(); // physcial model must have already been configured
  mesh::Mesh& mesh = *m_mesh.lock();

  if( physics().ndim() != mesh.dimension() )
    throw SetupError( FromHere(), "Dimensionality mismatch. Loaded mesh ndim " + to_str(mesh.dimension()) + " and physical model dimension " + to_str(pm.ndim()) );

  boost_foreach( Component& child, children() )
  {
    child.configure_option_recursively( SFDM::Tags::mesh(), mesh.uri() );
    child.configure_option_recursively( SFDM::Tags::solver(), this->uri() );
  }
}

////////////////////////////////////////////////////////////////////////////////

void SFDSolver::on_mesh_changed_event( SignalArgs& args )
{
  SignalOptions options( args );

  URI mesh_uri = options.value<URI>("mesh_uri");

  // Carefully see what needs to be changed!!!
  throw NotSupported(FromHere(),"Mesh may not be changed once configured!!! (yet)");

  //  configure_option( SFDM::Tags::mesh(), mesh_uri ); // trigger config_mesh()
}

////////////////////////////////////////////////////////////////////////////////

void SFDSolver::build_riemann_solver()
{
  if (is_not_null(m_riemann_solver))
    remove_component(*m_riemann_solver);
  m_riemann_solver = create_component("riemann_solver",option("riemann_solver").value<std::string>()).as_ptr<RiemannSolvers::RiemannSolver>();
  m_riemann_solver->configure_option("physical_model",physics().uri());
}

/////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3
