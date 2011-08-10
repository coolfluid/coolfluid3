// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OSystem.hpp"
#include "Common/LibLoader.hpp"
#include "Common/EventHandler.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CMesh.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/Variables.hpp"

#include "Solver/Actions/CSynchronizeFields.hpp"

#include "RDM/Tags.hpp"
#include "RDM/InitialConditions.hpp"
#include "RDM/BoundaryConditions.hpp"
#include "RDM/DomainDiscretization.hpp"
#include "RDM/IterativeSolver.hpp"
#include "RDM/TimeStepping.hpp"
#include "RDM/RDSolver.hpp"
#include "RDM/SetupSingleSolution.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Physics;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

namespace CF {
namespace RDM {


////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < RDM::RDSolver, CSolver, LibRDM > Solver_Builder;

////////////////////////////////////////////////////////////////////////////////

RDSolver::RDSolver ( const std::string& name  ) :
  CSolver ( name )
{
  // properties

  properties()["brief"] = std::string("Residual Distribution Solver");
  properties()["description"] = std::string("Long description not available");

  // options

  m_options.add_option< OptionT<std::string> >( RDM::Tags::update_vars(), "")
      ->attach_trigger ( boost::bind ( &RDSolver::config_physics, this ) );

  m_options.add_option(OptionComponent<CMesh>::create( RDM::Tags::mesh(), &m_mesh))
      ->description("Mesh the Discretization Method will be applied to")
      ->pretty_name("Mesh")
      ->attach_trigger ( boost::bind ( &RDSolver::config_mesh,   this ) );

  option(RDM::Tags::physical_model()).attach_trigger ( boost::bind ( &RDSolver::config_physics, this ) );

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

  m_prepare_mesh =
      create_static_component_ptr< CActionDirector >( "SetupMesh" );

  // for storing links to fields

  m_fields  = create_static_component_ptr< CGroup >( RDM::Tags::fields()  );

  m_actions = create_static_component_ptr< CGroup >( RDM::Tags::actions() );

  // create the parallel synchronization action

  m_actions->create_component_ptr<CSynchronizeFields>("Synchronize");

  // listen to mesh_updated events, emitted by the domain

  Core::instance().event_handler().connect_to_event("mesh_changed", this, &RDSolver::on_mesh_changed_event);

}


RDSolver::~RDSolver() {}

InitialConditions&    RDSolver::initial_conditions()     { return *m_initial_conditions; }

BoundaryConditions&   RDSolver::boundary_conditions()    { return *m_boundary_conditions; }

DomainDiscretization& RDSolver::domain_discretization()  { return *m_domain_discretization; }

IterativeSolver&      RDSolver::iterative_solver()       { return *m_iterative_solver; }

TimeStepping&         RDSolver::time_stepping()          { return *m_time_stepping; }

CActionDirector&      RDSolver::prepare_mesh()           { return *m_prepare_mesh; }

Common::CGroup& RDSolver::actions() { return *m_actions; }

Common::CGroup& RDSolver::fields()  { return *m_fields; }



void RDSolver::execute()
{
  m_time_stepping->execute();
}


void RDSolver::config_physics()
{
  try
  {
    PhysModel& pm = physics();

    std::string user_vars = option(  RDM::Tags::update_vars() ).value<std::string>();
    if( user_vars.empty() )
      return;

    Component::Ptr upv =
        find_component_ptr_with_tag(pm, RDM::Tags::update_vars());

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
      pm.create_variables( user_vars, RDM::Tags::update_vars() );

    boost_foreach( Component& comp, find_components(*this) )
      comp.configure_option_recursively( RDM::Tags::physical_model(), pm.uri() );

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
  if( is_null(m_mesh.lock()) ) return;

  CMesh& mesh = *(m_mesh.lock());

  // ensure physcial model has already been configured
  physics();

  // setup the fields

  prepare_mesh().configure_option_recursively( RDM::Tags::mesh(), mesh.uri() ); // trigger config_mesh()

  prepare_mesh().execute();

  // configure all other subcomponents with the mesh

  boost_foreach( Component& comp, find_components(*this) )
    comp.configure_option_recursively( RDM::Tags::mesh(), mesh.uri() );
}

void RDSolver::on_mesh_changed_event( SignalArgs& args )
{
  SignalOptions options( args );

  URI mesh_uri = options.value<URI>("mesh_uri");

  configure_option( RDM::Tags::mesh(), mesh_uri ); // trigger config_mesh()
}


////////////////////////////////////////////////////////////////////////////////


} // RDM
} // CF
