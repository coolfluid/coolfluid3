// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/CMesh.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/Variables.hpp"

#include "Solver/Actions/CSynchronizeFields.hpp"

#include "RDM/Core/InitialConditions.hpp"
#include "RDM/Core/BoundaryConditions.hpp"
#include "RDM/Core/DomainDiscretization.hpp"
#include "RDM/Core/IterativeSolver.hpp"
#include "RDM/Core/TimeStepping.hpp"
#include "RDM/Core/Solver.hpp"
#include "RDM/Core/SetupFields.hpp"

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Physics;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

namespace CF {
namespace RDM {


////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < RDM::Solver, CSolver, LibRDM > Solver_Builder;

////////////////////////////////////////////////////////////////////////////////

Solver::Solver ( const std::string& name  ) :
  CSolver ( name )
{
  // options

  m_options.add_option< OptionT<std::string> >( Tags::update_vars(), "")
      ->attach_trigger ( boost::bind ( &Solver::config_physics, this ) );

  m_options.add_option( OptionComponent<Physics::PhysModel>::create("physical_model", &m_physical_model))
      ->set_description("Physical model to discretize")
      ->set_pretty_name("Physics")
      ->mark_basic()
      ->attach_trigger ( boost::bind ( &Solver::config_physics, this ) );

  // subcomponents

  m_initial_conditions =
      create_static_component_ptr< InitialConditions >( InitialConditions::type_name() );

  m_boundary_conditions =
      create_static_component_ptr< BoundaryConditions >( BoundaryConditions::type_name() );

  m_domain_discretization =
      create_static_component_ptr< DomainDiscretization >( DomainDiscretization::type_name() );

  m_iterative_solver =
      create_static_component_ptr< IterativeSolver >( IterativeSolver::type_name() );

  m_time_stepping =
      create_static_component_ptr< TimeStepping >( TimeStepping::type_name() );

  m_time_stepping->append( *m_iterative_solver );

  // for storing links to fields

  m_fields  = create_static_component_ptr< CGroup >( Tags::fields()  );

  m_actions = create_static_component_ptr< CGroup >( Tags::actions() );

  // create the parallel synchronization action

  m_actions->create_component_ptr<CSynchronizeFields>("Synchronize");

}


Solver::~Solver() {}


void Solver::execute()
{
  CFinfo << "[RDM] solver" << CFendl;

  m_time_stepping->execute();
}


void Solver::config_physics()
{
  configure_option_recursively( "solver", uri() );

  if( is_null(m_physical_model.lock()) )
    return;

  std::string user_vars = option(  Tags::update_vars() ).value<std::string>();
  if( user_vars.empty() )
    return;

  Physics::PhysModel& pm = * m_physical_model.lock();

  Component::Ptr upv =
      find_component_ptr_with_tag(pm, Tags::update_vars());

  if( is_not_null(upv) ) // if exits insure is the good one
  {
    Variables& vars = upv->as_type<Variables>();
    if( vars.type() != user_vars )
    {
      pm.remove_component(vars.name() );
      upv.reset();
    }
  }

  if( is_null(upv) )
    pm.create_variables( user_vars, Tags::update_vars() );

  configure_option_recursively( Tags::physical_model(), pm.uri() );
}


void Solver::config_mesh()
{
  configure_option_recursively( "solver", uri() );

  if( is_null(m_mesh.lock()) ) return;

  CMesh& mesh = *(m_mesh.lock());

  Physics::PhysModel::Ptr physmodel = m_physical_model.lock();
  if( is_null( physmodel ) )
    throw SetupError(FromHere(), "Physical model not yet set for RKRD component " + uri().string() );

  CAction& setup = create_component<SetupFields>("SetupFields");

  setup.configure_option( "mesh", mesh.uri() );
  setup.configure_option( "physical_model", physmodel->uri() );

  setup.execute();

  remove_component(setup);

  //--------------------------------------------------
#if 0
  std::vector<URI> cleanup_fields;
  cleanup_fields.push_back( m_residual.lock()->uri() );
  cleanup_fields.push_back( m_wave_speed.lock()->uri() );
  m_cleanup->configure_option("Fields", cleanup_fields);

  m_compute_norm->configure_option("Field", m_residual.lock()->uri());
#endif
  //--------------------------------------------------
}


////////////////////////////////////////////////////////////////////////////////


} // RDM
} // CF
