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

#include "NavierStokesPhysics.hpp"

namespace cf3 {

namespace UFEM {

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

/// Possible types of SUPG computation to apply
class UFEM_API SUPGTypes
{
  public:

  /// Enumeration of the worker statuses recognized in CF
  enum Type  { 
               INVALID     =-1,
               // Definition from Tezduyar et al.
               TEZDUYAR    = 0,
               // definition from i.e. Trofimova et al. (Direct numerical simulation of turbulent channel flows using a stabilized finite element method Computers & Fluids, 2009, 38, 924-938)
               METRIC      = 1,
               // Implementation from Coolfluid 2
               CF2         = 2
             };

  typedef common::EnumT< SUPGTypes > ConverterBase;

  struct UFEM_API Convert : public ConverterBase
  {
    /// constructor where all the converting maps are built
    Convert();
    /// get the unique instance of the converter class
    static Convert& instance();
  };
};

/// Calculation of the stabilization coefficients for the SUPG method
struct ComputeTauImpl : boost::noncopyable
{
  ComputeTauImpl() :
    alpha_ps(1.),
    alpha_su(1.),
    alpha_bu(1.),
    supg_type(SUPGTypes::METRIC),
    supg_type_str("metric"),
    c1(1.),
    c2(16.),
    u_ref(1.)
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

  // Ignore lement-based variants
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

    if(supg_type == SUPGTypes::TEZDUYAR)
    {
      // Get the minimal edge length
      Real h_rgn = 1e10;
      for(Uint i = 0; i != ElementT::nb_nodes; ++i)
      {
        for(Uint j = 0; j != ElementT::nb_nodes; ++j)
        {
          if(i != j)
          {
            h_rgn = std::min(h_rgn, (u.support().nodes().row(i) - u.support().nodes().row(j)).squaredNorm());
          }
        }
      }
      h_rgn = sqrt(h_rgn);

      const Real umag = detail::norm(u.eval());
      const Real h_ugn = h_rgn;//fabs(2.*umag / (u.eval()*u.nabla()).sum());

      const Real tau_adv_inv = (2.*umag)/h_ugn;
      const Real tau_time_inv = 2./dt;
      const Real tau_diff_inv = (4.*element_nu)/(h_rgn*h_rgn);

      tau_su = 1./(tau_adv_inv + tau_time_inv + tau_diff_inv);
      tau_ps = tau_su;
      tau_bulk = tau_su*umag*umag;
    }
    else if(supg_type == SUPGTypes::METRIC)
    {
      u.support().compute_jacobian(GaussT::instance().coords.col(0));
      
      const Eigen::Matrix<Real, ElementT::dimensionality, ElementT::dimensionality> gij = u.support().jacobian_inverse().transpose() * u.support().jacobian_inverse();
      const Real tau_adv_sq = fabs((u.eval()*gij*detail::transpose(u.eval()))[0]); // Very close 0 but slightly negative sometimes
      const Real tau_diff = element_nu*element_nu*gij.squaredNorm();

      tau_su = 1. / sqrt((4.*c1*c1/(dt*dt)) + tau_adv_sq + c2*tau_diff);
      tau_ps = tau_su;
      
      // Use the standard SUPG factor to compute the bulk viscosity, or it goes up way too much
      // const Real tau_su_std = 1. / sqrt((4./(dt*dt)) + tau_adv_sq + 16.*tau_diff);
      tau_bulk = (1./tau_su) / gij.trace();
    }
    else if(supg_type == SUPGTypes::CF2)
    {
      // Get the maximal edge length
      Real he = 0.0;
      for (Uint i = 0; i != ElementT::nb_nodes; ++i)
      {
        for (Uint j = 0; j != ElementT::nb_nodes; ++j)
        {
          if (i != j)
          {
            he = std::max(he, (u.support().nodes().row(i) - u.support().nodes().row(j)).squaredNorm());
          }
        }
      }
      he = sqrt(he);

      const Real tau_ps_diff = he*he/(4*element_nu);
      const Real tau_ps_adv = he / (2*u_ref);
      tau_ps = 1.0/sqrt(1.0/(tau_ps_diff*tau_ps_diff) + 1.0/(tau_ps_adv*tau_ps_adv));
      
      tau_su = 0.0;
      const Real umag = detail::norm(u.eval());
      if(umag > 1e-10)
      {
        const Real h = he;//2.*umag / (u.eval()*u.nabla()).sum();
        const Real tau_adv = h/(2.*umag);
        const Real tau_time = 0.5*dt;
        const Real tau_diff = tau_ps_diff;
        tau_su = 1.0/sqrt(1.0/(tau_adv*tau_adv) + 1.0/(tau_time*tau_time) + 1.0/(tau_diff*tau_diff));
      }
      tau_bulk = tau_ps * std::max(umag*umag, u_ref*u_ref);
    }
    else
    {
      throw common::ShouldNotBeHere(FromHere(), "Unsupported SUPG method chosen");
    }
      
    tau_ps *= alpha_ps;
    tau_su *= alpha_su;
    tau_bulk *= alpha_bu;
  }
  
  void trigger_supg_type()
  {
    supg_type = SUPGTypes::Convert::instance().to_enum(supg_type_str);
  }

  Real alpha_ps, alpha_su, alpha_bu;
  SUPGTypes::Type supg_type;
  std::string supg_type_str;
  
  // Constants for the METRIC method
  Real c1,c2;
  
  // Reference velocity for the CF2 method
  Real u_ref;
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


} // UFEM
} // cf3


#endif // cf3_UFEM_SUPG_hpp
