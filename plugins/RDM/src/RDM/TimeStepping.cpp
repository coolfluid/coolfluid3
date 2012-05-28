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

#include "solver/Time.hpp"
#include "solver/actions/CriterionTime.hpp"
#include "solver/actions/CriterionMaxIterations.hpp"
#include "solver/actions/PeriodicWriteMesh.hpp"

#include "RDM/FaceTerm.hpp"

#include "TimeStepping.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::solver::actions;

namespace cf3 {
namespace RDM {


///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < TimeStepping, common::Action, LibRDM > TimeStepping_Builder;

///////////////////////////////////////////////////////////////////////////////////////

TimeStepping::TimeStepping ( const std::string& name ) :
  cf3::solver::ActionDirector(name)
{
  mark_basic();

  // properties

  properties().add( "iteration", Uint(0) );

  // static components

  m_time  = create_static_component<Time>("Time");

  m_pre_actions  = create_static_component<ActionDirector>("PreActions");

  m_post_actions = create_static_component<ActionDirector>("PostActions");

  post_actions().create_component<PeriodicWriteMesh>( "PeriodicWriter" );

  // dyanmic components
  create_component<CriterionMaxIterations>( "MaxIterations" );
  
  configure_option_recursively( "Time",    m_time );
}

bool TimeStepping::stop_condition()
{
  bool finish = false;
  boost_foreach(Criterion& stop_criterion, find_components<Criterion>(*this))
      finish |= stop_criterion();
  return finish;
}

void TimeStepping::execute()
{
  configure_option_recursively( "iterator", handle<Component>() );
  // start loop - iterations start from 1 ( max iter zero will do nothing )

  Uint k = 1;
  properties().property("iteration") = k;

  while( ! stop_condition() ) // time loop
  {

    // print iteration

    CFinfo << "time step [" << k << "] time [" << m_time->current_time() << "]" << CFendl;

    // (1) the pre actions - pre-process, user defined actions, etc

    m_pre_actions->execute();

    // (2) the registered actions that solve one time step

    ActionDirector::execute();

    // (3) the post actions - compute norm, post-process something, etc

    m_post_actions->execute();

    // raise event of time_step done

    raise_timestep_done();

    // advance time & iteration

    m_time->current_time() += m_time->dt();

    properties().property("iteration") = ++k; // update the iteration number

  }
}

void TimeStepping::raise_timestep_done()
{
  SignalOptions opts;

  opts.add( "time",  m_time->current_time() );
  opts.add( "dt",  m_time->dt() );
  opts.add( "iteration", properties().value<Uint>("iteration") );

  SignalFrame frame = opts.create_frame("timestep_done", uri(), URI());

  common::Core::instance().event_handler().raise_event( "timestep_done", frame);
}
///////////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3
