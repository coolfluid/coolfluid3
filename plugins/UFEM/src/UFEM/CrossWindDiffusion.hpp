// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_CrossWindDiffusion_hpp
#define cf3_UFEM_CrossWindDiffusion_hpp

#include "math/MatrixTypes.hpp"
#include "mesh/Integrators/Gauss.hpp"

#include "solver/actions/Proto/ElementOperations.hpp"
#include "solver/actions/Proto/ElementData.hpp"

namespace cf3 {

namespace UFEM {

struct CrosswindDiffusionImpl
{
  template<typename Signature>
  struct result;

  template<typename This, typename UT, typename CT>
  struct result<This(UT,CT)>
  {
    typedef const Eigen::Matrix<Real, 1, UT::dimension>& type;
  };

  CrosswindDiffusionImpl() {}

  // u: advection velocity. c: scalar field to stabilize
  template<typename UT, typename CT, typename ResultT>
  const ResultT& operator()(ResultT& result, const UT& u, const CT& c)
  {
    typedef typename UT::EtypeT ElementT;
    static const Uint dim = ElementT::dimension;
    typedef Eigen::Matrix<Real, dim, 1> ColVecT;

    ColVecT g = c.nabla() * c.value();

    const Real grad_norm = g.norm();
    const Real u_norm = u.eval().norm();
    if(grad_norm == 0. || u_norm == 0.)
    {
      result.setZero();
      return result;
    }

    g /= grad_norm;
    result.noalias() = ((u.eval().transpose().dot(g))*g).transpose();
    const Real a_par_norm = result.norm();

    const Real h = 2./(g.transpose()*c.nabla()).cwiseAbs().sum();
    const Real x = a_par_norm / (2.*u_norm);
    const Real tau_c = h/(a_par_norm)*x*(1.-x);

    result *= tau_c;
    return result;
  }

  // Non-copyable
  CrosswindDiffusionImpl(const CrosswindDiffusionImpl&) = delete;
  void operator=(const CrosswindDiffusionImpl&) = delete;
};

struct CrosswindDiffusion
{
  CrosswindDiffusion() :
    apply(boost::proto::as_child(data))
  {
  }

  // Stores the operator
  solver::actions::Proto::MakeSFOp<CrosswindDiffusionImpl>::stored_type data;

  // Use as apply(velocity_field, scalar_field)
  solver::actions::Proto::MakeSFOp<CrosswindDiffusionImpl>::reference_type apply;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_CrossWindDiffusion_hpp
