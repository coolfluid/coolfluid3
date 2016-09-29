// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_SUPG_hpp
#define cf3_UFEM_SUPG_hpp

#include "common/EnumT.hpp"

#include "math/MatrixTypes.hpp"

#include "solver/actions/Proto/ElementOperations.hpp"
#include "solver/actions/Proto/ElementData.hpp"

#include "../NavierStokesPhysics.hpp"

namespace cf3 {

namespace UFEM {

namespace compressible {

namespace detail
{

// Helper to get the norm of either a vector or a scalar
template<typename T>
inline Real norm(const T& vector)
{
  return vector.norm();
}

inline Real norm(const Real scalar)
{
  return scalar;
}

// Helper to get the mean of either a vector or a scalar
template<typename T>
inline Real mean(const T& vector)
{
  return vector.mean();
}

inline Real mean(const Real scalar)
{
  return scalar;
}

// Helper to get the transpose of either a vector or a scalar
template<typename T>
inline Eigen::Transpose<T const> transpose(const T& mat)
{
  return mat.transpose();
}

inline Real transpose(const Real val)
{
  return val;
}

}

/// Calculation of the stabilization coefficients for the SUPG method
struct ComputeTauImpl : boost::noncopyable
{
  ComputeTauImpl() :
    alpha_ps(1.),
    alpha_su(1.),
    alpha_bu(1.),
    c1(1.),
    c2(16.)
  {
  }

  typedef void result_type;

  /// Compute the coefficients for the full Navier-Stokes equations
  template<typename UT, typename NUT>
  void operator()(const UT& u, const NUT& nu_eff, const Real& dt, Real& tau_ps, Real& tau_su, Real& tau_bulk) const
  {
    // Average viscosity
    const Real element_nu = fabs(detail::mean(nu_eff.value()));

    compute_coefficients(u, element_nu, dt, tau_ps, tau_su, tau_bulk);
  }

  /// Only compute the SUPG coefficient
  template<typename UT, typename NUT>
  void operator()(const UT& u, const NUT& nu_eff, const Real& dt, Real& tau_su) const
  {
    Real tau_ps, tau_bu;
    compute_coefficients(u, fabs(detail::mean(nu_eff.value())), dt, tau_ps, tau_su, tau_bu);
  }

  /// Only compute the SUPG coefficient, overload for scalar viscosity
  template<typename UT>
  void operator()(const UT& u, const Real& nu_eff, const Real& dt, Real& tau_su) const
  {
    Real tau_ps, tau_bu;
    compute_coefficients(u, nu_eff, dt, tau_ps, tau_su, tau_bu);
  }

  // Ignore element-based variants
  template<typename SupportEtypeT, Uint Dim, bool IsEquationVar>
  void compute_coefficients(const solver::actions::Proto::EtypeTVariableData<solver::actions::Proto::ElementBased<Dim>, SupportEtypeT, Dim, IsEquationVar>& u, const Real element_nu, const Real& dt, Real& tau_ps, Real& tau_su, Real& tau_bulk) const
  {
  }

  template<typename UT>
  void compute_coefficients(const UT& u, const Real element_nu, const Real& dt, Real& tau_ps, Real& tau_su, Real& tau_bulk) const
  {
    typedef typename UT::EtypeT ElementT;
    static const Uint dim = ElementT::dimension;
    typedef mesh::Integrators::GaussMappedCoords<1, ElementT::shape> GaussT;

    u.compute_values(GaussT::instance().coords.col(0));


    u.support().compute_jacobian(GaussT::instance().coords.col(0));

    const Eigen::Matrix<Real, ElementT::dimensionality, ElementT::dimensionality> gij = u.support().jacobian_inverse().transpose() * u.support().jacobian_inverse();
    const Real tau_adv_sq = fabs((u.eval()*gij*detail::transpose(u.eval()))[0]); // Very close 0 but slightly negative sometimes
    const Real tau_diff = element_nu*element_nu*gij.squaredNorm();

    tau_su = 1. / sqrt((4.*c1*c1/(dt*dt)) + tau_adv_sq + c2*tau_diff);
    tau_ps = tau_su;

    // Use the standard SUPG factor to compute the bulk viscosity, or it goes up way too much
    const Real tau_su_std = 1. / sqrt((4./(dt*dt)) + tau_adv_sq + 16.*tau_diff);
    tau_bulk = (1./tau_su_std) / gij.trace();

    tau_ps *= alpha_ps;
    tau_su *= alpha_su;
    tau_bulk *= alpha_bu;
  }

  Real alpha_ps, alpha_su, alpha_bu;

  // Constants for the METRIC method
  Real c1,c2;
};

/// Convenience type for a compute_tau operation, grouping the stored operator and its proto counterpart
struct ComputeTau
{
  ComputeTau() :
    apply(boost::proto::as_child(data))
  {
  }

  // Stores the operator
  solver::actions::Proto::MakeSFOp<ComputeTauImpl>::stored_type data;

  // Use as apply(velocity_field, nu_eff_field, dt, tau_ps, tau_su, tau_bulk)
  solver::actions::Proto::MakeSFOp<ComputeTauImpl>::reference_type apply;
};

} // compressible
} // UFEM
} // cf3


#endif // cf3_UFEM_SUPG_hpp
