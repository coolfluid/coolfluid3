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
#include "common/Group.hpp"
#include "common/XML/SignalOptions.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/MeshMetadata.hpp"

#include "solver/CTime.hpp"
#include "solver/actions/CCriterionTime.hpp"
#include "solver/actions/CCriterionMaxIterations.hpp"
#include "solver/actions/CPeriodicWriteMesh.hpp"

#include "SFDM/TimeStepping.hpp"
#include "SFDM/Tags.hpp"
#include "SFDM/SFDSolver.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::solver::actions;

namespace cf3 {
namespace SFDM {


///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < TimeStepping, common::Action, LibSFDM > TimeStepping_Builder;

///////////////////////////////////////////////////////////////////////////////////////

TimeStepping::TimeStepping ( const std::string& name ) :
  cf3::solver::ActionDirector(name)
{
  mark_basic();

  // properties

  properties().add_property( "iteration", Uint(0) );

  // static components

  m_time  = create_static_component<CTime>("Time");

  m_pre_actions  = create_static_component<ActionDirector>("PreActions");

  m_post_actions = create_static_component<ActionDirector>("PostActions");

  post_actions().create_component<CPeriodicWriteMesh>( "PeriodicWriter" );

  // dyanmic components

//  CCriterionMaxIterations& maxiter =
//      create_component<CCriterionMaxIterations>( "MaxIterations" );
  create_component<CCriterionTime>( "EndTime" );
}

bool TimeStepping::stop_condition()
{
  Uint nb_criteria = 0;
  bool finish = false;
  boost_foreach(CCriterion& stop_criterion, find_components<CCriterion>(*this))
  {
    finish |= stop_criterion();
    ++nb_criteria;
  }
  if (nb_criteria == 0)
  {
    CFwarn << "No stop criteria available in [" << uri().string() << "]... exiting loop" << CFendl;
    return true; // stop
  }

//  if (properties().value<Uint>("iteration")>1)
//    return true; // stop

  return finish;
}

void TimeStepping::execute()
{
  configure_option_recursively( SFDM::Tags::time(),    m_time);
  configure_option_recursively( "iterator", handle<Component>() );
  // start loop - iterations start from 1 ( max iter zero will do nothing )

  Uint k = 1;
  properties().property("iteration") = k;

  while( ! stop_condition() ) // time loop
  {
    // (1) the pre actions - pre-process, user defined actions, etc

    m_pre_actions->execute();

    // (2) the registered actions that solve one time step

    ActionDirector::execute();

    // advance time & iteration

    m_time->current_time() += m_time->dt();

    properties().property("iteration") = ++k; // update the iteration number

    mesh().metadata()["iter"] = properties().property("iteration");
    mesh().metadata()["time"] = m_time->current_time();

    // (3) the post actions - compute norm, post-process something, etc

    m_post_actions->execute();

    // raise event of time_step done

    //raise_timestep_done();

    Real norm = boost::any_cast<Real>(solver().handle<SFDSolver>()->actions().get_child(Tags::L2norm())->properties().property("norm"));
    CFinfo << "iteration [" << k << "]  time [" << m_time->current_time() << "]  time step ["<<m_time->dt()<<"]  L2(rhs) ["<<norm<<"]" << CFendl;

  }
}

void TimeStepping::raise_timestep_done()
{
  SignalOptions opts;

  opts.add_option( "time",  m_time->current_time() );
  opts.add_option( "dt",    m_time->dt() );
  opts.add_option( "iteration", properties().value<Uint>("iteration") );

  SignalFrame frame = opts.create_frame("timestep_done", uri(), URI());

  common::Core::instance().event_handler().raise_event( "timestep_done", frame);
}
///////////////////////////////////////////////////////////////////////////////////////


} // SFDM
} // cf3
