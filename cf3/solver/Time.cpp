// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "solver/LibSolver.hpp"
#include "solver/Time.hpp"

namespace cf3 {
namespace solver {

using namespace common;

common::ComponentBuilder < Time, Component, LibSolver > Time_Builder;

////////////////////////////////////////////////////////////////////////////////

Time::Time ( const std::string& name  ) :
  Component ( name ),
  m_current_time(0.),
  m_dt(0.),
  m_invdt(0.),
  m_iter(0)
{
  mark_basic();

  properties()["brief"] = std::string("Time Tracking object");
  std::string description =
    "Offers configuration options for users to set a time step, the end time,\n"
    "and the current time.\n"
    "It also offers access functions to these values internally.\n"
    "Notice that the configuration options don't change value automatically to reflext the internal state,\n"
    "unless the code explicitely (re)configures them.";
  properties()["description"] = description;

  options().add("iteration", m_iter)
      .description("Current iteration of the simulation")
      .pretty_name("Iteration")
      .link_to(&m_iter)
      .mark_basic();


  options().add("current_time", m_current_time)
      .description("Current time of the simulation")
      .pretty_name("Current Time")
      .link_to(&m_current_time)
      .mark_basic();

  options().add("time_step", m_dt)
      .description("Maximal Time Step the simulation will use.\n"
                        "A CFL condition will be applied to make time step more strict if required.")
      .pretty_name("Time Step")
      .link_to(&m_dt)
      .mark_basic()
      .attach_trigger(boost::bind(&Time::trigger_timestep, this));

  options().add("end_time", m_current_time)
      .description("Time at which to finish the simulation")
      .pretty_name("End Time")
      .link_to(&m_end_time)
      .mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

Time::~Time()
{
}

////////////////////////////////////////////////////////////////////////////////

void Time::trigger_timestep()
{
  m_invdt = 1. / m_dt;
}

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
