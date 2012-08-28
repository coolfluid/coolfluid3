// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/PE/Comm.hpp"

#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Mesh.hpp"

#include "solver/Time.hpp"
#include "solver/Model.hpp"
#include "solver/Solver.hpp"

#include "sdm/Tags.hpp"
#include "sdm/TimeIntegrationStepComputer.hpp"
#include "math/Consts.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::math::Consts;

namespace cf3 {
namespace sdm {

///////////////////////////////////////////////////////////////////////////////////////

TimeIntegrationStepComputer::TimeIntegrationStepComputer ( const std::string& name ) :
  common::Action(name),
  m_tolerance(1e-12)
{
  mark_basic();
  // options
  options().add("time_accurate", true)
    .description("Time Accurate")
    .pretty_name("Time Accurate")
    .mark_basic()
    .add_tag("time_accurate");

  options().add(sdm::Tags::update_coeff(), m_update_coeff)
    .description("Update coefficient to multiply with residual")
    .pretty_name("Update Coefficient")
    .link_to(&m_update_coeff);

  options().add(sdm::Tags::wave_speed(), m_wave_speed)
    .description("Wave Speed multiplied divided by characteristic length")
    .pretty_name("Wave Speed")
    .link_to(&m_wave_speed);

  options().add(sdm::Tags::time(), m_time)
    .description("Time Tracking component")
    .pretty_name("Time")
    .link_to(&m_time);

}

////////////////////////////////////////////////////////////////////////////////

Real TimeIntegrationStepComputer::limit_end_time(const Real& time, const Real& end_time)
{
  const Real milestone_dt  =  m_time->options().value<Real>("time_step");
  if (milestone_dt == 0.)
    return end_time;

  const Real milestone_time = (Uint((time+m_tolerance)/milestone_dt)+1.)*milestone_dt;
  return std::min(milestone_time,end_time);
}

////////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////////

