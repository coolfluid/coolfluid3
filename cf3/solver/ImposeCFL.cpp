// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/PE/Comm.hpp"
#include "math/Consts.hpp"
#include "mesh/Field.hpp"
#include "solver/ImposeCFL.hpp"
#include "solver/Time.hpp"


/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::math::Consts;

namespace cf3 {
namespace solver {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ImposeCFL, common::Action, LibSolver > ImposeCFL_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ImposeCFL::ImposeCFL ( const std::string& name ) :
  TimeStepComputer(name),
  m_cfl(0.)
{
  mark_basic();
  // options
  options().add("cfl", std::string("1.") )
    .description("Courant Number")
    .pretty_name("CFL")
    .mark_basic()
    .attach_trigger( boost::bind( &ImposeCFL::parse_cfl, this) );
  parse_cfl();
}

////////////////////////////////////////////////////////////////////////////////

void ImposeCFL::parse_cfl()
{
  m_cfl_function.parse( options().value<std::string>("cfl"), "i,t,cfl" );
}

////////////////////////////////////////////////////////////////////////////////

void ImposeCFL::change_with_factor(const Real& factor)
{
  m_cfl *= factor;
  *m_time_step *= factor;
  m_time->dt() *= factor;
}

////////////////////////////////////////////////////////////////////////////////

void ImposeCFL::execute()
{
  if (is_null(m_wave_speed))  throw SetupError(FromHere(), "wave_speed was not configured");
  if (is_null(m_time_step))   throw SetupError(FromHere(), "time_step Field was not set");
  if (is_null(m_time))        throw SetupError(FromHere(), "Time component was not set");

  Field& wave_speed = *m_wave_speed;
  Field& time_step = *m_time_step;

  std::vector<Real> args(3);
  args[0] = m_time->iter();
  args[1] = m_time->current_time();
  args[2] = m_cfl;
  m_cfl = m_cfl_function(args);

  if (options().value<bool>("time_accurate")) // global time stepping
  {
    Time& time = *m_time;

    cf3_assert_desc("Fields not compatible: "+to_str(time_step.size())+"!="+to_str(wave_speed.size()),time_step.size() == wave_speed.size());

    /// compute time step
    //  -----------------
    /// - take user-defined time step
    Real dt = time.options().value<Real>("time_step");
    if (dt==0.) dt = math::Consts::real_max();

    /// - Make time step stricter through the CFL number
    Real min_dt = dt;
    Real max_dt = 0.;
    for (Uint i=0; i<wave_speed.size(); ++i)
    {
      if (wave_speed[i][0] > 0.)
      {
        dt = m_cfl/wave_speed[i][0];

        min_dt = std::min(min_dt,dt);
        max_dt = std::max(max_dt,dt);
      }
    }

    Real glb_min_dt;
    PE::Comm::instance().all_reduce(PE::min(), &min_dt, 1, &glb_min_dt);
    dt = glb_min_dt;

    /// - Make sure we reach final simulation time
    Real tf = time.options().value<Real>("end_time");
    if( time.current_time() + dt*(1+sqrt(eps()))> tf )
      dt = tf - time.current_time();

    /// Calculate the time_step
    //  -----------------------
    /// For Forward Euler: time_step = @f$ \Delta t @f$.
    /// @f[ Q^{n+1} = Q^n + \Delta t \ R @f]
    for (Uint i=0; i<time_step.size(); ++i)
    {
      time_step[i][0] = dt ;
    }

    // Update the new time step
    time.dt() = dt;

// UNCOMMENTING THIS WILL FIX THE UPPER-LIMIT OF THE WAVESPEED FOREVER :(
// DUE TO LINE 88
//    // Fix wave-speed for visualization
//    Real glb_max_dt;
//    PE::Comm::instance().all_reduce(PE::min(), &max_dt, 1, &glb_max_dt);
//    for (Uint i=0; i<wave_speed.size(); ++i)
//    {
//      if (wave_speed[i][0] == 0.)
//      {
//        wave_speed[i][0] = cfl/glb_max_dt;
//      }
//    }
  }
  else // local time stepping
  {
    if (is_not_null(m_time))  m_time->dt() = 0.;

    // Check for a minimum value for the wave speeds
    Real min_wave_speed = math::Consts::real_max();
    for (Uint i=0; i<wave_speed.size(); ++i)
    {
      if (wave_speed[i][0] > 0.)
      {
        min_wave_speed = std::min(min_wave_speed,wave_speed[i][0]);
      }
    }
    PE::Comm::instance().all_reduce(PE::min(), &min_wave_speed, 1, &min_wave_speed);
    if (min_wave_speed == 0.)
      throw common::BadValue(FromHere(), "Minimum wave-speed cannot be zero!");

    // Calculate the time_stepicient = CFL/wave_speed
    for (Uint i=0; i<wave_speed.size(); ++i)
    {
      if (wave_speed[i][0] == 0.)
      {
        wave_speed[i][0] = min_wave_speed;
      }
      time_step[i][0] = m_cfl/wave_speed[i][0];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

