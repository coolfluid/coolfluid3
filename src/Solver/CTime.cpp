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

  m_options.add_option(OptionT<Uint>::create("iteration","Iteration","Current iteration of the simulation", m_iter) )
      ->link_to(&m_iter)
      ->mark_basic();


  m_options.add_option(OptionT<Real>::create("time","Time","Current time of the simulation", m_time) )
      ->link_to(&m_time)
      ->mark_basic();

  m_options.add_option(OptionT<Real>::create("time_step","Time Step",
                                                "Maximal Time Step the simulation will use.\n"
                                                "A CFL condition will be applied to make time step more strict if required.",
                                                m_dt) )
      ->link_to(&m_dt)
      ->mark_basic();

  m_options.add_option(OptionT<Real>::create("end_time","End Time", "Time at which to finish the simulation", m_time) )
      ->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

CTime::~CTime()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
