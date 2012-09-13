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

#include "solver/Criterion.hpp"
#include "solver/Time.hpp"
#include "solver/Solver.hpp"

#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

#include "sdm/Solver.hpp"
#include "sdm/Tags.hpp"

#include "solver/History.hpp"
#include "solver/Time.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::math::Consts;

namespace cf3 {
namespace sdm {

///////////////////////////////////////////////////////////////////////////////////////

Solver::Solver( const std::string& name ) :
  common::Action(name)
{
  mark_basic();

  // options
  options().add( "print_iteration_summary", true);
  options().add( "time_integration", m_time_integration ).link_to(&m_time_integration);
  options().add( "dict", m_dict).link_to(&m_dict);
  options().add( "time", m_time).link_to(&m_time);
  options().add( "max_iteration",math::Consts::uint_max()).mark_basic();
  options().add( "history", m_history).link_to(&m_history);
  options().add( "pre_iteration", m_pre_iteration).link_to(&m_pre_iteration);
  options().add( "post_iteration", m_post_iteration).link_to(&m_post_iteration);
}

////////////////////////////////////////////////////////////////////////////////

bool Solver::stop_condition()
{
  Uint nb_criteria = 0;
  bool finish = false;
  boost_foreach(Criterion& stop_criterion, find_components<Criterion>(*this))
  {
    finish |= stop_criterion();
    ++nb_criteria;
  }

//  if (options().value<bool>("time_accurate"))
  {
    if (m_time->current_time() >= m_time->end_time())
      return true;
  }

  if (m_time->iter() >= options().value<Uint>("max_iteration"))
    return true; // stop

  return finish;
}

////////////////////////////////////////////////////////////////////////////////

void Solver::execute()
{
  setup();
////  configure_option_recursively( sdm::Tags::time(),    m_time);
//  configure_option_recursively( "iterator", handle<Component>() );
//  // start loop - iterations start from 1 ( max iter zero will do nothing )

////  properties().property("iteration") = m_time->iter();

//  common::Timer timer;
//  Real walltime = properties().value<Real>("walltime");
  while( ! stop_condition() ) // time loop
  {
    //    timer.restart();

    if (m_pre_iteration) m_pre_iteration->execute();

    step();
    m_time->current_time() += m_time->dt();
    ++m_time->iter();

    history()->set("iter",m_time->iter());
    history()->set("time",m_time->current_time());
    history()->set("dt",m_time->dt());

    if (m_post_iteration) m_post_iteration->execute();

    history()->save_entry();

//    CFinfo << "  " << iteration_summary() << CFendl;
    if ( options().value<bool>("print_iteration_summary") )
      CFinfo << "  " << history()->entry().summary() << CFendl;

  }
  history()->flush();
}

std::string Solver::iteration_summary()
{
  std::stringstream ss;
  CFinfo << "iter [" << std::setw(4) << m_time->iter() << "]  "
         << "time [" << std::setw(12) << std::scientific << m_time->current_time() << "]  "
         << "dt ["<< std::scientific << std::setw(12) << m_time->dt() <<"]  ";
  return ss.str();
}

void Solver::raise_iteration_done()
{
  SignalOptions opts;
  const Uint iter = properties().value<Uint>("iteration");
  opts.add( "iteration", iter );
  SignalFrame frame = opts.create_frame("iteration_done", uri(), URI());
  common::Core::instance().event_handler().raise_event( "iteration_done", frame);
}

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3
