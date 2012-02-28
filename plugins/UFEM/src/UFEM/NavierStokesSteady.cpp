// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/count.hpp>

#define BOOST_PROTO_MAX_ARITY 10
#ifdef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
#undef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
#endif
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

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
#include "solver/actions/Criterion.hpp"
#include "solver/Time.hpp"

#include "NavierStokesSteady.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

ComponentBuilder < NavierStokesSteady, Solver, LibUFEM > NavierStokesSteady_builder;

struct SteadyNSCriterion : Criterion
{
  SteadyNSCriterion(const std::string& name) : Criterion(name)
  {
    properties().add_property("p_update_history", p_update_history);
    properties().add_property("u_update_history", u_update_history);

    p_update_history.reserve(100000);
    u_update_history.reserve(100000);

    options().add_option("p_convergence", 1e-10)
      .pretty_name("Pressure convergence")
      .description("Maximum update over the whole field for the pressure")
      .link_to(&max_p_update);

    options().add_option("u_convergence", 1e-10)
      .pretty_name("Velocity convergence")
      .description("Maximum update over the whole field for the velocity")
      .link_to(&max_u_update);;
  }

  static std::string type_name() { return "SteadyNSCriterion"; }

  virtual bool operator()()
  {
    if(boost::accumulators::count(p_stats) < 1)
      return false;

    p_update_history.push_back(boost::accumulators::max(p_stats));
    u_update_history.push_back(boost::accumulators::max(u_stats));

    u_stats = StatsT();
    p_stats = StatsT();

    if(p_update_history.back() < max_p_update && u_update_history.back() < max_u_update)
    {
      properties()["p_update_history"] = p_update_history;
      properties()["u_update_history"] = u_update_history;

      return true;
    }

    return false;
  }

  Real max_p_update, max_u_update;

  /// Convergence statistics
  typedef boost::accumulators::accumulator_set< Real, boost::accumulators::stats<boost::accumulators::tag::max, boost::accumulators::tag::count> > StatsT;
  StatsT u_stats, p_stats;
  std::vector<Real> u_update_history, p_update_history;
};

ComponentBuilder < SteadyNSCriterion, Criterion, LibUFEM > SteadyNSCriterion_builder;

NavierStokesSteady::NavierStokesSteady(const std::string& name) :
  LinearSolver(name), m_p_under_relaxation(0.25), m_u_under_relaxation(0.25)
{
  options().add_option("initial_pressure", 0.)
    .description("Initial condition for the pressure")
    .pretty_name("Initial pressure")
    .link_to(&m_p0);

  options().add_option< std::vector<Real> >("initial_velocity")
    .description("Initial condition for the velocity")
    .pretty_name("Initial velocity")
    .attach_trigger(boost::bind(&NavierStokesSteady::trigger_u, this));

  options().add_option<Real>("reference_velocity")
    .description("Reference velocity for the calculation of the stabilization coefficients")
    .pretty_name("Reference velocity")
    .link_to(&m_coeffs.u_ref);

  options().add_option("density", 1.2)
    .description("Mass density (kg / m^3)")
    .pretty_name("Density")
    .link_to(&m_coeffs.rho)
    .attach_trigger(boost::bind(&NavierStokesSteady::trigger_rho, this));

  options().add_option("dynamic_viscosity", 1.7894e-5)
    .description("Dynamic Viscosity (kg / m s)")
    .pretty_name("Dynamic Viscosity")
    .link_to(&m_coeffs.mu);

  options().add_option("p_under_relaxation", m_p_under_relaxation).pretty_name("Pressure Under Relaxation").link_to(&m_p_under_relaxation);
  options().add_option("u_under_relaxation", m_u_under_relaxation).pretty_name("Velocity Under Relaxation").link_to(&m_u_under_relaxation);

  boost::shared_ptr<solver::actions::Iterate> iteration_loop = allocate_component<solver::actions::Iterate>("Iteration");
  Handle<SteadyNSCriterion> criterion = iteration_loop->create_component<SteadyNSCriterion>("ConvergenceCriterion");

  boost::mpl::vector2<mesh::LagrangeP1::Triag2D, mesh::LagrangeP1::Quad2D> allowed_elements;

  MeshTerm<0, VectorField> u("Velocity", Tags::solution());
  MeshTerm<1, ScalarField> p("Pressure", Tags::solution());

  MeshTerm<2, VectorField> u_adv("AdvectionVelocity", "linearized_velocity"); // The extrapolated advection velocity (n+1/2)
  MeshTerm<3, VectorField> u1("AdvectionVelocity1", "linearized_velocity");  // Two timesteps ago (n-1)
  MeshTerm<4, VectorField> u2("AdvectionVelocity2", "linearized_velocity"); // n-2
  MeshTerm<5, VectorField> u3("AdvectionVelocity3", "linearized_velocity"); // n-3

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
    ( // inner iterative loop
      iteration_loop
      << allocate_component<ZeroLSS>("ZeroLSS")
      << create_proto_action("LinearizeU", nodes_expression(u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3))
      << create_proto_action
      (
        "Assembly",
        elements_expression
        (
          allowed_elements,
          group
          (
            _A = _0,
            compute_tau(u, m_coeffs),
            element_quadrature
            (
              _A(p    , u[_i]) +=          transpose(N(p))       * nabla(u)[_i] + m_coeffs.tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(u), // Standard continuity + PSPG for advection
              _A(p    , p)     += m_coeffs.tau_ps * transpose(nabla(p))     * nabla(p) * m_coeffs.one_over_rho,     // Continuity, PSPG
              _A(u[_i], u[_i]) += m_coeffs.mu     * transpose(nabla(u))     * nabla(u) * m_coeffs.one_over_rho     + transpose(N(u) + m_coeffs.tau_su*u_adv*nabla(u)) * u_adv*nabla(u),     // Diffusion + advection
              _A(u[_i], p)     += m_coeffs.one_over_rho * transpose(N(u) + m_coeffs.tau_su*u_adv*nabla(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
              _A(u[_i], u[_j]) += transpose((m_coeffs.tau_bulk + 0.33333333333333*boost::proto::lit(m_coeffs.mu)*m_coeffs.one_over_rho)*nabla(u)[_i] // Bulk viscosity and second viscosity effect
                                             + 0.5*u_adv[_i]*(N(u) + m_coeffs.tau_su*u_adv*nabla(u))) * nabla(u)[_j]  // skew symmetric part of advection (standard +SUPG)
            ),
            system_matrix += _A,
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
        u += m_u_under_relaxation*solution(u),
        p += m_p_under_relaxation*solution(p),
        boost::proto::lit(criterion->p_stats)(_abs(solution(p))),
        boost::proto::lit(criterion->u_stats)(_norm(solution(u)))
      )))
    );
}

void NavierStokesSteady::trigger_rho()
{
  m_coeffs.one_over_rho = 1. / options().option("density").value<Real>();
}

void NavierStokesSteady::trigger_u()
{
  std::vector<Real> u_vec = options().option("initial_velocity").value< std::vector<Real> >();

  const Uint nb_comps = u_vec.size();
  m_u0.resize(nb_comps);
  for(Uint i = 0; i != nb_comps; ++i)
    m_u0[i] = u_vec[i];
}

} // UFEM
} // cf3
