// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/EventHandler.hpp"
#include "common/FindComponents.hpp"

#include "common/XML/SignalOptions.hpp"

#include "solver/actions/CPeriodicWriteMesh.hpp"
#include "solver/actions/CSynchronizeFields.hpp"
#include "solver/actions/CCriterionMaxIterations.hpp"
#include "solver/actions/CComputeLNorm.hpp"
#include "solver/actions/CPrintIterationSummary.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/Reset.hpp"

#include "IterativeSolver.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::solver::actions;

namespace cf3 {
namespace RDM {


///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < IterativeSolver, common::Action, LibRDM > IterativeSolver_Builder;

///////////////////////////////////////////////////////////////////////////////////////

IterativeSolver::IterativeSolver ( const std::string& name ) :
  cf3::solver::ActionDirector(name)
{
  mark_basic();

  // properties

  m_properties.add_property( "iteration", Uint(0) );

  // static components

  m_pre_actions  = create_static_component_ptr<ActionDirector>("PreActions");

  m_update = create_static_component_ptr<ActionDirector>("Update");

  m_post_actions = create_static_component_ptr<ActionDirector>("PostActions");

  // dynamic components

  create_component<CCriterionMaxIterations>( "MaxIterations" );

  CComputeLNorm& cnorm = post_actions().create_component<CComputeLNorm>( "ComputeNorm" );
  post_actions().append( cnorm );

  CPrintIterationSummary& cprint = post_actions().create_component<CPrintIterationSummary>( "IterationSummary" );
  post_actions().append( cprint );

  CPeriodicWriteMesh& cwriter = post_actions().create_component<CPeriodicWriteMesh>( "PeriodicWriter" );
  post_actions().append( cwriter );

  cnorm.configure_option("Scale", true);
  cnorm.configure_option("Order", 2u);
}

bool IterativeSolver::stop_condition()
{
  bool finish = false;
  boost_foreach(CCriterion& stop_criterion, find_components<CCriterion>(*this))
      finish |= stop_criterion();
  return finish;
}


void IterativeSolver::execute()
{
  RDM::RDSolver& mysolver = solver().as_type< RDM::RDSolver >();

  /// @todo this configuration sould be in constructor but does not work there

  configure_option_recursively( "iterator", this->uri() );

  // access components (out of loop)

  ActionDirector& boundary_conditions =
      access_component( "cpath:../BoundaryConditions" ).as_type<ActionDirector>();

  ActionDirector& domain_discretization =
      access_component( "cpath:../DomainDiscretization" ).as_type<ActionDirector>();

  Action& synchronize = mysolver.actions().get_child("Synchronize").as_type<Action>();

  Component& cnorm = post_actions().get_child("ComputeNorm");
  cnorm.configure_option("Field", mysolver.fields().get_child( RDM::Tags::residual() ).follow()->uri() );

  Component& cprint = post_actions().get_child("IterationSummary");
  cprint.configure_option("norm", cnorm.uri() );

  // iteration loop

  Uint iter = 1; // iterations start from 1 ( max iter zero will do nothing )
  property("iteration") = iter;


  while( ! stop_condition() ) // non-linear loop
  {
    // (1) the pre actions - cleanup residual, pre-process something, etc

    pre_actions().execute();

    // (2) domain discretization

    domain_discretization.execute();

    // (3) apply boundary conditions

    boundary_conditions.execute();

    // (4) update

    update().execute();

    // (5) update

    synchronize.execute();

    // (6) the post actions - compute norm, post-process something, etc

    post_actions().execute();

    // raise signal that iteration is done

    raise_iteration_done();

    // increment iteration

    property("iteration") = ++iter; // update the iteration number

  }
}

void IterativeSolver::raise_iteration_done()
{
  SignalOptions opts;
  const Uint iter = properties().value<Uint>("iteration");
  opts.add_option< OptionT<Uint> >( "iteration", iter );
  SignalFrame frame = opts.create_frame("iteration_done", uri(), URI());

  common::Core::instance().event_handler().raise_event( "iteration_done", frame);
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
