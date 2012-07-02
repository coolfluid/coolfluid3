// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/EventHandler.hpp"
#include "common/FindComponents.hpp"

#include "common/XML/SignalOptions.hpp"

#include "solver/actions/PeriodicWriteMesh.hpp"
#include "solver/actions/SynchronizeFields.hpp"
#include "solver/actions/CriterionMaxIterations.hpp"
#include "solver/actions/ComputeLNorm.hpp"
#include "solver/actions/PrintIterationSummary.hpp"

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

  properties().add( "iteration", Uint(0) );

  // static components

  m_pre_actions  = create_static_component<ActionDirector>("PreActions");

  m_update = create_static_component<ActionDirector>("Update");

  m_post_actions = create_static_component<ActionDirector>("PostActions");

  // dynamic components

  create_component<CriterionMaxIterations>( "MaxIterations" );

  ComputeLNorm& cnorm = *post_actions().create_component<ComputeLNorm>( "ComputeNorm" );
  post_actions().create_component<PrintIterationSummary>( "IterationSummary" );
  post_actions().create_component<PeriodicWriteMesh>( "PeriodicWriter" );

  cnorm.options().set("scale", true);
  cnorm.options().set("order", 2u);
}

bool IterativeSolver::stop_condition()
{
  bool finish = false;
  boost_foreach(Criterion& stop_criterion, find_components<Criterion>(*this))
      finish |= stop_criterion();
  return finish;
}


void IterativeSolver::execute()
{
  RDM::RDSolver& mysolver = *solver().handle< RDM::RDSolver >();

  /// @todo this configuration sould be in constructor but does not work there

  configure_option_recursively( "iterator", handle<Component>() );

  // access components (out of loop)

  ActionDirector& boundary_conditions =
      *access_component( "cpath:../BoundaryConditions" )->handle<ActionDirector>();

  ActionDirector& domain_discretization =
      *access_component( "cpath:../DomainDiscretization" )->handle<ActionDirector>();

  Action& synchronize = *mysolver.actions().get_child("Synchronize")->handle<Action>();

  Handle<Component> cnorm = post_actions().get_child("ComputeNorm");
  cnorm->options().set("table", follow_link(mysolver.fields().get_child( RDM::Tags::residual() ))->uri() );

  Component& cprint = *post_actions().get_child("IterationSummary");
  cprint.options().set("norm", cnorm );

  // iteration loop

  Uint iter = 1; // iterations start from 1 ( max iter zero will do nothing )
  properties().property("iteration") = iter;


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
    properties().property("iteration") = ++iter; // update the iteration number
  }
}

void IterativeSolver::raise_iteration_done()
{
  SignalOptions opts;
  const Uint iter = properties().value<Uint>("iteration");
  opts.add( "iteration", iter );
  SignalFrame frame = opts.create_frame("iteration_done", uri(), URI());

  common::Core::instance().event_handler().raise_event( "iteration_done", frame);
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
