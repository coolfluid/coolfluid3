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

#include "solver/actions/SolveLSS.hpp"
#include "solver/actions/ZeroLSS.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/actions/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "Tags.hpp"

#include "UFEM/NavierStokesOps.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

ComponentBuilder < SpalartAllmaras, LSSActionUnsteady, LibUFEM > SpalartAllmaras_builder;

SpalartAllmaras::SpalartAllmaras(const std::string& name) :
  LSSActionUnsteady(name)
{
  // TODO: Move this to the physical model
  options().add("SA_constant_cb1", 0.1355)
    .description("SA_constant_cb1")
    .pretty_name("SA_constant_cb1")
    .link_to(&cb1);

  options().add("SA_constant_cb2", 0.622)
    .description("SA_constant_cb2")
    .pretty_name("SA_constant_cb2")
    .link_to(&cb2);

  options().add("SA_constant_cw1", 3.2390678167757287)
    .description("SA_constant_cw1")
    .pretty_name("SA_constant_cw1")
    .link_to(&cw1);

  options().add("SA_constant_cw2", 0.3)
    .description("SA_constant_cw2")
    .pretty_name("SA_constant_cw2")
    .link_to(&cw2);

  options().add("SA_constant_cw3", 2.0)
    .description("SA_constant_cw3")
    .pretty_name("SA_constant_cw3")
    .link_to(&cw3);

  options().add("SA_constant_cv1", 7.1)
    .description("SA_constant_cv1")
    .pretty_name("SA_constant_cv1")
    .link_to(&cv1);

  options().add("SA_constant_ct3", 1.2)
    .description("SA_constant_ct3")
    .pretty_name("SA_constant_ct3")
    .link_to(&ct3);

  options().add("SA_constant_ct4", 0.5)
    .description("SA_constant_ct4")
    .pretty_name("SA_constant_ct4")
    .link_to(&ct4);

  options().add("SA_constant_kappa", 0.41)
    .description("SA_constant_kappa")
    .pretty_name("SA_constant_kappa")
    .link_to(&kappa);

  options().add("SA_constant_sigma", 0.66666666666666666666)
    .description("SA_constant_sigma")
    .pretty_name("SA_constant_sigma")
    .link_to(&sigma);

  options().add("SA_constant_r", 1.)
    .description("SA_constant_r")
    .pretty_name("SA_constant_r")
    .link_to(&r);

  options().add("SA_constant_g", 1.)
    .description("SA_constant_g")
    .pretty_name("SA_constant_g")
    .link_to(&g);

  options().add("SA_constant_d", 1.)
    .description("SA_constant_d")
    .pretty_name("SA_constant_d")
    .link_to(&d);


  options().option(solver::Tags::physical_model()).attach_trigger(boost::bind(&SpalartAllmaras::trigger_physical_model, this));

  // The code will only be active for these element types
  boost::mpl::vector2<mesh::LagrangeP1::Line1D,mesh::LagrangeP1::Quad2D> allowed_elements;

  set_solution_tag("spalart_allmaras_solution");

  MeshTerm<0, ScalarField> NU("TurbulentViscosity", solution_tag());
  MeshTerm<1, VectorField> u_adv("AdvectionVelocity","linearized_velocity");
  MeshTerm<2, VectorField> u("Velocity","velocity");

  *this
    << allocate_component<ZeroLSS>("ZeroLSS")
    << create_proto_action
    (
      "SpecializedAssembly",
      elements_expression
      (
        // specialized_elements,
        allowed_elements,
        group
        (
          _A = _0, _T = _0,
          UFEM::compute_tau(u_adv, m_coeffs),
          element_quadrature
          (
            _A(NU) += transpose(N(NU)) * u_adv * nabla(NU) + m_coeffs.tau_su * transpose(u_adv*nabla(NU)) * u_adv * nabla(NU)                 // advection term
               + cb1 * transpose(N(NU)) * N(NU) * ( (1.) +  (NU / (kappa * kappa * d * d)) *  // for 1.: vorticity magnitude is missing due to error message : (nabla(u) - transpose(nabla(u)))
               (1 - ((NU/m_coeffs.mu)/(1+(NU/m_coeffs.mu)*((NU/m_coeffs.mu)*(NU/m_coeffs.mu)*(NU/m_coeffs.mu))/(cv1+((NU/m_coeffs.mu)*(NU/m_coeffs.mu)*(NU/m_coeffs.mu)))))))
               + ((transpose(N(NU)) * N(NU) * NU ) / (d*d)) * cw1 * r * ((1 + cw3*cw3*cw3*cw3*cw3*cw3 )/(r + cw3*cw3*cw3*cw3*cw3*cw3 ))      // ^(1/6) is missing due to error message, wall distance, r
               - (1/sigma) * ((NU + m_coeffs.mu) * transpose(nabla(NU)) * nabla(NU))
                       - (1/sigma) * (cb2) * transpose(N(NU)) * transpose(nabla(NU) * nodal_values(NU))*nabla(NU),                                                      // should be nabla(NU)^2
            _T(NU,NU) +=  transpose(N(NU) + m_coeffs.tau_su * u_adv * nabla(NU)) * N(NU)                                                      // Time, standard and SUPG
          ),
          system_matrix += invdt() * _T + 1.0 * _A,
          system_rhs += -_A * _b
        )
      )
    )
    << allocate_component<BoundaryConditions>("BoundaryConditions")
    << allocate_component<SolveLSS>("SolveLSS")
    << create_proto_action("Update", nodes_expression(NU += solution(NU)));

  get_child("BoundaryConditions")->handle<BoundaryConditions>()->set_solution_tag(solution_tag());

}

void SpalartAllmaras::trigger_physical_model()
{
  dynamic_cast<NavierStokesPhysics&>(physical_model()).link_properties(m_coeffs);
}

void SpalartAllmaras::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  initial_conditions.create_initial_condition(solution_tag());
}


} // UFEM
} // cf3
