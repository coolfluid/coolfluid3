// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionT.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "Solver/CTime.hpp"
#include "Solver/CModel.hpp"
#include "FVM/Core/ComputeUpdateCoefficient.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;

namespace CF {
namespace FVM {
namespace Core {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ComputeUpdateCoefficient, CAction, LibCore > ComputeUpdateCoefficient_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ComputeUpdateCoefficient::ComputeUpdateCoefficient ( const std::string& name ) :
  CAction(name),
  m_time_accurate(false),
  m_CFL(1.)
{
  mark_basic();
  // options
  m_options.add_option< OptionT<bool> > ("time_accurate", m_time_accurate)
      ->set_description("Time Accurate")
      ->set_pretty_name("Time Accurate")
      ->mark_basic()
      ->link_to(&m_time_accurate)
      ->add_tag("time_accurate");

  m_options.add_option< OptionT<Real> > ("cfl", m_CFL)
      ->set_description("Courant Number")
      ->set_pretty_name("CFL")
      ->mark_basic()
      ->link_to(&m_CFL)
      ->add_tag("cfl");

  m_options.add_option(OptionURI::create("update_coeff", URI("cpath:"), URI::Scheme::CPATH))
      ->set_description("Update coefficient to multiply with residual")
      ->set_pretty_name("Update Coefficient")
      ->attach_trigger ( boost::bind ( &ComputeUpdateCoefficient::config_update_coeff,   this ) )
      ->add_tag("update_coeff");

  m_options.add_option(OptionURI::create("wave_speed", URI("cpath:"), URI::Scheme::CPATH))
      ->set_description("WaveSpeed needed")
      ->set_pretty_name("Wave Speed")
      ->attach_trigger ( boost::bind ( &ComputeUpdateCoefficient::config_wave_speed,   this ) )
      ->add_tag("wave_speed");

  m_options.add_option(OptionURI::create("volume", URI("cpath:"), URI::Scheme::CPATH))
      ->set_description("Volume needed for time accurate simulations")
      ->set_pretty_name("Volume")
      ->attach_trigger ( boost::bind ( &ComputeUpdateCoefficient::config_volume,   this ) )
      ->add_tag(Mesh::Tags::volume());

  m_options.add_option(OptionURI::create("ctime", URI("cpath:"), URI::Scheme::CPATH))
      ->set_description("Time Tracking component")
      ->set_pretty_name("Time")
      ->attach_trigger ( boost::bind ( &ComputeUpdateCoefficient::config_time,   this ) )
      ->add_tag("ctime");

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
  URI uri;  option("ctime").put_value(uri);
  m_time = Common::Core::instance().root().access_component_ptr(uri)->as_ptr<CTime>();
}

////////////////////////////////////////////////////////////////////////////////

void ComputeUpdateCoefficient::execute()
{
  if (m_wave_speed.expired())    throw SetupError(FromHere(), "WaveSpeed field was not set");
  if (m_update_coeff.expired()) throw SetupError(FromHere(), "UpdateCoeff Field was not set");

  CTable<Real>& wave_speed = m_wave_speed.lock()->data();
  CTable<Real>& update_coeff = m_update_coeff.lock()->data();

  if (m_time_accurate) // global time stepping
  {
    if (m_time.expired())   throw SetupError(FromHere(), "Time component was not set");
    if (m_volume.expired()) throw SetupError(FromHere(), "Volume Field was not set");

    CTime& time = *m_time.lock();
    CTable<Real>& volume = m_volume.lock()->data();

    cf_assert_desc("Fields not compatible",volume.size() == wave_speed.size());
    cf_assert_desc("Fields not compatible",update_coeff.size() == wave_speed.size());

    // compute which dt to take
    Real tf = time.option("end_time").value<Real>();
    Real dt = time.option("time_step").value<Real>();
    if( time.current_time() + dt > tf )
      dt = tf - time.current_time();

    // Make time step stricter through the CFL number
    for (Uint i=0; i<wave_speed.size(); ++i)
    {
      if (volume[i][0] > 0)
        dt = std::min(dt, m_CFL*volume[i][0]/wave_speed[i][0] );
    }

    // Calculate the update_coefficient = dt/dx
    for (Uint i=0; i<update_coeff.size(); ++i)
    {
      if (volume[i][0] > 0)
        update_coeff[i][0] = dt/volume[i][0];
    }

    // Update the new time step
    time.dt() = dt;

  }
  else // local time stepping
  {
    if (!m_time.expired())  m_time.lock()->dt() = 0.;

    // Calculate the update_coefficient = CFL/wave_speed
    for (Uint i=0; i<update_coeff.size(); ++i)
      update_coeff[i][0] = m_CFL/wave_speed[i][0];
  }
}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

