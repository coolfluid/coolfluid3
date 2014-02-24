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

#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"
#include "mesh/LagrangeP0/Triag.hpp"
#include "mesh/LagrangeP0/Hexa.hpp"
#include "mesh/LagrangeP0/Tetra.hpp"
#include "mesh/LagrangeP0/Prism.hpp"

#include "SUPG.hpp"
#include "NSResidual.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/Tags.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;
using boost::proto::lit;

common::ComponentBuilder < NSResidual, common::Action, LibUFEM > NSResidual_Builder;

NSResidual::NSResidual(const std::string& name) :
  ProtoAction(name),
  m_theta(0.5)
{
  options().add(solver::Tags::time(), m_time)
    .pretty_name("Time")
    .description("Component that keeps track of time for this simulation")
    .attach_trigger(boost::bind(&NSResidual::trigger_time, this))
    .link_to(&m_time);
    
  options().add("alpha_ps", compute_tau.data.op.alpha_ps)
    .pretty_name("alpha_ps")
    .description("Constant to multiply the PSPG parameter with.")
    .link_to(&(compute_tau.data.op.alpha_ps));
      
  options().add("alpha_su", compute_tau.data.op.alpha_su)
    .pretty_name("alpha_su")
    .description("Constant to multiply the SUPG parameter with.")
    .link_to(&(compute_tau.data.op.alpha_su));
      
  options().add("alpha_bu", compute_tau.data.op.alpha_bu)
    .pretty_name("alpha_bu")
    .description("Constant to multiply the Bulk parameter with.")
    .link_to(&(compute_tau.data.op.alpha_bu));
    
  options().add("supg_type", compute_tau.data.op.supg_type_str)
    .pretty_name("SUPG Type")
    .description("Type of computation for the stabilization coefficients.")
    .link_to(&(compute_tau.data.op.supg_type_str))
    .attach_trigger(boost::bind(&ComputeTauImpl::trigger_supg_type, &compute_tau.data.op));
    
  options().add("c1", compute_tau.data.op.c1)
    .pretty_name("c1")
    .description("Constant adjusting the time part of SUPG in the metric tensor formulation")
    .link_to(&(compute_tau.data.op.c1));
    
  options().add("c2", compute_tau.data.op.c2)
    .pretty_name("c2")
    .description("Constant adjusting the time part of SUPG in the metric tensor formulation")
    .link_to(&(compute_tau.data.op.c2));
    
  options().add("theta", m_theta)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&m_theta);
    
  FieldVariable<0, VectorField> u_adv("AdvectionVelocity", "linearized_velocity");
  FieldVariable<1, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");
  FieldVariable<2, VectorField> u("single_field_velocity", "ns_single_field_solution");
  FieldVariable<3, VectorField> u1("AdvectionVelocity1", "linearized_velocity");
  FieldVariable<4, ScalarField> p("single_field_pressure", "ns_single_field_solution");
  FieldVariable<5, ScalarField> p1("copy_Pressure", "copy_navier_stokes_p_solution");
  FieldVariable<6, VectorField> u_residual("u_residual", "navier_stokes_residual");
  FieldVariable<7, ScalarField> p_residual("p_residual", "navier_stokes_residual");
  FieldVariable<8, VectorField> g("Force", "body_force");
  
  static boost::proto::terminal< ElementVector< boost::mpl::int_<1> > >::type const _x1 = {};
  static boost::proto::terminal< ElementVector< boost::mpl::int_<2> > >::type const _r = {};
  
  set_expression(elements_expression
  (
    boost::mpl::vector2<mesh::LagrangeP1::Quad2D, mesh::LagrangeP1::Hexa3D>(),
    group
    (
      compute_tau.apply(u_adv, nu_eff, lit(m_dt), lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      _A = _0, _T = _0, _a = _0,
      element_quadrature
      (
        _A(p    , u[_i]) += transpose(N(p) + tau_ps*u_adv*nabla(p)*0.5) * nabla(u)[_i] + tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(u), // Standard continuity + PSPG for advection
        _A(p    , p)     += tau_ps * transpose(nabla(p)) * nabla(p), // Continuity, PSPG
        _A(u[_i], u[_i]) += nu_eff * transpose(nabla(u)) * nabla(u) + transpose(N(u) + tau_su*u_adv*nabla(u)) * u_adv*nabla(u), // Diffusion + advection
        _A(u[_i], p)     += transpose(N(u) + tau_su*u_adv*nabla(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
        _A(u[_i], u[_j]) += transpose(tau_bulk*nabla(u)[_i] // Bulk viscosity
                            + 0.5*u_adv[_i]*(N(u) + tau_su*u_adv*nabla(u))) * nabla(u)[_j],  // skew symmetric part of advection (standard +SUPG)
        _T(p    , u[_i]) += tau_ps * transpose(nabla(p)[_i]) * N(u), // Time, PSPG
        _T(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u), // Time, standard and SUPG
        _a[u[_i]] += transpose(N(u)) * g[_i]
      ),
      group
      (
        _x1[u[_i]] = transpose(transpose(nodal_values(u1))[_i]),
        _x1[p] = nodal_values(p1),
        _r = _A*_x1 - _a,
        _A(p) = _A(p) / lit(m_theta),
        _r +=  ( _T/lit(m_dt) + lit(m_theta)*_A) * (_x - _x1),
        u_residual += _r[u],
        p_residual += _r[p]
      )
    )
  ));

  FieldVariable<0, VectorField> u_semi("Velocity", "navier_stokes_u_solution");
  FieldVariable<1, ScalarField> p_semi("Pressure", "navier_stokes_p_solution");

  m_zero_field = create_static_component<ProtoAction>("ZeroField");
  m_zero_field->set_expression(nodes_expression(group
  (
    u_residual[_i] = 0.,
    p_residual = 0.,
    u = u_semi,
    p = p_semi
  )));
}

NSResidual::~NSResidual()
{
}

void NSResidual::execute()
{
  m_zero_field->execute();
  cf3::solver::actions::Proto::ProtoAction::execute();
}

void NSResidual::on_regions_set()
{
  cf3::solver::Action::on_regions_set();
  m_zero_field->options().set("regions", options().option("regions").value());
}

void NSResidual::trigger_time()
{
  if(is_null(m_time))
      return;

  m_time->options().option("time_step").attach_trigger(boost::bind(&NSResidual::trigger_timestep, this));
  trigger_timestep();
}

void NSResidual::trigger_timestep()
{
  m_dt = m_time->dt();
  CFdebug << "Set NSResidual time step to " << m_dt << CFendl;
}

} // namespace UFEM

} // namespace cf3
