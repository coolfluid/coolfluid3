// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_CrossWindDiffusion_hpp
#define cf3_UFEM_CrossWindDiffusion_hpp

#include "math/MatrixTypes.hpp"

#include "mesh/Integrators/Gauss.hpp"

namespace cf3 {

namespace UFEM {

struct CrosswindDiffusion
{
  typedef Real result_type;

  CrosswindDiffusion() : d0(1.)
  {
  }

  template<typename UT, typename CT>
  Real operator()(const UT& u, const CT& c)
  {
    typedef typename UT::EtypeT ElementT;
    static const Uint dim = ElementT::dimension;
    typedef mesh::Integrators::GaussMappedCoords<1, ElementT::shape> GaussT;
    typedef Eigen::Matrix<Real, dim, 1> ColVecT;

    ColVecT g = c.nabla() * c.value();
    const Real grad_norm = g.norm();
    const Real u_norm = u.eval().norm();
    if(grad_norm < 1e-10 || u_norm < 1e-10)
    {
      return 0.;
    }
    g /= grad_norm;
    const Real hg = 2./(g.transpose()*c.nabla()).cwiseAbs().sum();
    return d0*hg*u_norm;
  }

  Real d0;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_CrossWindDiffusion_hpp
