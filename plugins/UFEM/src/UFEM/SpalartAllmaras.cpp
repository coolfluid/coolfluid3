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
using namespace boost::proto;

ComponentBuilder < SpalartAllmaras, LSSActionUnsteady, LibUFEM > SpalartAllmaras_builder;

/// Minimum between two scalars
inline Real min(const Real a, const Real b)
{
  return a < b ? a : b;
}
/// Wraps around the min function for the element matrix
static boost::proto::terminal< double(*)(double, double) >::type const _min = {&min};

/// Any value to the power of 6
inline Real ttpo6(const Real a)
{
  return a*a*a*a*a*a;
}
/// Wraps around the ttpo6 (to the power of 6) function for the element matrix
static boost::proto::terminal< double(*)(double) >::type const _ttpo6 = {&ttpo6};

/// Wraps around the std::pow function for the element matrix
static boost::proto::terminal< double(*)(double, double) >::type const _pow = {&::pow};

inline Real fv1(const Real chi, const Real cv1)
{
    return chi*chi*chi / (chi*chi*chi + cv1*cv1*cv1);
}

static boost::proto::terminal< double(*)(double, double) >::type const _fv1 = {&fv1};

struct ComputeSACoeffs
{
  typedef void result_type;

  template<typename UT, typename NUT>
  void operator()(const UT& u, const NUT& nu_t, SACoeffs& coeffs, const SUPGCoeffs& c) const
  {

    // nu_t.value() is a column vector with the nodal values of the viscosity for the element.
    // mean comes from the Eigen library
    const Real nu_t_cell = nu_t.value().mean();
    //coeffs.nu_t_cell = nu_t.value().mean();
    const Real nu_lam = c.one_over_rho*c.mu;
    coeffs.chi = nu_t_cell / nu_lam;

    coeffs.Kappa = 0.41;

    // Computing S needs the gradient, which is calculated at a mapped coordinate.
    // We take the first gauss point here, i.e. we approximate the gradient by the value at the cell center.
    typedef mesh::Integrators::GaussMappedCoords<1, UT::SupportT::EtypeT::shape> GaussT; // the type of the Gauss point source
    // nabla is the gradient matrix of the shape function of u
    const typename UT::GradientT& nabla_mat = u.nabla(GaussT::instance().coords); // access the gauss point, in this case (0.5, 0.5) for triangles and tetras and (0., 0.) otherwise
    Eigen::Matrix<Real, UT::dimension, UT::dimension> nabla_u = nabla_mat * u.value(); // The gradient of the velocity is the shape function gradient matrix multiplied with the nodal values
    // wall distance
    // const Real d = u.support().coordinates(GaussT::instance().coords)[1]; // y-coordinate of the cell center
    coeffs.D = u.support().coordinates(GaussT::instance().coords)[1]; // y-coordinate of the cell center
    coeffs.omega = sqrt(2.)*0.5*(nabla_u - nabla_u.transpose()).norm();
    coeffs.Fv1 = fv1(coeffs.chi, 7.1);
    coeffs.Fv2 = 1 - (coeffs.chi/(1+coeffs.chi*coeffs.Fv1));
    coeffs.shat = coeffs.omega + (nu_t_cell * coeffs.Fv2)/(coeffs.Kappa*coeffs.Kappa*coeffs.D*coeffs.D);
    if(coeffs.shat < 0.3*coeffs.omega)
    {
      coeffs.shat = 0.3*coeffs.omega;
    }
    coeffs.one_over_D = 1/coeffs.D;
    coeffs.one_over_D_squared = 1/(coeffs.D*coeffs.D);
    coeffs.one_over_Kappa = 1/coeffs.Kappa;
    coeffs.one_over_Kappa_squared = 1/(coeffs.Kappa*coeffs.Kappa);
    coeffs.one_over_shat = 1/coeffs.shat;
    coeffs.one_over_KappaD_squared = 1/(coeffs.D * coeffs.D * coeffs.Kappa * coeffs.Kappa);
    // Average cell velocity
    //const typename ElementT::CoordsT u_avg = u.value().colwise().mean();
  }
};

/// Placeholder for the compute_tau operation
static solver::actions::Proto::MakeSFOp<ComputeSACoeffs>::type const compute_sa_coeffs = {};

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

 // options().add("SA_constant_d", 1.)
 //   .description("SA_constant_d")
 //   .pretty_name("SA_constant_d")
 //   .link_to(&d);

 // options().add("SA_constant_S", 1.)
 //   .description("SA_constant_S")
 //   .pretty_name("SA_constant_S")
 //   .link_to(&S);

  options().option(solver::Tags::physical_model()).attach_trigger(boost::bind(&SpalartAllmaras::trigger_physical_model, this));

  // The code will only be active for these element types
  boost::mpl::vector3<mesh::LagrangeP1::Line1D,mesh::LagrangeP1::Quad2D,mesh::LagrangeP1::Triag2D> allowed_elements;

  set_solution_tag("spalart_allmaras_solution");

  FieldVariable<0, ScalarField> NU("TurbulentViscosity", solution_tag());
  FieldVariable<1, VectorField> u_adv("AdvectionVelocity","linearized_velocity");
  FieldVariable<2, VectorField> u("Velocity","navier_stokes_solution");
  FieldVariable<3, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity"); // This is the viscosity that needs to be modified to be visible in NavierStokes


//  FieldVariable<3, ScalarField> d("Walldistance","walldistance");

  //fw = g * _pow(((1+_pow(cw3,6))/(_pow(g,6)+_pow(cw3,6))),1/6)

  // g = (r + cw2*(_pow(r,6)-r))

  // r = (_min(10, ((NU)/(kappa*kappa*d*d*S)) ))

  //S = ( ( _norm(nabla(u) * nodal_values(u) - transpose(nabla(u) * nodal_values(u)) ) ) +  (NU / (kappa * kappa * d * d)) *
  //(1 - ((NU/m_su_coeffs.mu)/(1+(NU/m_su_coeffs.mu)*((NU/m_su_coeffs.mu)*(NU/m_su_coeffs.mu)*(NU/m_su_coeffs.mu))/(cv1+((NU/m_su_coeffs.mu)*(NU/m_su_coeffs.mu)*(NU/m_su_coeffs.mu)))))))

  *this
    << allocate_component<ZeroLSS>("ZeroLSS")
//    << create_proto_action("set_wall_distance", nodes_expression(d=coordinates[1]))
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
                        UFEM::compute_tau(u_adv, m_su_coeffs),
                        compute_sa_coeffs(u_adv, NU, m_sa_coeffs, m_su_coeffs),
                        element_quadrature
                        (
                           _A(NU) +=
                             transpose(N(NU)) * u_adv * nabla(NU) + m_su_coeffs.tau_su * transpose(u_adv*nabla(NU)) * u_adv * nabla(NU)                             // advection terms
                             + cb1 * transpose(N(NU)) * N(NU) *  m_sa_coeffs.shat                                                                                   // production

                             + cw1*((transpose(N(NU))*N(NU) * NU) * m_sa_coeffs.one_over_D_squared) * ((_min(10, ((NU)*(m_sa_coeffs.one_over_KappaD_squared*m_sa_coeffs.shat )) )) + cw2*(_pow((_min(10, ((NU)*(m_sa_coeffs.one_over_KappaD_squared*m_sa_coeffs.shat)) )),6)      // cw1 * fw * (NU_hat/d)^2
                             -(_min(10, ((NU)*(m_sa_coeffs.one_over_KappaD_squared*m_sa_coeffs.shat)) )))) * _pow(((1+_pow(cw3,6))/(_pow(((_min(10, ((NU)*(m_sa_coeffs.one_over_KappaD_squared*m_sa_coeffs.shat)) )) + cw2*(_pow((_min(10, ((NU)*(m_sa_coeffs.one_over_KappaD_squared*
                             m_sa_coeffs.shat)) )),6)-(_min(10, ((NU)*(m_sa_coeffs.one_over_KappaD_squared*m_sa_coeffs.shat)) )))),6)+_pow(cw3,6))),1/6)            // destruction

                             + (1/sigma) * ((NU + m_su_coeffs.mu) * transpose(nabla(NU)) * nabla(NU))                                                               // diffusion: (NU+NU_hat) partial NU_hat to xj to xj
                             + (1/sigma) * (cb2) * transpose(N(NU)) * transpose(nabla(NU) * nodal_values(NU))*nabla(NU),                                            // diffusion: nabla(NU)^2 times the weight function
                           _T(NU,NU) +=  transpose(N(NU) + m_su_coeffs.tau_su * u_adv * nabla(NU)) * N(NU)                                                          // Time, standard and SUPG
                        ),
                      system_matrix += invdt() * _T + 1.0 * _A,
                      system_rhs += -_A * _x

 //            _cout << "_A:" << _A << "\n" << "NU:" << NU(gauss_points_1) << "\n" << "omega:" << m_sa_coeffs.omega << "\n" << "Fv1:" << m_sa_coeffs.Fv1 << "\n" << "Fv2:" << m_sa_coeffs.Fv2 << "\n" << "Kappa:" << m_sa_coeffs.Kappa << "\n" << "shat:" << m_sa_coeffs.shat << "\n"
 //            << "D:" << m_sa_coeffs.D << "\n"  << "coeffs.nu_t_cell:" << m_sa_coeffs.nu_t_cell << "\n" << "m_sa_coeffs.one_over_D:" << m_sa_coeffs.one_over_D << "\n" << "m_sa_coeffs.one_over_D_squared:" << m_sa_coeffs.one_over_D_squared << "\n"
 //            << "m_sa_coeffs.one_over_KappaD_squared:" << m_sa_coeffs.one_over_KappaD_squared << "\n"
                      )
      )
    )
    << allocate_component<BoundaryConditions>("BoundaryConditions")
    << allocate_component<SolveLSS>("SolveLSS")
    << create_proto_action("Update", nodes_expression(
       group(
         NU += solution(NU),                                                                                                                                           //update for Navier-Stokes solution
 //        nu_eff = lit(m_su_coeffs.mu) / m_su_coeffs.rho //lit comes from boost::proto
 //        nu_eff = (boost::proto::lit(m_su_coeffs.mu) / m_su_coeffs.rho) + boost::proto::lit(m_su_coeffs.rho) * NU * ( (m_sa_coeffs.chi) * (m_sa_coeffs.chi) * (m_sa_coeffs.chi)/( (7.1*7.1*7.1) + ((m_sa_coeffs.chi) * (m_sa_coeffs.chi) * (m_sa_coeffs.chi) )) )
         nu_eff = (lit(m_su_coeffs.mu) / m_su_coeffs.rho) + NU * _fv1(NU/lit(m_su_coeffs.mu)*m_su_coeffs.rho, 7.1)

       )));

    get_child("BoundaryConditions")->handle<BoundaryConditions>()->set_solution_tag(solution_tag());

}

void SpalartAllmaras::trigger_physical_model()
{
  dynamic_cast<NavierStokesPhysics&>(physical_model()).link_properties(m_su_coeffs);
}

void SpalartAllmaras::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  initial_conditions.create_initial_condition(solution_tag());
}


} // UFEM
} // cf3
