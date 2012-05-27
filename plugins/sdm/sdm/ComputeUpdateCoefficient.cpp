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
#include "sdm/ComputeUpdateCoefficient.hpp"
#include "math/Consts.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::math::Consts;

namespace cf3 {
namespace sdm {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeUpdateCoefficient, common::Action, LibSDM > ComputeUpdateCoefficient_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ComputeUpdateCoefficient::ComputeUpdateCoefficient ( const std::string& name ) :
  solver::Action(name),
  m_tolerance(1e-12)
{
  mark_basic();
  // options
  options().add("time_accurate", true)
    .description("Time Accurate")
    .pretty_name("Time Accurate")
    .mark_basic()
    .add_tag("time_accurate");

  options().add("cfl", 1.)
    .description("Courant Number")
    .pretty_name("CFL")
    .mark_basic()
    .add_tag("cfl");

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

void ComputeUpdateCoefficient::execute()
{
  link_fields();

  if (is_null(m_wave_speed))   throw SetupError(FromHere(), "WaveSpeed field was not set");
  if (is_null(m_update_coeff)) throw SetupError(FromHere(), "UpdateCoeff Field was not set");

  Field& wave_speed = *m_wave_speed;
  Field& update_coeff = *m_update_coeff;
  Real cfl = options().value<Real>("cfl");
  if (options().value<bool>("time_accurate")) // global time stepping
  {
    if (is_null(m_time))   throw SetupError(FromHere(), "Time component was not set");

    Time& time = *m_time;

    cf3_assert_desc("Fields not compatible: "+to_str(update_coeff.size())+"!="+to_str(wave_speed.size()),update_coeff.size() == wave_speed.size());

    /// compute time step
    //  -----------------
    /// - take user-defined time step
    Real dt = time.options().value<Real>("time_step");
    if (dt==0.) dt = math::Consts::real_max();

    /// - Make time step stricter through the CFL number
    Real min_dt = dt;
    Real max_dt = 0.;
    RealVector ws(wave_speed.row_size());
    for (Uint i=0; i<wave_speed.size(); ++i)
    {
      if (wave_speed[i][0] > 0)
      {
        dt = cfl/wave_speed[i][0];

        min_dt = std::min(min_dt,dt);
        max_dt = std::max(max_dt,dt);
      }
    }
    Real glb_min_dt;
    PE::Comm::instance().all_reduce(PE::min(), &min_dt, 1, &glb_min_dt);
    dt = glb_min_dt;

    /// - Make sure we reach milestones and final simulation time
    Real tf = limit_end_time(time.current_time(), time.options().value<Real>("end_time"));
    if( time.current_time() + dt + m_tolerance > tf )
      dt = tf - time.current_time();

    /// Calculate the update_coefficient
    //  --------------------------------
    /// For Forward Euler: update_coefficient = @f$ \Delta t @f$.
    /// @f[ Q^{n+1} = Q^n + \Delta t \ R @f]
    for (Uint i=0; i<update_coeff.size(); ++i)
    {
      update_coeff[i][0] = dt ;
    }

    // Update the new time step
    time.dt() = dt;

  }
  else // local time stepping
  {
    if (is_not_null(m_time))  m_time->dt() = 0.;

    // Calculate the update_coefficient = CFL/wave_speed
    RealVector ws(wave_speed.row_size());
    for (Uint i=0; i<wave_speed.size(); ++i)
    {
      if (wave_speed[i][0] > 0)
        update_coeff[i][0] = cfl/wave_speed[i][0];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

Real ComputeUpdateCoefficient::limit_end_time(const Real& time, const Real& end_time)
{
  const Real milestone_dt  =  m_time->options().value<Real>("time_step");
  if (milestone_dt == 0)
    return end_time;

  const Real milestone_time = (Uint((time+m_tolerance)/milestone_dt)+1.)*milestone_dt;
  return std::min(milestone_time,end_time);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeUpdateCoefficient::link_fields()
{
  if( is_null( m_update_coeff ) )
  {
    m_update_coeff = Handle<Field>( follow_link( solver().field_manager().get_child( sdm::Tags::update_coeff() ) ) );
    options().set( sdm::Tags::update_coeff(), m_update_coeff );
  }

  if( is_null( m_wave_speed ) )
  {
    m_wave_speed = Handle<Field>( follow_link( solver().field_manager().get_child( sdm::Tags::wave_speed() ) ) );
    options().set( sdm::Tags::wave_speed(), m_wave_speed );
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////////

