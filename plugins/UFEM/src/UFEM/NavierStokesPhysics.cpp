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

NavierStokesPhysics::NavierStokesPhysics(const std::string& name): DynamicModel(name)
{
  options().add<Real>("reference_velocity")
    .description("Reference velocity for the calculation of the stabilization coefficients")
    .pretty_name("Reference velocity")
    .link_to(&m_coeffs.u_ref);

  options().add("density", 1.2)
    .description("Mass density (kg / m^3)")
    .pretty_name("Density")
    .link_to(&m_coeffs.rho)
    .attach_trigger(boost::bind(&NavierStokesPhysics::trigger_rho, this));

  options().add("dynamic_viscosity", 1.7894e-5)
    .description("Dynamic Viscosity (kg / m s)")
    .pretty_name("Dynamic Viscosity")
    .link_to(&m_coeffs.mu);
}

void NavierStokesPhysics::trigger_rho()
{
  m_coeffs.one_over_rho = 1. / options().option("density").value<Real>();
  BOOST_FOREACH(SUPGCoeffs* coeffs, m_linked_coeffs)
  {
    coeffs->one_over_rho = m_coeffs.one_over_rho;
  }
}

void NavierStokesPhysics::link_properties(SUPGCoeffs& props)
{
  m_linked_coeffs.push_back(&props);
  
  props = m_coeffs;
  
  options()["reference_velocity"].link_to(&props.u_ref);
  options()["density"].link_to(&props.rho);
  options()["dynamic_viscosity"].link_to(&props.mu);
}



} // UFEM
} // cf3
