// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/Log.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/EventHandler.hpp"
#include "Common/CGroup.hpp"
#include "Common/Core.hpp"

#include "Mesh/CMesh.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/Variables.hpp"

#include "Solver/Actions/CSynchronizeFields.hpp"

#include "SFDM/SFDSolver.hpp"
#include "SFDM/IterativeSolver.hpp"
#include "SFDM/TimeStepping.hpp"

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Physics;
using namespace CF::Solver;
using namespace CF::Solver::Actions;


namespace CF {
namespace SFDM {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < SFDSolver, CSolver, LibSFDM > SFDSolver_Builder;

////////////////////////////////////////////////////////////////////////////////

SFDSolver::SFDSolver ( const std::string& name  ) : CSolver ( name )
{
  // properties

  properties()["brief"] = std::string("Spectral Finite Difference Method");
  properties()["description"] = std::string("Long description not available");

  // options
  options().add_option< OptionT<std::string> >( SFDM::Tags::solution_vars(), "")
      ->attach_trigger ( boost::bind ( &SFDSolver::config_physics, this ) );

  options().add_option( OptionT<Uint>::create("solution_order",2u) )
      ->pretty_name("Solution Order")
      ->description("Setting this will create the appropriate spaces in the mesh")
      ->attach_trigger( boost::bind( &SFDSolver::config_mesh , this ) );

  options().add_option(OptionComponent<CMesh>::create( Solver::Tags::mesh(), &m_mesh))
      ->description("Mesh the Discretization Method will be applied to")
      ->pretty_name("Mesh")
      ->attach_trigger ( boost::bind ( &SFDSolver::config_mesh,   this ) );

  option(Solver::Tags::physical_model()).attach_trigger ( boost::bind ( &SFDSolver::config_physics, this ) );

  // for storing links to fields
  m_fields  = create_static_component_ptr< CGroup >( SFDM::Tags::fields()  );

  // Shared actions by the solver
  m_actions = create_static_component_ptr< CGroup >( SFDM::Tags::actions() );

  // create the parallel synchronization action
  CSynchronizeFields& synchronize = m_actions->create_component<CSynchronizeFields>("Synchronize");

  // listen to mesh_updated events, emitted by the domain
  Core::instance().event_handler().connect_to_event("mesh_changed", this, &SFDSolver::on_mesh_changed_event);

  // for setting up the mesh
  m_prepare_mesh  = create_static_component_ptr< CActionDirector >( "SetupMesh" );

  m_time_stepping    = create_static_component_ptr< TimeStepping >( TimeStepping::type_name() );

  m_iterative_solver = create_static_component_ptr< IterativeSolver >( IterativeSolver::type_name() );
  m_time_stepping->append(m_iterative_solver);

  m_domain_discretization= create_static_component_ptr< DomainDiscretization > ( DomainDiscretization::type_name() );
  m_iterative_solver->pre_update().append(m_domain_discretization);
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
      child.configure_option_recursively( Solver::Tags::physical_model(), pm.uri() );
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

  CMesh& mesh = *(m_mesh.lock());

  Physics::PhysModel& pm = physics(); // physcial model must have already been configured

  if( pm.ndim() != mesh.dimension() )
    throw SetupError( FromHere(), "Dimensionality mismatch. Loaded mesh ndim " + to_str(mesh.dimension()) + " and physical model dimension " + to_str(pm.ndim()) );

  // setup the fields

  prepare_mesh().configure_option_recursively( Solver::Tags::mesh(), mesh.uri() ); // trigger config_mesh()

  prepare_mesh().execute();

  // configure all other subcomponents with the mesh

  boost_foreach( Component& child, children() )
  {
    child.configure_option_recursively( Solver::Tags::mesh(), mesh.uri() );
  }
}

////////////////////////////////////////////////////////////////////////////////

void SFDSolver::on_mesh_changed_event( SignalArgs& args )
{
  SignalOptions options( args );

  URI mesh_uri = options.value<URI>("mesh_uri");

  configure_option( Solver::Tags::mesh(), mesh_uri ); // trigger config_mesh()
}

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF
