// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include "common/Signal.hpp"
#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/EventHandler.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionComponent.hpp"
#include "common/ActionDirector.hpp"
#include "common/FindComponents.hpp"
#include "common/Group.hpp"
#include "common/Action.hpp"

#include "math/Consts.hpp"
#include "math/VariablesDescriptor.hpp"

#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

#include "solver/Criterion.hpp"
#include "solver/Time.hpp"
#include "solver/PDESolver.hpp"

#include "solver/PDESolver.hpp"
#include "solver/TimeStepComputer.hpp"

#include "solver/History.hpp"
#include "solver/Time.hpp"
#include "solver/PDE.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::math::Consts;

namespace cf3 {
namespace solver {

///////////////////////////////////////////////////////////////////////////////////////

PDESolver::PDESolver( const std::string& name ) :
  common::Action(name)
{
  // options
  options().add( "pde", m_pde )
      .link_to(&m_pde)
      .attach_trigger( boost::bind( &PDESolver::config_time_step_computer, this) )
      .attach_trigger( boost::bind( &PDESolver::config_history, this) )
      .mark_basic();

  options().add( "time_step_computer", std::string("cf3.solver.ImposeCFL") )
      .attach_trigger( boost::bind( &PDESolver::set_time_step_computer, this) )
      .mark_basic();
  set_time_step_computer();

  options().add( "print_iteration_summary", true);
  options().add( "max_iteration",math::Consts::uint_max()).mark_basic();
  options().add( "history", m_history).link_to(&m_history);
  options().add( "pre_iteration", m_pre_iteration).link_to(&m_pre_iteration);
  options().add( "post_iteration", m_post_iteration).link_to(&m_post_iteration);
  options().add( "pre_update", m_pre_update).link_to(&m_pre_update);
  options().add( "post_update", m_post_update).link_to(&m_post_update);

  regist_signal("solve_time_step")
      .description("Solve a time step")
      .pretty_name("Solve Time Step")
      .connect   ( boost::bind( &PDESolver::signal_solve_time_step,    this, _1 ) )
      .signature ( boost::bind( &PDESolver::signature_solve_time_step, this, _1 ) );

  regist_signal("solve_iterations")
      .description("Solve iterations")
      .pretty_name("Solve Iterations")
      .connect   ( boost::bind( &PDESolver::signal_solve_iterations,    this, _1 ) )
      .signature ( boost::bind( &PDESolver::signature_solve_iterations, this, _1 ) );


  m_history = create_static_component<History>("history");
}

////////////////////////////////////////////////////////////////////////////////

void PDESolver::config_history()
{
  m_history->options().set("file",URI("solve_"+m_pde->name()+".tsv"));
  m_history->options().set("dimension",m_pde->nb_dim());
}

////////////////////////////////////////////////////////////////////////////////

void PDESolver::set_time_step_computer()
{
  if (is_not_null(m_time_step_computer)
      && m_time_step_computer->derived_type_name() == options().value<std::string>("time_step_computer") )
  {
    return;
  }
  if (is_not_null(m_time_step_computer))  remove_component(*m_time_step_computer);

  m_time_step_computer = create_component("time_step_computer",options().value<std::string>("time_step_computer"))->handle<TimeStepComputer>();
  m_time_step_computer->mark_basic();

  config_time_step_computer();
}

////////////////////////////////////////////////////////////////////////////////

void PDESolver::config_time_step_computer()
{
  if (is_not_null(m_time_step_computer))
  {
    if ( is_not_null(m_pde) )
      m_time_step_computer->options().set("time",m_pde->time());
  }
}

////////////////////////////////////////////////////////////////////////////////

bool PDESolver::stop_condition()
{
  Uint nb_criteria = 0;
  bool finish = false;
  boost_foreach(Criterion& stop_criterion, find_components<Criterion>(*this))
  {
    finish |= stop_criterion();
    ++nb_criteria;
  }

  if ( m_pde->time() && m_time_step_computer->options().value<bool>("time_accurate") )
  {
    if (m_pde->time()->current_time() >= m_pde->time()->end_time())
      return true;
  }

  if (m_pde->time()->iter() >= options().value<Uint>("max_iteration"))
    return true; // stop

  return finish;
}

////////////////////////////////////////////////////////////////////////////////

void PDESolver::do_iteration()
{
  if (m_pre_iteration) m_pre_iteration->execute();

  step();
  m_pde->time()->current_time() += m_pde->time()->dt();
  ++m_pde->time()->iter();

  history()->set("iter",m_pde->time()->iter());
  if (m_time_step_computer->options().value<bool>("time_accurate"))
  {
    history()->set("time",m_pde->time()->current_time());
    history()->set("dt",m_pde->time()->dt());
  }

  if (m_post_iteration) m_post_iteration->execute();

  history()->save_entry();

//    CFinfo << "  " << iteration_summary() << CFendl;
  if ( options().value<bool>("print_iteration_summary") )
    CFinfo << "  " << history()->entry().summary() << CFendl;

  m_pde->time()->options().set( "current_time", m_pde->time()->current_time() );

}

////////////////////////////////////////////////////////////////////////////////

void PDESolver::solve_time_step(const Real time_step)
{
  if ( is_null(m_pde) ) throw SetupError(FromHere(), "PDE is not configured");
  if ( is_null(m_pde->time()) ) throw InvalidStructure(FromHere(), "PDE does not have time term");
  m_pde->time()->options().set("end_time",m_pde->time()->current_time()+time_step );
  execute();
}

////////////////////////////////////////////////////////////////////////////////

void PDESolver::solve_iterations(const Uint nb_iterations)
{
  setup();
  for (Uint iter=0; iter<nb_iterations; ++iter)
  {
    do_iteration();
  }
  if (m_history) history()->flush();
}

////////////////////////////////////////////////////////////////////////////////

void PDESolver::execute()
{
  setup();
  while( ! stop_condition() ) // time loop
  {
    do_iteration();
  }
  if (m_history) history()->flush();
}

////////////////////////////////////////////////////////////////////////////////

std::string PDESolver::iteration_summary()
{
  std::stringstream ss;
  CFinfo << "iter [" << std::setw(4) << m_pde->time()->iter() << "]  "
         << "time [" << std::setw(12) << std::scientific << m_pde->time()->current_time() << "]  "
         << "dt ["<< std::scientific << std::setw(12) << m_pde->time()->dt() <<"]  ";
  return ss.str();
}

void PDESolver::raise_iteration_done()
{
  SignalOptions opts;
  const Uint iter = properties().value<Uint>("iteration");
  opts.add( "iteration", iter );
  SignalFrame frame = opts.create_frame("iteration_done", uri(), URI());
  common::Core::instance().event_handler().raise_event( "iteration_done", frame);
}

////////////////////////////////////////////////////////////////////////////////

void PDESolver::signal_solve_time_step(SignalArgs &args)
{
  SignalOptions opts(args);
  solve_time_step(opts.value<Real>("time_step"));
}

void PDESolver::signature_solve_time_step(SignalArgs &args)
{
  SignalOptions opts(args);
  opts.add("time_step",0.);
}

////////////////////////////////////////////////////////////////////////////////

void PDESolver::signal_solve_iterations(SignalArgs &args)
{
  SignalOptions opts(args);
  solve_iterations(opts.value<Uint>("iterations"));
}

void PDESolver::signature_solve_iterations(SignalArgs &args)
{
  SignalOptions opts(args);
  opts.add("iterations",0u);
}

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
