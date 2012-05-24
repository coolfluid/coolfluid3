// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_scalar_Diffusion1D_hpp
#define cf3_sdm_scalar_Diffusion1D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "sdm/DiffusiveTerm.hpp"
#include "sdm/scalar/LibScalar.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace scalar {

template <Uint NB_EQS, Uint NB_DIM>
struct DiffusionPhysData : PhysDataBase<NB_EQS,NB_DIM>
{
  typedef Eigen::Matrix<Real,NB_DIM,NB_EQS> RealMatrixNDIMxNEQS;
  RealMatrixNDIMxNEQS solution_gradient;
};

////////////////////////////////////////////////////////////////////////////////

class sdm_scalar_API Diffusion1D : public DiffusiveTerm< DiffusionPhysData<1u,1u> >
{
public:
  static std::string type_name() { return "Diffusion1D"; }
  Diffusion1D(const std::string& name) : DiffusiveTerm< PhysData >(name)
  {
    m_mu = 1.;
    options().add_option("mu",m_mu).description("Diffusion coefficient").link_to(&m_mu);
  }

  virtual ~Diffusion1D() {}

  virtual void compute_flux(PhysData& data, const RealVectorNDIM& unit_normal,
                            RealVectorNEQS& flux, Real& wave_speed)
  {
    flux[0] = m_mu * data.solution_gradient[0] * unit_normal[XX];
    wave_speed = m_mu;
  }
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  Real m_mu;
};

////////////////////////////////////////////////////////////////////////////////

} // scalar
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_scalar_Diffusion1D_hpp
