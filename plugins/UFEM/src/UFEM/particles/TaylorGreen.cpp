// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PE/Comm.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/EventHandler.hpp"

#include "math/Consts.hpp"

#include "TaylorGreen.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/Tags.hpp"

namespace cf3
{

namespace UFEM
{

namespace particles
{

using namespace solver::actions::Proto;
using boost::proto::lit;

namespace detail
{

using math::Consts::pi;

inline Real square(const Real x)
{
  return x*x;
}

Real TaylorGreenModel::ux(const Real t) const
{
  return Ua - (Vs*::cos((pi()*(-(t*Ua) + x))/D)*::sin((pi()*(-(t*Va) + y))/D))/::exp((2*nu*pi()*pi()*t)/(D*D));
}

Real TaylorGreenModel::uy(const Real t) const
{
  return Va + (Vs*::cos((pi()*(-(t*Va) + y))/D)*::sin((pi()*(-(t*Ua) + x))/D))/::exp((2*nu*pi()*pi()*t)/(D*D));
}

Real TaylorGreenModel::vx(const Real t) const
{
  return (8*(D*D*D)*::exp((6*nu*(pi()*pi())*t)/(D*D))*Ua + Vs*(4*::exp((2*nu*(pi()*pi())*t)/(D*D))*pi()*Vs*tau*(D*pi()*Ua*tau*(::cos((2*pi()*(-(t*Ua) + x))/D) + ::cos((2*pi()*(-(t*Va) + y))/D)) + (-1 + beta)*((D*D) + 2*nu*(pi()*pi())*tau)*::sin((2*pi()*(t*Ua - x))/D)) - 4*D*::exp((4*nu*(pi()*pi())*t)/(D*D))*((D*D) - 2*nu*(pi()*pi())*(-1 + beta)*tau)*(::sin((pi()*(t*(Ua - Va) - x + y))/D) + ::sin((pi()*(-(t*(Ua + Va)) + x + y))/D)) + D*(pi()*pi())*(Vs*Vs)*beta*(tau*tau)*(::sin((pi()*(t*(Ua + 3*Va) - x - 3*y))/D) + ::sin((pi()*(3*t*Ua + t*Va - 3*x - y))/D) - ::sin((pi()*(3*t*Ua - t*Va - 3*x + y))/D) - ::sin((pi()*(t*Ua - 3*t*Va - x + 3*y))/D))))/(8*(D*D*D)*::exp((6*nu*(pi()*pi())*t)/(D*D)) + 4*D*::exp((2*nu*(pi()*pi())*t)/(D*D))*(pi()*pi())*(Vs*Vs)*(tau*tau)*(::cos((2*pi()*(-(t*Ua) + x))/D) + ::cos((2*pi()*(-(t*Va) + y))/D)));
}

Real TaylorGreenModel::vy(const Real t) const
{
  return (8*(D*D*D)*::exp((6*nu*(pi()*pi())*t)/(D*D))*Va + 4*::exp((2*nu*(pi()*pi())*t)/(D*D))*pi()*(Vs*Vs)*tau*(D*pi()*Va*tau*(::cos((2*pi()*(-(t*Ua) + x))/D) + ::cos((2*pi()*(-(t*Va) + y))/D)) + (-1 + beta)*((D*D) + 2*nu*(pi()*pi())*tau)*::sin((2*pi()*(t*Va - y))/D)) - 4*D*::exp((4*nu*(pi()*pi())*t)/(D*D))*Vs*((D*D) - 2*nu*(pi()*pi())*(-1 + beta)*tau)*(::sin((pi()*(t*(Ua - Va) - x + y))/D) - ::sin((pi()*(-(t*(Ua + Va)) + x + y))/D)) - D*(pi()*pi())*(Vs*Vs*Vs)*beta*(tau*tau)*(::sin((pi()*(t*(Ua + 3*Va) - x - 3*y))/D) + ::sin((pi()*(3*t*Ua + t*Va - 3*x - y))/D) + ::sin((pi()*(3*t*Ua - t*Va - 3*x + y))/D) + ::sin((pi()*(t*Ua - 3*t*Va - x + 3*y))/D)))/(8*(D*D*D)*::exp((6*nu*(pi()*pi())*t)/(D*D)) + 4*D*::exp((2*nu*(pi()*pi())*t)/(D*D))*(pi()*pi())*(Vs*Vs)*(tau*tau)*(::cos((2*pi()*(-(t*Ua) + x))/D) + ::cos((2*pi()*(-(t*Va) + y))/D)));
}

void update_tg_values_func(const TaylorGreenModel& d, RealVector& out, const Real t, const Real dt)
{
  out[0] = d.ux(t);
  out[1] = d.uy(t);
  out[2] = d.ux(t+dt);
  out[3] = d.uy(t+dt);
  out[4] = d.vx(t+dt);
  out[5] = d.vy(t+dt);
}
static boost::proto::terminal< void(*)(const TaylorGreenModel&, RealVector&, const Real, const Real) >::type const update_tg_values = {&update_tg_values_func};

}

common::ComponentBuilder < TaylorGreen, common::Action, LibUFEMParticles > TaylorGreen_Builder;

TaylorGreen::TaylorGreen(const std::string& name) :
  ProtoAction(name),
  m_tg_values(6)
{
  options().add(solver::Tags::time(), m_time)
    .pretty_name("Time")
    .description("Component that keeps track of time for this simulation")
    .attach_trigger(boost::bind(&TaylorGreen::trigger_time, this))
    .link_to(&m_time);
    
  options().add("ua", m_tg_model.Ua)
    .pretty_name("Ua")
    .description("Constant X velocity")
    .link_to(&(m_tg_model.Ua));
      
  options().add("va", m_tg_model.Va)
    .pretty_name("Va")
    .description("Constant Y velocity")
    .link_to(&(m_tg_model.Va));

  options().add("vs", m_tg_model.Vs)
    .pretty_name("Vs")
    .description("Vortex velocity")
    .link_to(&(m_tg_model.Vs));

  options().add("d", m_tg_model.D)
    .pretty_name("D")
    .description("Vortex diameter")
    .link_to(&(m_tg_model.D));

  options().add("beta", m_tg_model.beta)
    .pretty_name("Beta")
    .description("Particle density ratio parameter")
    .link_to(&(m_tg_model.beta));

  options().add("tau", m_tg_model.tau)
    .pretty_name("Tau")
    .description("Generalized particle relaxation time")
    .link_to(&(m_tg_model.tau));
    
  FieldVariable<0, VectorField> u("FluidVelocityTG", "taylor_green");
  FieldVariable<1, VectorField> u1("AdvectionVelocity1", "linearized_velocity");
  FieldVariable<2, VectorField> v("ParticleVelocityTG", "taylor_green");
  PhysicsConstant nu("kinematic_viscosity");
  PhysicsConstant rho_f("density");
  
  set_expression(nodes_expression_2d
  (
    group
    (
      group
      (
        lit(m_tg_model.nu) = nu,
        lit(m_tg_model.x) = coordinates[0],
        lit(m_tg_model.y) = coordinates[1]
      ),
      detail::update_tg_values(lit(m_tg_model), lit(m_tg_values), lit(m_t), lit(m_dt)),
      group
      (
        u1[0] = lit(m_tg_values)[0],
        u1[1] = lit(m_tg_values)[1],
        u[0] = lit(m_tg_values)[2],
        u[1] = lit(m_tg_values)[3],
        v[0] = lit(m_tg_values)[4],
        v[1] = lit(m_tg_values)[5]
      )
    )
  ));
}

TaylorGreen::~TaylorGreen()
{
}


void TaylorGreen::trigger_time()
{
  if(is_null(m_time))
      return;

  m_time->options().option("current_time").attach_trigger(boost::bind(&TaylorGreen::trigger_current_time, this));
  m_time->options().option("time_step").attach_trigger(boost::bind(&TaylorGreen::trigger_current_time, this));
  trigger_current_time();
}

void TaylorGreen::trigger_current_time()
{
  m_t = m_time->current_time();
  m_dt = m_time->dt();
}

} // namespace particles

} // namespace UFEM

} // namespace cf3
