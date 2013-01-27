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
#include "common/Timer.hpp"
#include "common/XML/SignalOptions.hpp"

#include "math/Consts.hpp"

#include "mesh/Mesh.hpp"

#include "solver/Time.hpp"
#include "solver/History.hpp"
#include "solver/Criterion.hpp"

#include "solver/TimeStepping.hpp"
#include "solver/Tags.hpp"

#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::solver;

namespace cf3 {
namespace solver {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < TimeStepping, common::Action, LibSolver > TimeStepping_Builder;

///////////////////////////////////////////////////////////////////////////////////////

TimeStepping::TimeStepping ( const std::string& name ) :
  common::ActionDirector(name)
{
  mark_basic();

 // properties

  properties().add("finished",true);
  properties().add("cputime", 0. );

  options().add("walltime",0.)
      .description("Wall time to start with");
  options().add("step",0u)
      .mark_basic()
      .description("Step to start with");
  options().add("time",0.)
      .mark_basic()
      .description("Time to start with");
  options().add("time_step",0.)
      .mark_basic()
      .attach_trigger( boost::bind( &TimeStepping::config_time_step, this) );
  options().add("end_time",0.)
      .mark_basic()
      .attach_trigger( boost::bind( &TimeStepping::config_end_time, this) );

  options().add("max_steps",math::Consts::uint_max());
  options().add("time_accurate",true).mark_basic();

  // static components

  m_pre_actions  = create_static_component<ActionDirector>("pre_actions");

  m_post_actions = create_static_component<ActionDirector>("post_actions");

  m_history      = create_static_component<History>("history");
  history()->options().set("file",URI("file:timestepping.tsv"));
  history()->options().set("dimension",3u);
  
  // Set a few variables in history. More can be added during run-time
  // following the same way.
  history()->set("step",0);
  history()->set("time",0.);
  history()->set("time_step",0.);
  history()->set("walltime",0.);
  history()->set("cputime",0.);
  history()->set("memory",0.);

  std::vector<std::string> disabled_actions;
  disabled_actions.push_back("pre_actions");
  disabled_actions.push_back("post_actions");
  options().set("disabled_actions",disabled_actions);

  regist_signal("do_step")
      .description("Do a time step")
      .pretty_name("Do Step")
      .connect( boost::bind( &TimeStepping::signal_do_step, this, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

void TimeStepping::config_end_time()
{
  Real end_time  = options().value<Real>("end_time");
  Real time_step = options().value<Real>("time_step");
  if (time_step == 0.)
    time_step = end_time;

  boost_foreach( const Handle<Time>& time_comp, m_times)
  {
    if (is_not_null(time_comp)) time_comp->options().set("end_time",end_time);
  }
  options().set("time_step",time_step);
  properties()["finished"] = stop_condition();
}

////////////////////////////////////////////////////////////////////////////////

void TimeStepping::config_time_step()
{
  Real time_step = options().value<Real>("time_step");
  boost_foreach( const Handle<Time>& time_comp, m_times)
  {
    if (is_not_null(time_comp)) time_comp->options().set("time_step",options().value<Real>("time_step"));
  }
}

////////////////////////////////////////////////////////////////////////////////

void TimeStepping::add_time( const Handle<solver::Time>& time )
{
  m_times.push_back(time);
}

///////////////////////////////////////////////////////////////////////////////////////

bool TimeStepping::finished()
{
  return properties().value<bool>("finished");
}

bool TimeStepping::not_finished()
{
  return ! finished();
}

////////////////////////////////////////////////////////////////////////////////

bool TimeStepping::stop_condition()
{
  Uint nb_criteria = 0;
  bool finish = false;
  boost_foreach(Criterion& stop_criterion, find_components<Criterion>(*this))
  {
    finish |= stop_criterion();
    ++nb_criteria;
  }

  if (options().value<bool>("time_accurate"))
  {
    Real time = options().value<Real>("time");
    Real end_time = options().value<Real>("end_time");
    if (time >= end_time)
      return true; // stop
  }

  Uint step = options().value<Uint>("step");
  Uint max_steps = options().value<Uint>("max_steps");
  if (step >= max_steps)
    return true; // stop

  return finish;
}

///////////////////////////////////////////////////////////////////////////////////////

void TimeStepping::execute()
{
  while( ! stop_condition() ) // time loop
  {
    do_step();
  }
  history()->flush();
}

////////////////////////////////////////////////////////////////////////////////

void TimeStepping::do_step()
{
  // Prepare step
  common::Timer timer;
  Real walltime = options().value<Real>("walltime");
  Real cputime;
  Real memory;
  Uint step = options().value<Uint>("step");
  Real time = options().value<Real>("time");
  Real time_step = options().value<Real>("time_step");
  time_step = std::min(time_step,options().value<Real>("end_time")-time);

  // Configure end_time of this step
  boost_foreach( const Handle<Time>& time_comp, m_times)
  {
    if (is_not_null(time_comp)) time_comp->options().set("end_time",time + time_step);
  }
  /// (1) the pre actions - pre-process, user defined actions, etc

  m_pre_actions->execute();

  /// (2) the registered actions that solve one time step

  ActionDirector::execute();

  /// (3) advance time & iteration

  ++step;
  time += time_step;
  options().set("time",time);
  options().set("step",step);

  /// (4) the post actions - compute norm, post-process something, etc

  m_post_actions->execute();

  /// (5) raise event of time_step done

  raise_timestep_done();

  /// (6) Statistics
  memory = common::OSystem::instance().layer()->memory_usage()/1024/1024; // in MB
  cputime = timer.elapsed();
  walltime += cputime;
  options().set("walltime",walltime);
  properties()["cputime"] = cputime;

  /// (7) Write history
  CFinfo << "Writing history" << CFendl;

  history()->set("step",step);
  history()->set("time",time);
  history()->set("time_step",time_step);
  history()->set("walltime",walltime);
  history()->set("cputime",cputime);
  history()->set("memory",memory);
  
    CFinfo << "Saving entry" << CFendl;
  history()->save_entry();

  /// (8) Output info
      CFinfo << "output summary" << CFendl;
  CFinfo << history()->entry().summary() << CFendl;
//  if (options().value<bool>("time_accurate"))
//    CFinfo << "step [" << std::setw(4) << step << "]  "
//           << "time [" << std::setw(12) << std::scientific << m_time->current_time() << "]  "
//           << "dt ["<< std::scientific << std::setw(12) << time_step <<"]  "
//           << "walltime ["<< std::scientific << std::setw(12) << walltime<<"]  "
//           << "cputime ["<< std::scientific << std::setw(12) << cputime<<"]  "
//           << "memory ["<< std::setw(11) << common::OSystem::instance().layer()->memory_usage_str() <<"]  "
//           << CFendl;
//  else
//    CFinfo << "step [" << std::setw(4) << step << "]  "
//           << "walltime ["<< std::scientific << std::setw(11) << walltime << "]  "
//           << "cputime ["<< std::scientific << std::setw(12) << cputime<<"]  "
//           << "memory ["<< std::setw(12) << common::OSystem::instance().layer()->memory_usage_str() <<"]  "
//           << CFendl;

  properties()["finished"] = stop_condition();
}

///////////////////////////////////////////////////////////////////////////////////////

void TimeStepping::raise_timestep_done()
{
  SignalOptions opts;

  opts.add( "time",  options().value<Real>("time") );
  opts.add( "time_step", options().value<Real>("time_step") );
  opts.add( "step", options().value<Uint>("step") );

  SignalFrame frame = opts.create_frame("timestep_done", uri(), URI());

  common::Core::instance().event_handler().raise_event( "timestep_done", frame);
}

////////////////////////////////////////////////////////////////////////////////

void TimeStepping::signal_do_step( common::SignalArgs& args)
{
  do_step();
}

///////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
