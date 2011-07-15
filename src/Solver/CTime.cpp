// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"

#include "Solver/LibSolver.hpp"
#include "Solver/CTime.hpp"

namespace CF {
namespace Solver {

using namespace Common;

Common::ComponentBuilder < CTime, Component, LibSolver > CTime_Builder;

////////////////////////////////////////////////////////////////////////////////

CTime::CTime ( const std::string& name  ) :
  Component ( name ),
  m_time(0.),
  m_dt(0.),
  m_invdt(0.),
  m_iter(0)
{
  mark_basic();

  m_properties["brief"] = std::string("Time Tracking object");
  std::string description =
    "Offers configuration options for users to set a time step, the end time,\n"
    "and the current time.\n"
    "It also offers access functions to these values internally.\n"
    "Notice that the configuration options don't change value automatically to reflext the internal state,\n"
    "unless the code explicitely (re)configures them.";
  m_properties["description"] = description;

  m_options.add_option(OptionT<Uint>::create("iteration", m_iter) )
      ->set_description("Current iteration of the simulation")
      ->set_pretty_name("Iteration")
      ->link_to(&m_iter)
      ->mark_basic();


  m_options.add_option(OptionT<Real>::create("time", m_time) )
      ->set_description("Current time of the simulation")
      ->set_pretty_name("Time")
      ->link_to(&m_time)
      ->mark_basic();

  m_options.add_option(OptionT<Real>::create("time_step", m_dt) )
      ->set_description("Maximal Time Step the simulation will use.\n"
                        "A CFL condition will be applied to make time step more strict if required.")
      ->set_pretty_name("Time Step")
      ->link_to(&m_dt)
      ->mark_basic()
      ->attach_trigger(boost::bind(&CTime::trigger_timestep, this));

  m_options.add_option(OptionT<Real>::create("end_time", m_time) )
      ->set_description("Time at which to finish the simulation")
      ->set_pretty_name("End Time")
      ->link_to(&m_end_time)
      ->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

CTime::~CTime()
{
}

////////////////////////////////////////////////////////////////////////////////

void CTime::trigger_timestep()
{
  m_invdt = 1. / m_dt;
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
