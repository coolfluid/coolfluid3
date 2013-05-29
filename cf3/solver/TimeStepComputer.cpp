// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionList.hpp"

#include "mesh/Field.hpp"
#include "solver/Time.hpp"
#include "solver/TimeStepComputer.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {

common::RegistTypeInfo<TimeStepComputer,LibSolver> regist_TimeStepComputer();

////////////////////////////////////////////////////////////////////////////////

TimeStepComputer::TimeStepComputer ( const std::string& name ) :
  common::Action(name)
{
  regist_typeinfo(this);

  options().add("time_accurate", true)
    .description("Time Accurate")
    .pretty_name("Time Accurate")
    .mark_basic()
    .add_tag("time_accurate");

  options().add("time_step", m_time_step)
    .description("Time step")
    .pretty_name("Time step")
    .link_to(&m_time_step);

  options().add("wave_speed", m_wave_speed)
    .description("Wave Speed divided by characteristic length")
    .pretty_name("Wave Speed")
    .link_to(&m_wave_speed);

  options().add("time", m_time)
    .description("Time Tracking component")
    .pretty_name("Time")
    .link_to(&m_time);
}

////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

