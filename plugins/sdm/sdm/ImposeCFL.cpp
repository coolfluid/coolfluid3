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
#include "sdm/ImposeCFL.hpp"
#include "math/Consts.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::math::Consts;

namespace cf3 {
namespace sdm {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ImposeCFL, sdm::TimeIntegrationStepComputer, LibSDM > ImposeCFL_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ImposeCFL::ImposeCFL ( const std::string& name ) :
  TimeIntegrationStepComputer(name)
{
  mark_basic();
  // options
  options().add("cfl", 1.)
    .description("Courant Number")
    .pretty_name("CFL")
    .mark_basic()
    .add_tag("cfl");
}

////////////////////////////////////////////////////////////////////////////////

void ImposeCFL::execute()
{
  if (is_null(m_wave_speed))   throw SetupError(FromHere(), "wave_speed was not configured");
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
      if (wave_speed[i][0] > 0.)
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


    // Fix wave-speed for visualization
    Real glb_max_dt;
    PE::Comm::instance().all_reduce(PE::min(), &max_dt, 1, &glb_max_dt);
    for (Uint i=0; i<wave_speed.size(); ++i)
    {
      if (wave_speed[i][0] == 0.)
      {
        wave_speed[i][0] = cfl/glb_max_dt;
      }
    }
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

    // Calculate the update_coefficient = CFL/wave_speed
    for (Uint i=0; i<wave_speed.size(); ++i)
    {
      if (wave_speed[i][0] == 0.)
      {
        wave_speed[i][0] = min_wave_speed;
      }
      update_coeff[i][0] = cfl/wave_speed[i][0];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////////

