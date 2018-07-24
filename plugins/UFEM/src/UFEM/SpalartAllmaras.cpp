// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "SpalartAllmaras.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Component.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PropertyList.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/ElementGradDiv.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

static boost::proto::terminal< double(*)(double, double) >::type const _fv1 = {&fv1};

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;
using namespace boost::proto;

ComponentBuilder < SpalartAllmaras, LSSActionUnsteady, LibUFEM > SpalartAllmaras_builder;

SpalartAllmaras::SpalartAllmaras(const std::string& name) :
  LSSActionUnsteady(name),
  comp_sa(boost::proto::as_child(m_sa_coeff))
{
  options().add("velocity_tag", "navier_stokes_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the velocity field")
    .attach_trigger(boost::bind(&SpalartAllmaras::trigger_set_expression, this));

  options().add("theta", m_theta)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&m_theta);

  set_solution_tag("spalart_allmaras_solution");


  create_component<math::LSS::ZeroLSS>("ZeroLSS");
  create_component<ProtoAction>("Assembly");
  create_component<BoundaryConditions>("BoundaryConditions")->set_solution_tag(solution_tag());
  create_component<math::LSS::SolveLSS>("SolveLSS");
  create_component<ProtoAction>("Update");

  trigger_set_expression();
}

void SpalartAllmaras::trigger_set_expression()
{
  // The code will only be active for these element types
  boost::mpl::vector2<mesh::LagrangeP1::Quad2D, mesh::LagrangeP1::Triag2D> allowed_elements;

  FieldVariable<0, ScalarField> nu_sa("SAViscosity", solution_tag());
  FieldVariable<1, VectorField> u("Velocity",options().value<std::string>("velocity_tag"));
  FieldVariable<2, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity"); // This is the viscosity that needs to be modified to be visible in NavierStokes
  FieldVariable<3, ScalarField> d("wall_distance", "wall_distance");

  PhysicsConstant nu_lam("kinematic_viscosity");

  const auto d2 = make_lambda([&](const Real d)
  {
    if(d>0.0)
    {
      return d*d;
    }
    return 1e-16;
  });

  Handle<ProtoAction>(get_child("Assembly"))->set_expression(
  elements_expression
  (
    allowed_elements,
    group
    (
      _A = _0, _T = _0, _a = _0,
      compute_tau.apply( u, nu_eff, lit(dt()), lit(tau_su)),
      comp_sa( u, nu_sa, d, nu_lam),
      element_quadrature
      (
        _A(nu_sa) += transpose(N(nu_sa) + (tau_su*u + cw.apply(u, nu_sa))*nabla(nu_sa)) * u * nabla(nu_sa)   // advection terms
                   - lit(m_sa_coeff.op.cb1) * lit(m_sa_coeff.op.Stilde) * transpose(N(nu_sa)) * N(nu_sa) // production
                   + (lit(m_sa_coeff.op.cw1) * lit(m_sa_coeff.op.fw)) * nu_sa/d2(d) * transpose(N(nu_sa)) * N(nu_sa) // destruction
                   + (lit(1.)/lit(m_sa_coeff.op.sigma) * (nu_sa + nu_lam)) * transpose(nabla(nu_sa)) * nabla(nu_sa), // diffusion
        _T(nu_sa,nu_sa) +=  transpose(N(nu_sa) + (tau_su*u + cw.apply(u, nu_sa))*nabla(nu_sa)) * N(nu_sa), // Time, standard and SUPG
        _a[nu_sa] += N(nu_sa)*(lit(m_sa_coeff.op.cb2)/lit(m_sa_coeff.op.sigma)*(partial(nu_sa,_i) * partial(nu_sa,_i)))
      ),
      system_matrix += invdt() * _T + m_theta * _A,
      system_rhs += -_A * _x + _a
    )
  ));

  Handle<ProtoAction>(get_child("Update"))->set_expression(nodes_expression(
  group
  (
    nu_sa += solution(nu_sa),
    nu_eff = nu_lam + _max(0.0, nu_sa * _fv1(nu_sa/nu_lam, lit(m_sa_coeff.op.cv1)))
  )));
}

void SpalartAllmaras::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  initial_conditions.create_initial_condition(solution_tag());
}


} // UFEM
} // cf3
