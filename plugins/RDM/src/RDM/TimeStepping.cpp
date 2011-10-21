// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/CBuilder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/EventHandler.hpp"

#include "common/XML/SignalOptions.hpp"

#include "Solver/CTime.hpp"
#include "Solver/Actions/CCriterionTime.hpp"
#include "Solver/Actions/CCriterionMaxIterations.hpp"
#include "Solver/Actions/CPeriodicWriteMesh.hpp"

#include "RDM/FaceTerm.hpp"

#include "TimeStepping.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::Solver;
using namespace cf3::Solver::Actions;

namespace cf3 {
namespace RDM {


///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < TimeStepping, CAction, LibRDM > TimeStepping_Builder;

///////////////////////////////////////////////////////////////////////////////////////

TimeStepping::TimeStepping ( const std::string& name ) :
  cf3::Solver::ActionDirector(name)
{
  mark_basic();

  // properties

  m_properties.add_property( "iteration", Uint(0) );

  // static components

  m_time  = create_static_component_ptr<CTime>("Time");

  m_pre_actions  = create_static_component_ptr<CActionDirector>("PreActions");

  m_post_actions = create_static_component_ptr<CActionDirector>("PostActions");

  CPeriodicWriteMesh& cwriter = post_actions().create_component<CPeriodicWriteMesh>( "PeriodicWriter" );
  post_actions().append( cwriter );

  // dyanmic components

  CCriterionMaxIterations& maxiter =
      create_component<CCriterionMaxIterations>( "MaxIterations" );
}

bool TimeStepping::stop_condition()
{
  bool finish = false;
  boost_foreach(CCriterion& stop_criterion, find_components<CCriterion>(*this))
      finish |= stop_criterion();
  return finish;
}

void TimeStepping::execute()
{
  /// @todo these configurations sould be in constructor but does not work there
  ///       becasue uri() is undefined on the constructor ( component is still free )

  configure_option_recursively( "ctime",    m_time->uri() );
  configure_option_recursively( "iterator", this->uri() );

  // start loop - iterations start from 1 ( max iter zero will do nothing )

  Uint k = 1;
  property("iteration") = k;

  while( ! stop_condition() ) // time loop
  {

    // print iteration

    CFinfo << "time step [" << k << "] time [" << m_time->current_time() << "]" << CFendl;

    // (1) the pre actions - pre-process, user defined actions, etc

    m_pre_actions->execute();

    // (2) the registered actions that solve one time step

    CActionDirector::execute();

    // (3) the post actions - compute norm, post-process something, etc

    m_post_actions->execute();

    // raise event of time_step done

    raise_timestep_done();

    // advance time & iteration

    m_time->current_time() += m_time->dt();

    property("iteration") = ++k; // update the iteration number

  }
}

void TimeStepping::raise_timestep_done()
{
  SignalOptions opts;

  opts.add_option< OptionT<Uint> >( "time",  m_time->current_time() );
  opts.add_option< OptionT<Uint> >( "dt",  m_time->dt() );
  opts.add_option< OptionT<Uint> >( "iteration", properties().value<Uint>("iteration") );

  SignalFrame frame = opts.create_frame("timestep_done", uri(), URI());

  common::Core::instance().event_handler().raise_event( "timestep_done", frame);
}
///////////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3
