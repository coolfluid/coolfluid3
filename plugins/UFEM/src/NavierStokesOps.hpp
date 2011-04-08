// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_NavierStokesOps_hpp
#define CF_UFEM_NavierStokesOps_hpp

#include "Math/MatrixTypes.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Solver/Actions/Proto/ElementOperations.hpp"

namespace CF {
namespace UFEM {

/// Operation to set the different coefficients for PSPG, SUPG and bulk viscosity
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct SetTau;

/// 2D specialization
template<typename SF, Uint Offset, Uint MatrixSize>
struct SetTau<SF, 2, Offset, MatrixSize>
{ 
  /// Dummy result
  typedef int result_type;
  
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(const int, const SupportT& support, const VarDataT& field, StateT& state) const
  {
    const Real he=sqrt(4./3.141592654*support.volume());
    const Real ree=state.u_ref*he/(2.*state.nu);
    const Real xi=std::max(0.,std::min(ree/3.,1.));
    state.tau_ps = he*xi/(2.*state.u_ref);
    state.tau_bulk = he*state.u_ref/xi;
    
    // Average cell velocity
    const RealVector2 u = field.value().colwise().mean();
    const Real umag = u.norm();
    state.tau_su = 0.;
    if(umag > 1e-10)
    {
      const Real h = 2. * support.volume() / (support.nodes() * (u / umag)).array().abs().sum();
      Real ree=umag*h/(2.*state.nu);
      Real xi=std::max(0.,std::min(ree/3.,1.));
      state.tau_su = h*xi/(2.*umag);
    }
    
    //std::cout << "tau_ps: " << state.tau_ps << ", tau_su: " << state.tau_su << std::endl;
    
    // We need this one every time, so cache it
    state.integral_weight = Mesh::Integrators::GaussMappedCoords<1, SF::shape>::instance().weights[0]
                            * support.jacobian_determinant(Mesh::Integrators::GaussMappedCoords<1, SF::shape>::instance().coords.col(0));
    
    // Cache the matrices used in the operators, calculated at the first gauss point
    field.compute_values(Mesh::Integrators::GaussMappedCoords<1, SF::shape>::instance().coords.col(0), support);
    
    return 0;
  }
};

/// Pressure contribution to the continuity equation (PSPG of pressure gradient) for matrix A
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct ContinuityPressureA
{ 
  typedef Eigen::Matrix<Real, SF::nb_nodes, SF::nb_nodes> MatrixT;
  typedef const MatrixT& result_type;
  
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& field, StateT& state) const
  {
    // PSPG of the pressure gradient becomes a pressure laplacian
    matrix.noalias() = state.tau_ps / state.rho * field.gradient().transpose() * field.gradient() * state.integral_weight;
    return matrix;
  };
};

/// Velocity contribution to the continuity equation (divergence + PSPG of advective term) for matrix A
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct ContinuityVelocityA
{
  BOOST_MPL_ASSERT_RELATION(Dim, ==, SF::dimension);
  
  typedef Eigen::Matrix<Real, SF::nb_nodes, Dim*SF::nb_nodes> MatrixT;
  typedef const MatrixT& result_type;
  
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& field, StateT& state) const
  {
    for(Uint i = 0; i != SF::dimension; ++i)
    {
      matrix.template block<SF::nb_nodes, SF::nb_nodes>(0, i*SF::nb_nodes).noalias()
        = field.shape_function().transpose() * field.gradient().row(i) // Divergence (standard)
        + state.tau_ps * field.gradient().row(i).transpose() * field.advection() // PSPG for advection
        + 0.5*state.tau_ps * field.gradient().transpose() * field.eval().transpose() * field.gradient().row(i); // Skew symmetric
    }
    
    // Integration
    matrix *= state.integral_weight;
    
    return matrix;
  };
};

/// Pressure contribution to the momentum equation (standard + SUPG) for matrix A
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct MomentumPressureA
{ 
  typedef Eigen::Matrix<Real, SF::dimension*SF::nb_nodes, SF::nb_nodes> MatrixT;
  typedef const MatrixT& result_type;
  
  // We pass data for u here, to be able to calculate the SUPG terms. This implies the same shape function for pressure and velocity
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& field, StateT& state) const
  {
    // Standard + SUPG
    for(Uint i = 0; i != SF::dimension; ++i)
      matrix.template block<SF::nb_nodes, SF::nb_nodes>(i*SF::nb_nodes, 0).noalias()
        = state.integral_weight/state.rho * (field.shape_function() + state.tau_su * field.advection()).transpose() * field.gradient().row(i);
    
    return matrix;
  };
};

/// Velocity contribution to the momentum equation (standard + SUPG + bulk viscosity) for matrix A
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct MomentumVelocityA
{ 
  typedef Eigen::Matrix<Real, SF::dimension*SF::nb_nodes, SF::dimension*SF::nb_nodes> MatrixT;
  typedef const MatrixT& result_type;
  
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& field, StateT& state) const
  {
    for(Uint i = 0; i != SF::dimension; ++i)
    {
      // Skew symmetry and bulk viscosity
      for(Uint j = 0; j != SF::dimension; ++j)
        matrix.template block<SF::nb_nodes, SF::nb_nodes>(i*SF::nb_nodes, j*SF::nb_nodes).noalias()
          = (0.5 * field.eval()[i] * (field.shape_function() + state.tau_su * field.advection()) // Skew symmetry (standard + SUPG)
              + state.tau_bulk * field.gradient().row(i) // bulk viscosity
            ).transpose() * field.gradient().row(j);
      
      matrix.template block<SF::nb_nodes, SF::nb_nodes>(i*SF::nb_nodes, i*SF::nb_nodes)
        += state.nu*state.rho * field.gradient().transpose() * field.gradient() // diffusion
        + (field.shape_function() + state.tau_su * field.advection()).transpose() * field.advection(); // Advection + SUPG
    }
    
    // Integration
    matrix *= state.integral_weight;
    
    return matrix;
  };
};

/// Continuity equation time contribution (PSPG)
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct ContinuityT
{ 
  typedef Eigen::Matrix<Real, SF::nb_nodes, SF::dimension*SF::nb_nodes> MatrixT;
  typedef const MatrixT& result_type;
  
  // We pass data for u here, to be able to calculate the SUPG terms
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& field, StateT& state) const
  {
    for(Uint i = 0; i != SF::dimension; ++i)
    {
      matrix.template block<SF::nb_nodes, SF::nb_nodes>(0, i*SF::nb_nodes).noalias()
        = state.integral_weight * state.tau_ps * field.gradient().row(i).transpose() * field.shape_function();
    }
    
    return matrix;
  };
};

/// Momentum equation time contribution (Standard + SUPG)
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct MomentumT
{ 
  typedef Eigen::Matrix<Real, SF::dimension*SF::nb_nodes, SF::dimension*SF::nb_nodes> MatrixT;
  typedef const MatrixT& result_type;
  
  // We pass data for u here, to be able to calculate the SUPG terms
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& field, StateT& state) const
  {
    matrix.setZero();
    
    // SUPG
    for(Uint i = 0; i != SF::dimension; ++i)
    {
      matrix.template block<SF::nb_nodes, SF::nb_nodes>(i*SF::nb_nodes, i*SF::nb_nodes).noalias()
        = state.integral_weight * state.tau_su * field.advection().transpose() * field.shape_function();
    }
    
    // Second order integration for the standard time. WARNING: This must be the last op on the element, since it destroys
    // the precomputed values for first-order integration
    typedef Mesh::Integrators::GaussMappedCoords<2, SF::shape> GaussT2;
    for(Uint i = 0; i != GaussT2::nb_points; ++i)
    {
      const typename SF::ShapeFunctionsT& sf = field.shape_function(GaussT2::instance().coords.col(i));
      Eigen::Matrix<Real, SF::nb_nodes, SF::nb_nodes> m = GaussT2::instance().weights[i] * support.jacobian_determinant(GaussT2::instance().coords.col(i)) * sf.transpose() * sf;
      for(Uint d = 0; d != Dim; ++d)
        matrix.template block<SF::nb_nodes, SF::nb_nodes>(SF::nb_nodes*d, SF::nb_nodes*d) += m;
    }
    
    return matrix;
  };
};

/// Stores the coefficients for the SUPG model and shares them inside a proto expression through the state
struct SUPGState
{ 
  /// Reference velocity magnitude
  Real u_ref;
  
  /// Kinematic viscosity
  Real nu;
  
  /// Density
  Real rho;
  
  /// Model coefficients
  Real tau_ps, tau_su, tau_bulk;
  
  /// Jacobian determinant at the first gauss point, multiplied with the first gauss weight
  Real integral_weight;
};

Solver::Actions::Proto::MakeSFOp<SetTau>::type set_tau = {};
Solver::Actions::Proto::MakeSFOp<ContinuityPressureA>::type continuity_p_a = {};
Solver::Actions::Proto::MakeSFOp<ContinuityVelocityA>::type continuity_u_a = {};
Solver::Actions::Proto::MakeSFOp<MomentumPressureA>::type momentum_p_a = {};
Solver::Actions::Proto::MakeSFOp<MomentumVelocityA>::type momentum_u_a = {};
Solver::Actions::Proto::MakeSFOp<ContinuityT>::type continuity_t = {};
Solver::Actions::Proto::MakeSFOp<MomentumT>::type momentum_t = {};

} // UFEM
} // CF


#endif // CF_UFEM_NavierStokesOps_hpp
