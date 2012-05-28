// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OSystem.hpp"
#include "common/LibLoader.hpp"
#include "common/EventHandler.hpp"
#include "common/FindComponents.hpp"
#include "common/XML/SignalOptions.hpp"

#include "mesh/Mesh.hpp"

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"

#include "solver/actions/SynchronizeFields.hpp"

#include "RDM/Tags.hpp"
#include "RDM/InitialConditions.hpp"
#include "RDM/BoundaryConditions.hpp"
#include "RDM/DomainDiscretization.hpp"
#include "RDM/IterativeSolver.hpp"
#include "RDM/TimeStepping.hpp"
#include "RDM/RDSolver.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::physics;
using namespace cf3::solver;
using namespace cf3::solver::actions;

namespace cf3 {
namespace RDM {


////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < RDM::RDSolver, Solver, LibRDM > solver_Builder;

////////////////////////////////////////////////////////////////////////////////

RDSolver::RDSolver ( const std::string& name  ) :
  Solver ( name )
{
  switch_to_sol=true;

  // properties

  properties()["brief"] = std::string("Residual Distribution Solver");
  properties()["description"] = std::string("Long description not available");

  // options

  options().add( RDM::Tags::update_vars(), "")
      .attach_trigger ( boost::bind ( &RDSolver::config_physics, this ) );

  options().add( "solution_space", RDM::Tags::solution() )
      .pretty_name("Solution Space")
      .attach_trigger ( boost::bind ( &RDSolver::config_mesh,   this ) );

  options().add(RDM::Tags::mesh(), m_mesh)
      .description("Mesh the Discretization Method will be applied to")
      .pretty_name("Mesh")
      .attach_trigger ( boost::bind ( &RDSolver::config_mesh,   this ) )
      .link_to(&m_mesh);

  options().option(RDM::Tags::physical_model()).attach_trigger ( boost::bind ( &RDSolver::config_physics, this ) );

  // subcomponents

  m_initial_conditions =
      create_static_component< InitialConditions >( InitialConditions::type_name() );

  m_boundary_conditions =
      create_static_component< BoundaryConditions >( BoundaryConditions::type_name() );

  m_domain_discretization =
      create_static_component< DomainDiscretization >( DomainDiscretization::type_name() );

  m_iterative_solver =
      create_static_component< IterativeSolver >( IterativeSolver::type_name() );

  m_time_stepping =
      create_static_component< TimeStepping >( TimeStepping::type_name() );

  m_time_stepping->add_link( *m_iterative_solver );

  m_prepare_mesh =
      create_static_component< common::ActionDirector >( "SetupMesh" );

  // for storing links to fields

  m_fields  = create_static_component< Group >( RDM::Tags::fields()  );

  m_actions = create_static_component< Group >( RDM::Tags::actions() );

  // create the parallel synchronization action

  m_actions->create_component<SynchronizeFields>("Synchronize");

  // listen to mesh_updated events, emitted by the domain

  Core::instance().event_handler().connect_to_event("mesh_changed", this, &RDSolver::on_mesh_changed_event);

}


RDSolver::~RDSolver() {}

InitialConditions&    RDSolver::initial_conditions()     { return *m_initial_conditions; }

BoundaryConditions&   RDSolver::boundary_conditions()    { return *m_boundary_conditions; }

DomainDiscretization& RDSolver::domain_discretization()  { return *m_domain_discretization; }

IterativeSolver&      RDSolver::iterative_solver()       { return *m_iterative_solver; }

TimeStepping&         RDSolver::time_stepping()          { return *m_time_stepping; }

common::ActionDirector&      RDSolver::prepare_mesh()           { return *m_prepare_mesh; }

common::Group& RDSolver::actions() { return *m_actions; }

common::Group& RDSolver::fields()  { return *m_fields; }



void RDSolver::execute()
{
  m_time_stepping->execute();
}


void RDSolver::config_physics()
{
  try
  {
    PhysModel& pm = physics();

    std::string user_vars = options().option(  RDM::Tags::update_vars() ).value<std::string>();
    if( user_vars.empty() )
      return;

    Handle< Variables > upv(find_component_ptr_with_tag(pm, RDM::Tags::update_vars()));

    if( is_not_null(upv) ) // if exits insure is the good one
    {
      if( upv->type() != user_vars )
      {
        pm.remove_component(upv->name() );
        upv.reset();
      }
    }

    if( is_null(upv) )
      pm.create_variables( user_vars, RDM::Tags::update_vars() );

    boost_foreach( Component& comp, find_components(*this) )
      comp.configure_option_recursively( RDM::Tags::physical_model(), pm.handle<PhysModel>() );

    // load the library which has the correct RDM physics

    std::string modeltype = pm.model_type();
    boost::to_lower( modeltype );
    OSystem::instance().lib_loader()->load_library( "coolfluid_rdm_" + modeltype );
  }
  catch(SetupError&)
  {
    // Do nothing if physmodel is not configured
  }
}


void RDSolver::config_mesh()
{
  if( is_null(m_mesh) ) return;

  Mesh& mesh = *(m_mesh);

  physics::PhysModel& pm = physics(); // physcial model must have already been configured

  if( pm.ndim() != mesh.dimension() )
    throw SetupError( FromHere(), "Dimensionality mismatch. Loaded mesh ndim " + to_str(mesh.dimension()) + " and physical model dimension " + to_str(pm.ndim()) );

  // setup the fields
  prepare_mesh().configure_option_recursively( RDM::Tags::mesh(), m_mesh ); // trigger config_mesh()

  prepare_mesh().execute();

  // configure all other subcomponents with the mesh

  boost_foreach( Component& comp, find_components(*this) )
    comp.configure_option_recursively( RDM::Tags::mesh(), m_mesh );
}

void RDSolver::on_mesh_changed_event( SignalArgs& args )
{
  SignalOptions options( args );

  Handle<Mesh> mesh( access_component(options.value<URI>("mesh_uri")) );

  this->options().set( RDM::Tags::mesh(), mesh ); // trigger config_mesh()
}


////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3
