// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionT.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CMesh.hpp"
#include "Solver/CTime.hpp"
#include "Solver/CModel.hpp"
#include "FVM/ComputeUpdateCoefficient.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

namespace CF {
namespace FVM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ComputeUpdateCoefficient, CAction, LibFVM > ComputeUpdateCoefficient_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
ComputeUpdateCoefficient::ComputeUpdateCoefficient ( const std::string& name ) : 
  CAction(name),
  m_time_accurate(false),
  m_CFL(1.)
{
  mark_basic();
  // options
  m_properties.add_option< OptionT<bool> > ("Time Accurate", "Time Accurate", m_time_accurate)
    ->mark_basic()
    ->link_to(&m_time_accurate)
    ->add_tag("time_accurate");
    
  m_properties.add_option< OptionT<Real> > ("CFL", "Courant Number", m_CFL)
    ->mark_basic()
    ->link_to(&m_CFL)
    ->add_tag("cfl");

  m_properties.add_option(OptionURI::create("UpdateCoeff","Update coefficient", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &ComputeUpdateCoefficient::config_update_coeff,   this ) )
    ->add_tag("update_coeff");

  m_properties.add_option(OptionURI::create("WaveSpeed","WaveSpeed needed", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &ComputeUpdateCoefficient::config_wave_speed,   this ) )
    ->add_tag("wave_speed");

  m_properties.add_option(OptionURI::create("Volume","Volume needed for time accurate simulations", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &ComputeUpdateCoefficient::config_volume,   this ) )
    ->add_tag("volume");

  m_properties.add_option(OptionURI::create("Time","Time", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &ComputeUpdateCoefficient::config_time,   this ) )
    ->add_tag("time");

}

////////////////////////////////////////////////////////////////////////////////

void ComputeUpdateCoefficient::config_update_coeff()
{
  URI uri;  property("UpdateCoeff").put_value(uri);
  m_update_coeff = Core::instance().root()->look_component<CField2>(uri);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeUpdateCoefficient::config_volume()
{
  URI uri;  property("Volume").put_value(uri);
  m_volume = Core::instance().root()->look_component<CField2>(uri);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeUpdateCoefficient::config_wave_speed()
{
  URI uri;  property("WaveSpeed").put_value(uri);
  m_wave_speed = Core::instance().root()->look_component<CField2>(uri);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeUpdateCoefficient::config_time()
{
  URI uri;  property("Time").put_value(uri);
  m_time = Core::instance().root()->look_component<CTime>(uri);
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

    // Set time step to user-specified timestep
    // It is up to an external controller to set the internal value
    // time.dt() to the default value.
    Real dt = time.dt();
    
    // Make time step stricter through the CFL number
    for (Uint i=0; i<wave_speed.size(); ++i)
    {
      dt = std::min(dt, m_CFL*volume[i][0]/wave_speed[i][0] );
    }
    
    // Update to more strict time step
    time.dt() = dt;

    // Calculate the update_coefficient = dt/dx
    update_coeff = time.dt();
    update_coeff /= volume;
  }
  else // local time stepping
  {
    if (!m_time.expired())  m_time.lock()->dt() = 0.;

    // Calculate the update_coefficient = CFL/wave_speed
    update_coeff = m_CFL;
    update_coeff /= wave_speed;
  }
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

