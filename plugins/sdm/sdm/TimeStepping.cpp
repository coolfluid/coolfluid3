// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/OptionComponent.hpp"
#include "common/PropertyList.hpp"
#include "common/EventHandler.hpp"
#include "common/FindComponents.hpp"
#include "common/Group.hpp"
#include "common/XML/SignalOptions.hpp"

#include "math/Consts.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/MeshMetadata.hpp"

#include "solver/Time.hpp"
#include "solver/actions/CriterionTime.hpp"
#include "solver/actions/CriterionMaxIterations.hpp"
#include "solver/actions/PeriodicWriteMesh.hpp"

#include "sdm/TimeStepping.hpp"
#include "sdm/Tags.hpp"
#include "sdm/SDSolver.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::solver::actions;

namespace cf3 {
namespace sdm {


///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < TimeStepping, common::Action, LibSDM > TimeStepping_Builder;

///////////////////////////////////////////////////////////////////////////////////////

TimeStepping::TimeStepping ( const std::string& name ) :
  cf3::solver::ActionDirector(name)
{
  mark_basic();

 // properties

  properties().add_property( "iteration", Uint(0) );

  options().add_option(Tags::time(), m_time)
      .description("Time tracking component")
      .pretty_name("Time")
      .link_to(&m_time);

  options().add_option("max_iteration",math::Consts::uint_max()).mark_basic();
  options().add_option("cfl",std::string("1.0")).attach_trigger( boost::bind( &TimeStepping::parse_cfl, this)).mark_basic();
  parse_cfl();
  options().add_option("time_accurate",true).mark_basic();

  // static components

  m_pre_actions  = create_static_component<ActionDirector>("PreActions");

  m_post_actions = create_static_component<ActionDirector>("PostActions");

  post_actions().create_component<PeriodicWriteMesh>( "PeriodicWriter" );

}

void TimeStepping::parse_cfl()
{
  m_cfl.Parse(options().option("cfl").value_str(),"i,t");

  if ( m_cfl.GetParseErrorType() !=  FunctionParser::FP_NO_ERROR )
  {
    std::string msg("ParseError in parsing cfl number: ");
    msg += " Error [" +std::string(m_cfl.ErrorMsg()) + "]";
    msg += " Function [" + options().option("cfl").value_str() + "]";
    msg += " Vars: [ i, t ]";
    throw common::ParsingFailed (FromHere(),msg);
  }

}

bool TimeStepping::stop_condition()
{
  Uint nb_criteria = 0;
  bool finish = false;
  boost_foreach(Criterion& stop_criterion, find_components<Criterion>(*this))
  {
    finish |= stop_criterion();
    ++nb_criteria;
  }

  if (options().option("time_accurate").value<bool>())
  {
    if (m_time->current_time() + 1e-12 > m_time->end_time())
      return true;
  }

  if (m_time->iter() >= options().option("max_iteration").value<Uint>())
    return true; // stop

  return finish;
}

void TimeStepping::execute()
{
  configure_option_recursively( sdm::Tags::time(),    m_time);
  configure_option_recursively( "iterator", handle<Component>() );
  // start loop - iterations start from 1 ( max iter zero will do nothing )

  properties().property("iteration") = m_time->iter();

  while( ! stop_condition() ) // time loop
  {
    // (1) the pre actions - pre-process, user defined actions, etc

    solver().handle<SDSolver>()->actions().get_child("compute_update_coefficient")->options().configure_option(sdm::Tags::time(),m_time);
    solver().handle<SDSolver>()->actions().get_child("compute_update_coefficient")->options().configure_option("time_accurate",options().option("time_accurate").value<bool>());
    std::vector<Real> vars(2);
    vars[0] = properties().value<Uint>("iteration");
    vars[1] = m_time->current_time();
    Real cfl = m_cfl.Eval(&vars[0]);
    solver().handle<SDSolver>()->actions().get_child("compute_update_coefficient")->options().configure_option("cfl",cfl);

    m_pre_actions->execute();

    // (2) the registered actions that solve one time step

    ActionDirector::execute();

    // advance time & iteration

    m_time->current_time() += m_time->dt();
    ++m_time->iter();
    properties().property("iteration") = m_time->iter(); // update the iteration number

    mesh().metadata()["iter"] = m_time->iter();
    mesh().metadata()["time"] = m_time->current_time();

    // (3) the post actions - compute norm, post-process something, etc

    m_post_actions->execute();

    // raise event of time_step done

    //raise_timestep_done();

    Real norm = boost::any_cast<Real>(solver().handle<SDSolver>()->actions().get_child(Tags::L2norm())->properties().property("norm"));

    if (options().option("time_accurate").value<bool>())
      CFinfo << "iter [" << std::setw(4) << m_time->iter() << "]  cfl [" << std::setw(12) << cfl<< "]  time [" << std::setw(12) << std::scientific << m_time->current_time() << "]  dt ["<< std::scientific << std::setw(12) << m_time->dt()<<"]  L2(rhs) ["<< std::scientific <<std::setw(12) << norm<<"]" << CFendl;
    else
      CFinfo << "iter [" << std::setw(4) << m_time->iter() << "]  cfl [" << std::setw(12) << cfl<< "]  L2(rhs) [" << std::scientific << std::setw(12) << norm << "]" << CFendl;


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


} // sdm
} // cf3
