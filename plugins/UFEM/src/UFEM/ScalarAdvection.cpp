// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "ScalarAdvection.hpp"

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

#include "Tags.hpp"

#include "UFEM/NavierStokesOps.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

ComponentBuilder < ScalarAdvection, solver::Solver, LibUFEM > ScalarAdvection_builder;

ScalarAdvection::ScalarAdvection(const std::string& name) :
  LinearSolverUnsteady(name)
{
  options().add_option("initial_scalar", 0.)
    .description("Initial condition for the scalar")
    .pretty_name("Initial scalar")
    .link_to(&m_Phi0);

  options().add_option<Real>("reference_velocity")
    .description("Reference velocity for the calculation of the stabilization coefficients")
    .pretty_name("Reference velocity")
    .link_to(&m_coeffs.u_ref);

  options().add_option("density", 1.2)
    .description("Mass density (kg / m^3)")
    .pretty_name("Density")
    .link_to(&m_coeffs.rho)
    .attach_trigger(boost::bind(&ScalarAdvection::trigger_rho, this));

  options().add_option("dynamic_viscosity", 1.7894e-5)
    .description("Dynamic Viscosity (kg / m s)")
    .pretty_name("Dynamic Viscosity")
    .link_to(&m_coeffs.mu);

  options().add_option("scalar_coefficient", 1.)
    .description("Scalar coefficient ")
    .pretty_name("Scalar coefficient")
    .link_to(&m_alpha);

  options().add_option< std::vector<Real> >("initial_velocity")
    .description("Initial condition for the velocity")
    .pretty_name("Initial velocity")
    .attach_trigger(boost::bind(&ScalarAdvection::trigger_u, this));

  // For these elements, faster, specialized code exists
  boost::mpl::vector2<mesh::LagrangeP1::Line1D,mesh::LagrangeP1::Quad2D> allowed_elements;

  m_solution_tag = "scalar_advection_solution";

  MeshTerm<0, ScalarField> Phi("Scalar", m_solution_tag);
  MeshTerm<1, VectorField> u_adv("AdvectionVelocity","linearized_velocity");

  boost::shared_ptr<solver::actions::Iterate> time_loop = allocate_component<solver::actions::Iterate>("TimeLoop");
  time_loop->create_component<solver::actions::CriterionTime>("CriterionTime");

  *this
    << create_proto_action("InitializeScalar", nodes_expression(Phi = m_Phi0))
    << create_proto_action("InitializeVelocity", nodes_expression(u_adv = m_u0))
    <<
    ( // Time loop
      time_loop
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
            element_quadrature( _A(Phi) += transpose(N(Phi)) * u_adv * nabla(Phi) + m_coeffs.tau_su * transpose(u_adv*nabla(Phi))  * u_adv * nabla(Phi) +  m_alpha * transpose(nabla(Phi)) * nabla(Phi) ,
                  _T(Phi,Phi) +=  transpose(N(Phi) + m_coeffs.tau_su * u_adv * nabla(Phi)) * N(Phi) ),
            system_matrix += invdt() * _T + 1.0 * _A,
            system_rhs += -_A * _b
          )
        )
      )
      << allocate_component<BoundaryConditions>("BoundaryConditions")
      << allocate_component<SolveLSS>("SolveLSS")
      << create_proto_action("Update", nodes_expression(Phi += solution(Phi)))
      << allocate_component<solver::actions::AdvanceTime>("AdvanceTime")

    );

  time_loop->get_child("BoundaryConditions")->handle<BoundaryConditions>()->set_solution_tag(m_solution_tag);

}

void ScalarAdvection::trigger_rho()
{
  m_coeffs.one_over_rho = 1. / options().option("density").value<Real>();
}

void ScalarAdvection::trigger_u()
{
  std::vector<Real> u_vec = options().option("initial_velocity").value< std::vector<Real> >();

  const Uint nb_comps = u_vec.size();
  m_u0.resize(nb_comps);
  for(Uint i = 0; i != nb_comps; ++i)
    m_u0[i] = u_vec[i];
}


} // UFEM
} // cf3
