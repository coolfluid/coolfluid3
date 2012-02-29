// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "NavierStokes.hpp"

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

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

ComponentBuilder < NavierStokes, solver::Solver, LibUFEM > NavierStokes_builder;

NavierStokes::NavierStokes(const std::string& name) :
  LinearSolverUnsteady(name)
{
  options().add_option("initial_pressure", 0.)
    .description("Initial condition for the pressure")
    .pretty_name("Initial pressure")
    .link_to(&m_p0);

  options().add_option< std::vector<Real> >("initial_velocity")
    .description("Initial condition for the velocity")
    .pretty_name("Initial velocity")
    .attach_trigger(boost::bind(&NavierStokes::trigger_u, this));

  options().add_option<Real>("reference_velocity")
    .description("Reference velocity for the calculation of the stabilization coefficients")
    .pretty_name("Reference velocity")
    .link_to(&m_coeffs.u_ref);

  options().add_option("density", 1.2)
    .description("Mass density (kg / m^3)")
    .pretty_name("Density")
    .link_to(&m_coeffs.rho)
    .attach_trigger(boost::bind(&NavierStokes::trigger_rho, this));

  options().add_option("dynamic_viscosity", 1.7894e-5)
    .description("Dynamic Viscosity (kg / m s)")
    .pretty_name("Dynamic Viscosity")
    .link_to(&m_coeffs.mu);

  // For these elements, faster, specialized code exists
  boost::mpl::vector2<mesh::LagrangeP1::Triag2D, mesh::LagrangeP1::Tetra3D> specialized_elements;

  MeshTerm<0, ScalarField> p("Pressure", Tags::solution());
  MeshTerm<1, VectorField> u("Velocity", Tags::solution());

  MeshTerm<2, VectorField> u_adv("AdvectionVelocity", "linearized_velocity"); // The extrapolated advection velocity (n+1/2)
  MeshTerm<3, VectorField> u1("AdvectionVelocity1", "linearized_velocity");  // Two timesteps ago (n-1)
  MeshTerm<4, VectorField> u2("AdvectionVelocity2", "linearized_velocity"); // n-2
  MeshTerm<5, VectorField> u3("AdvectionVelocity3", "linearized_velocity"); // n-3

  boost::shared_ptr<solver::actions::Iterate> time_loop = allocate_component<solver::actions::Iterate>("TimeLoop");
  time_loop->create_component<solver::actions::CriterionTime>("CriterionTime");

  *this
    << create_proto_action("Initialize", nodes_expression(group
    (
      p = m_p0,
      u = m_u0,
      u1 = u,
      u2 = u,
      u3 = u
    )))
    <<
    ( // Time loop
      time_loop
      << allocate_component<ZeroLSS>("ZeroLSS")
      << create_proto_action("LinearizeU", nodes_expression(u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3))
      << create_proto_action
      (
        "GenericAssembly",
        generic_ns_assembly(*this, m_coeffs)
      )
      << create_proto_action
      (
        "SpecializedAssembly",
        elements_expression
        (
          specialized_elements,
          group
          (
            _A(p) = _0, _A(u) = _0, _T(p) = _0, _T(u) = _0,
            supg_specialized(p, u, u_adv, m_coeffs, _A, _T),
            system_matrix += invdt() * _T + 1.0 * _A,
            system_rhs += -_A * _b
          )
        )
      )
      << allocate_component<BoundaryConditions>("BoundaryConditions")
      << allocate_component<SolveLSS>("SolveLSS")
      << create_proto_action("Update", nodes_expression(group
      (
        u3 = u2,
        u2 = u1,
        u1 = u,
        u += solution(u),
        p += solution(p)
      )))
      << allocate_component<solver::actions::AdvanceTime>("AdvanceTime")
    );
}

void NavierStokes::trigger_rho()
{
  m_coeffs.one_over_rho = 1. / options().option("density").value<Real>();
}

void NavierStokes::trigger_u()
{
  std::vector<Real> u_vec = options().option("initial_velocity").value< std::vector<Real> >();

  const Uint nb_comps = u_vec.size();
  m_u0.resize(nb_comps);
  for(Uint i = 0; i != nb_comps; ++i)
    m_u0[i] = u_vec[i];
}


} // UFEM
} // cf3
