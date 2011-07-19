// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionComponent.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/Variables.hpp"

#include "Solver/Actions/CSynchronizeFields.hpp"

#include "RDM/Core/InitialConditions.hpp"
#include "RDM/Core/BoundaryConditions.hpp"
#include "RDM/Core/DomainDiscretization.hpp"
#include "RDM/Core/IterativeSolver.hpp"
#include "RDM/Core/TimeStepping.hpp"
#include "RDM/Core/Solver.hpp"

using namespace CF::Common;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::Physics;

namespace CF {
namespace RDM {
namespace Core {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < RDM::Core::Solver, CSolver, LibCore > Solver_Builder;

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

  m_synchronize =
      create_static_component_ptr<CSynchronizeFields>("Synchronize");

}


Solver::~Solver() {}


void Solver::execute()
{
  CFinfo << "[RDM] solver" << CFendl;

  m_time_stepping->execute();
}

void Solver::config_physics()
{
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
}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // RDM
} // CF
