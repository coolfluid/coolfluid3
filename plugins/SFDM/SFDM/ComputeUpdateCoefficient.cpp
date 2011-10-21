// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CBuilder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionT.hpp"
#include "common/PE/Comm.hpp"

#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Mesh.hpp"

#include "Solver/CTime.hpp"
#include "Solver/CModel.hpp"
#include "Solver/CSolver.hpp"

#include "SFDM/Tags.hpp"
#include "SFDM/ComputeUpdateCoefficient.hpp"
#include "math/Consts.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::Solver;
using namespace cf3::math::Consts;

namespace cf3 {
namespace SFDM {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeUpdateCoefficient, Action, LibSFDM > ComputeUpdateCoefficient_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ComputeUpdateCoefficient::ComputeUpdateCoefficient ( const std::string& name ) :
  Solver::Action(name),
  m_freeze(false),
  m_tolerance(1e-12)
{
  mark_basic();
  // options
  m_options.add_option< OptionT<bool> > ("time_accurate", true)
    ->description("Time Accurate")
    ->pretty_name("Time Accurate")
    ->mark_basic()
    ->add_tag("time_accurate");

  m_options.add_option< OptionT<Real> > ("cfl", 1.)
    ->description("Courant Number")
    ->pretty_name("CFL")
    ->mark_basic()
    ->add_tag("cfl");

  m_options.add_option(OptionComponent<Field>::create(SFDM::Tags::update_coeff(), &m_update_coeff))
    ->description("Update coefficient to multiply with residual")
    ->pretty_name("Update Coefficient");

  m_options.add_option(OptionComponent<Field>::create(SFDM::Tags::wave_speed(), &m_wave_speed ))
    ->description("Wave Speed multiplied divided by characteristic length")
    ->pretty_name("Wave Speed");

  m_options.add_option(OptionComponent<CTime>::create(SFDM::Tags::time(), &m_time))
    ->description("Time Tracking component")
    ->pretty_name("Time");

  m_options.add_option(OptionT<Real>::create("milestone_dt", 0.))
    ->description("Limits time-steps to fall on milestones")
    ->pretty_name("Milestone Time Step");

  m_options.add_option(OptionT<bool>::create("freeze_update_coeff", m_freeze))
    ->description("Disable (re)computation of update_coefficient. Some multistage methods might want to freeze this")
    ->pretty_name("Freeze Update Coefficient")
    ->link_to(&m_freeze);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeUpdateCoefficient::execute()
{
  if (m_freeze == false)
  {
    link_fields();

    if (m_wave_speed.expired())    throw SetupError(FromHere(), "WaveSpeed field was not set");
    if (m_update_coeff.expired()) throw SetupError(FromHere(), "UpdateCoeff Field was not set");

    Field& wave_speed = *m_wave_speed.lock();
    Field& update_coeff = *m_update_coeff.lock();
    Real cfl = option("cfl").value<Real>();

    if (option("time_accurate").value<bool>()) // global time stepping
    {
      if (m_time.expired())   throw SetupError(FromHere(), "Time component was not set");

      CTime& time = *m_time.lock();

      cf3_assert_desc("Fields not compatible: "+to_str(update_coeff.size())+"!="+to_str(wave_speed.size()),update_coeff.size() == wave_speed.size());

      /// compute time step
      //  -----------------
      /// - take user-defined time step
      Real dt = time.option("time_step").value<Real>();

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
      Real tf = limit_end_time(time.current_time(), time.option("end_time").value<Real>());
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
      if (!m_time.expired())  m_time.lock()->dt() = 0.;

      // Calculate the update_coefficient = CFL/wave_speed
      RealVector ws(wave_speed.row_size());
      for (Uint i=0; i<wave_speed.size(); ++i)
      {
        for (Uint j=0; j<wave_speed.row_size(); ++j)
          ws[j] = wave_speed[i][j];
        Real abs_ws = ws.norm();
        update_coeff[i][0] = cfl/(abs_ws);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

Real ComputeUpdateCoefficient::limit_end_time(const Real& time, const Real& end_time)
{
  const Real milestone_dt   = option("milestone_dt").value<Real>();
  if (milestone_dt == 0)
    return end_time;

  const Real milestone_time = (Uint((time+m_tolerance)/milestone_dt)+1.)*milestone_dt;
  return std::min(milestone_time,end_time);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeUpdateCoefficient::link_fields()
{
  if( is_null( m_update_coeff.lock() ) )
  {
    m_update_coeff = solver().field_manager()
        .get_child( SFDM::Tags::update_coeff() ).follow()->as_ptr_checked<Field>();
    configure_option( SFDM::Tags::update_coeff(), m_update_coeff.lock()->uri() );
  }

  if( is_null( m_wave_speed.lock() ) )
  {
    m_wave_speed = solver().field_manager()
        .get_child( SFDM::Tags::wave_speed() ).follow()->as_ptr_checked<Field>();
    configure_option( SFDM::Tags::wave_speed(), m_wave_speed.lock()->uri() );
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////////

