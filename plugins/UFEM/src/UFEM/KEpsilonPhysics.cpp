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

#include "KEpsilonPhysics.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;

namespace detail
{
  template<typename FunctorT>
  Real solve_yplus(FunctorT&& f)
  {
    Real yp_low = 0.;
    Real yp_high = 1000.;
    Uint count = 1;
    Real yplus = 0.;
    while(((yp_high-yp_low) > 1e-4) && count < 1000)
    {
      yplus = (yp_low+yp_high)/2.;
      if (f(yplus) < yplus)
      {
        yp_high = yplus;
      }
      else
      {
        yp_low = yplus;
      }
      count += 1;
    }
    if(count == 1000)
    {
      throw common::FailedToConverge(FromHere(), "y+ computation did not converge");
    }
    return yplus;
  }
}

common::ComponentBuilder < KEpsilonPhysics, physics::PhysModel, LibUFEM > KEpsilonPhysics_Builder;

KEpsilonPhysics::KEpsilonPhysics(const std::string& name): NavierStokesPhysics(name)
{
  options().add("kappa", 0.4)
    .description("Von Karaman constant")
    .pretty_name("kappa")
    .attach_trigger(boost::bind(&KEpsilonPhysics::trigger_recompute, this))
    .mark_basic();

  options().add("z0", 0.)
    .description("Roughness height for ABL (m)")
    .pretty_name("z0")
    .attach_trigger(boost::bind(&KEpsilonPhysics::trigger_recompute, this))
    .mark_basic();

  options().add("uref", 0.)
    .description("Reference mean velocity for ABL (m/s)")
    .pretty_name("Uref")
    .attach_trigger(boost::bind(&KEpsilonPhysics::trigger_recompute, this))
    .mark_basic();

  options().add("href", 0.)
    .description("Reference height for ABL (m)")
    .pretty_name("Href")
    .attach_trigger(boost::bind(&KEpsilonPhysics::trigger_recompute, this))
    .mark_basic();

  options().add("yplus", 0.)
    .description("y+ value corresponding to the mesh boundary. Depends on kappa and set when z0 = 0")
    .pretty_name("y+")
    .attach_trigger(boost::bind(&KEpsilonPhysics::trigger_yplus, this))
    .mark_basic();

  options().add("utau", 0.)
    .description("Friction velocity. Computed from z0 , uref and href when z0 > 0")
    .pretty_name("utau")
    .attach_trigger(boost::bind(&KEpsilonPhysics::trigger_utau, this))
    .mark_basic();

  options().add("zwall", 0.)
    .description("Position of the mesh boundary relative to the physical wall")
    .pretty_name("z wall")
    .mark_basic();

  options().add("c_mu", 0.09)
    .description("C_mu model constant")
    .pretty_name("C_mu")
    .mark_basic();

  options().option("kinematic_viscosity").attach_trigger(boost::bind(&KEpsilonPhysics::trigger_recompute, this));

  trigger_recompute();
}

void KEpsilonPhysics::trigger_recompute()
{
  m_recursing = true;
  const Real kappa = options().value<Real>("kappa");
  const Real z0 = options().value<Real>("z0");
  const Real uref = options().value<Real>("uref");
  const Real href = options().value<Real>("href");

  if(z0 == 0.)
  {
    options().set("yplus", detail::solve_yplus([=](const Real yplus) { return 1./kappa * std::log(yplus) + 5.2; } ));
    options().set("utau", 0.);
  }
  else
  {
    const Real utau = kappa*uref/std::log((href+z0)/z0);
    const Real nu = options().value<Real>("kinematic_viscosity");
    options().set("utau", utau);
    options().set("yplus", 0.);
  }
  m_recursing = false;
}

void KEpsilonPhysics::trigger_yplus()
{
  if(m_recursing)
    return;

  throw common::BadValue(FromHere(), "Setting a value for yplus at " + uri().path() + " is not allowed. Please set z0, uref and h to obtain the correct value");
}

void KEpsilonPhysics::trigger_utau()
{
  if(m_recursing)
    return;

  throw common::BadValue(FromHere(), "Setting a value for utau at " + uri().path() + " is not allowed. Please set z0, uref and h to obtain the correct value");
}

} // UFEM
} // cf3
