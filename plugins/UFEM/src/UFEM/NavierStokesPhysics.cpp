// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"

#include "NavierStokesPhysics.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;

common::ComponentBuilder < NavierStokesPhysics, physics::PhysModel, LibUFEM > NavierStokesPhysics_Builder;

NavierStokesPhysics::NavierStokesPhysics(const std::string& name): DynamicModel(name), m_recursing(false)
{
  options().add("density", 1.2)
    .description("Mass density (kg / m^3)")
    .pretty_name("Density")
    .attach_trigger(boost::bind(&NavierStokesPhysics::trigger_rho, this))
    .link_to(&m_rho)
    .mark_basic();

  options().add("dynamic_viscosity", 1.7894e-5) //1.7894e-5
    .description("Dynamic Viscosity (kg / m s)")
    .pretty_name("Dynamic Viscosity")
    .attach_trigger(boost::bind(&NavierStokesPhysics::trigger_mu, this))
    .link_to(&m_mu)
    .mark_basic();

  options().add("kinematic_viscosity", 1.7894e-5/1.2) // 1.7894e-5/1.
    .description("Kinematic Viscosity (m^2/s)")
    .pretty_name("Kinematic Viscosity")
    .attach_trigger(boost::bind(&NavierStokesPhysics::trigger_nu, this))
    .mark_basic();

  options().add<Real>("reference_temperature", 288.)
    .description("Reference temperature for the boussinesq approximation (K)")
    .pretty_name("Reference temperature")
    .mark_basic();

  options().add("thermal_expansion_coefficient", 1.0)
    .description("Thermal expansion coefficient betha ()")
    .pretty_name("Thermal_expansion_coefficient")
    .mark_basic();

  options().add("specific_heat_capacity", 1.0) //cp
    .description("Specific heat capacity cp ()")
    .pretty_name("Specific_heat_capacity")
    .mark_basic();

  options().add("thermal_conductivity_solid", 1.0) //lambda_s
    .description("thermal conductivity in the solid ()")
    .pretty_name("Thermal_Conductivity_Solid")
    .mark_basic();

  options().add("scalar_coefficient", 1.0)  //m_alpha
    .description("scalar coefficient for scalar advection equaiton ()")
    .pretty_name("Scalar_Coefficient")
    .mark_basic();

  options().add("thermal_conductivity_fluid", 1.0) //lambda_f
    .description("thermal conductivity in the fluid ()")
    .pretty_name("Thermal_Conductivity_Fluid")
    .mark_basic();

  options().add("heat_transfer_coefficient", 1.0) //h
    .description("Heat transfer coefficient h for Robin boundary condition ()")
    .pretty_name("Heat_transfer_coefficient")
    .mark_basic();

  options().add("particle_density", 1000.0)
    .description("Density of the material building up a particle")
    .pretty_name("Particle Density")
    .mark_basic();

  options().add("heat_transfer_coefficient_dynamic", 1.0) //h_dynamic
    .description("Heat transfer coefficient h for Robin boundary condition, dynamically and locally calculated")
    .pretty_name("Heat_transfer_coefficient_dynamic")
    .mark_basic();

  options().add("bulk_temperature", 1000.) //t_bulk[K]
    .description("bulk temperature for dynamically calculated heat transfer coefficient")
    .pretty_name("Bulk_Temperature")
    .mark_basic();

  options().add("resistance_solid_coefficient", 1.0)
    .description("Resitance coeffient for the RobinFluid condition")
    .pretty_name("R Solid")
    .mark_basic();

  //options().add<RealVector>("gravitatonal_acceleration")
  //  .description("Acceleration due to gravitation ()")
  //  .pretty_name("Gravitatonal_acceleration")
  //  .mark_basic();
}

void NavierStokesPhysics::trigger_rho()
{
  m_recursing = true;
  options().set("kinematic_viscosity", m_mu / m_rho);
  m_recursing = false;
}

void NavierStokesPhysics::trigger_mu()
{
  trigger_rho();
}

void NavierStokesPhysics::trigger_nu()
{
  if(m_recursing)
    return;

  trigger_mu();
  throw common::BadValue(FromHere(), "Setting a value for the kinematic_viscosity at " + uri().path() + " is not allowed. Please set rho and mu to obtain the correct value");
}

} // UFEM
} // cf3
