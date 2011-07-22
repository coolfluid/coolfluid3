// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @todo remove
#include "Common/Log.hpp"

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionT.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "Solver/CTime.hpp"
#include "Solver/CModel.hpp"
#include "Solver/FlowSolver.hpp"
#include "SFDM/ComputeUpdateCoefficient.hpp"
#include "Math/MathConsts.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Math::MathConsts;

namespace CF {
namespace SFDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ComputeUpdateCoefficient, CAction, LibSFDM > ComputeUpdateCoefficient_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ComputeUpdateCoefficient::ComputeUpdateCoefficient ( const std::string& name ) :
  CAction(name),
  m_time_accurate(false),
  m_CFL(1.),
  m_freeze(false),
  m_tolerance(1e-12)
{
  mark_basic();
  // options
  m_options.add_option< OptionT<bool> > ("time_accurate", m_time_accurate)
    ->description("Time Accurate")
    ->pretty_name("Time Accurate")
    ->mark_basic()
    ->link_to(&m_time_accurate)
    ->add_tag("time_accurate");

  m_options.add_option< OptionT<Real> > ("cfl", m_CFL)
    ->description("Courant Number")
    ->pretty_name("CFL")
    ->mark_basic()
    ->link_to(&m_CFL)
    ->add_tag("cfl");

  m_options.add_option(OptionURI::create(FlowSolver::Tags::update_coeff(), URI("cpath:"), URI::Scheme::CPATH))
    ->description("Update coefficient to multiply with residual")
    ->pretty_name("Update Coefficient")
    ->attach_trigger ( boost::bind ( &ComputeUpdateCoefficient::config_update_coeff,   this ) );

  m_options.add_option(OptionURI::create(FlowSolver::Tags::wave_speed(), URI("cpath:"), URI::Scheme::CPATH))
    ->description("WaveSpeed needed")
    ->pretty_name("Wave Speed")
    ->attach_trigger ( boost::bind ( &ComputeUpdateCoefficient::config_wave_speed,   this ) );

  m_options.add_option(OptionURI::create("volume", URI("cpath:"), URI::Scheme::CPATH))
    ->description("Volume needed for time accurate simulations")
    ->pretty_name("Volume")
    ->attach_trigger ( boost::bind ( &ComputeUpdateCoefficient::config_volume,   this ) );

  m_options.add_option(OptionURI::create(FlowSolver::Tags::time(), URI("cpath:"), URI::Scheme::CPATH))
    ->description("Time Tracking component")
    ->pretty_name("Time")
    ->attach_trigger ( boost::bind ( &ComputeUpdateCoefficient::config_time,   this ) );

  m_options.add_option(OptionT<Real>::create("milestone_dt", 0.))
    ->description("Limits time-steps to fall on milestones")
    ->pretty_name("Milestone Time Step");

  m_options.add_option(OptionT<bool>::create("freeze_update_coeff", m_freeze))
    ->description("Disable (re)computation of update_coefficient. Some multistage methods might want to freeze this")
    ->pretty_name("Freeze Update Coefficient")
    ->link_to(&m_freeze);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeUpdateCoefficient::config_update_coeff()
{
  URI uri;  option("update_coeff").put_value(uri);
  m_update_coeff = Common::Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
}

////////////////////////////////////////////////////////////////////////////////

void ComputeUpdateCoefficient::config_volume()
{
  URI uri;  option("volume").put_value(uri);
  m_volume = Common::Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
}

////////////////////////////////////////////////////////////////////////////////

void ComputeUpdateCoefficient::config_wave_speed()
{
  URI uri;  option("wave_speed").put_value(uri);
  m_wave_speed = Common::Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
}

////////////////////////////////////////////////////////////////////////////////

void ComputeUpdateCoefficient::config_time()
{
  URI uri;  option("time").put_value(uri);
  m_time = Common::Core::instance().root().access_component_ptr(uri)->as_ptr<CTime>();
}

////////////////////////////////////////////////////////////////////////////////

void ComputeUpdateCoefficient::execute()
{
  if (m_freeze == false)
  {
    if (m_wave_speed.expired())    throw SetupError(FromHere(), "WaveSpeed field was not set");
    if (m_update_coeff.expired()) throw SetupError(FromHere(), "UpdateCoeff Field was not set");
    if (m_volume.expired()) throw SetupError(FromHere(), "Volume Field was not set");

    CTable<Real>& wave_speed = m_wave_speed.lock()->data();
    CTable<Real>& update_coeff = m_update_coeff.lock()->data();
    CTable<Real>& volume = m_volume.lock()->data();

    CTable<Real>& jacob_det = m_volume.lock()->parent().get_child("jacobian_determinant").as_type<CField>().data();

    if (m_time_accurate) // global time stepping
    {
      if (m_time.expired())   throw SetupError(FromHere(), "Time component was not set");

      CTime& time = *m_time.lock();

      cf_assert_desc("Fields not compatible: "+to_str(volume.size())+"!="+to_str(wave_speed.size()),volume.size() == wave_speed.size());
      cf_assert_desc("Fields not compatible: "+to_str(update_coeff.size())+"!="+to_str(wave_speed.size()),update_coeff.size() == wave_speed.size());

      /// compute time step
      //  -----------------
      /// - take user-defined time step
      Real dt = time.option("time_step").value<Real>();

      /// - Make time step stricter through the CFL number
      Real min_dt = dt;
      Real max_dt = 0.;
      for (Uint i=0; i<wave_speed.size(); ++i)
      {
        if (volume[i][0] > 0 && wave_speed[i][0] > 0)
        {
          dt = m_CFL*volume[i][0]/wave_speed[i][0];
//          dt = m_CFL/wave_speed[i][0];

          min_dt = std::min(min_dt,dt);
          max_dt = std::max(max_dt,dt);
        }
      }
      dt = min_dt;
      /// - Make sure we reach milestones and final simulation time
      Real tf = limit_end_time(time.time(), time.option("end_time").value<Real>());
      if( time.time() + dt + m_tolerance > tf )
        dt = tf - time.time();

      /// Calculate the update_coefficient
      //  --------------------------------
      /// For Forward Euler: update_coefficient = @f$ \Delta t @f$.
      /// @f[ Q^{n+1} = Q^n + \Delta t \ R @f]
      for (Uint i=0; i<update_coeff.size(); ++i)
      {
        update_coeff[i][0] = dt ;/// jacob_det[i][0];
      }

      // Update the new time step
      time.dt() = dt;

    }
    else // local time stepping
    {
      if (!m_time.expired())  m_time.lock()->dt() = 0.;

      // Calculate the update_coefficient = CFL/wave_speed
      for (Uint i=0; i<update_coeff.size(); ++i)
        update_coeff[i][0] = m_CFL*volume[i][0]/wave_speed[i][0];
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

} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

