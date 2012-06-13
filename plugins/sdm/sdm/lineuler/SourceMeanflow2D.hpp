// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_SourceMeanflow2D_hpp
#define cf3_sdm_lineuler_SourceMeanflow2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/OptionComponent.hpp"

#include "math/Consts.hpp"
#include "math/MatrixTypesConversion.hpp"

#include "solver/Time.hpp"

#include "sdm/SourceTerm.hpp"
#include "sdm/lineuler/LibLinEuler.hpp"
#include "Physics/LinEuler/Cons2D.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

////////////////////////////////////////////////////////////////////////////////

struct SourceMeanflowPhysData2D : PhysDataBase<4u,2u>
{
  typedef Eigen::Matrix<Real,NDIM,NEQS> RealMatrixNDIMxNEQS;
  RealVectorNEQS meanflow;
  RealMatrixNDIMxNEQS meanflow_gradient;
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Source term, coming from inhomogeneous mean flow
///
/// The source term H is defined as
///
///      [                                 0                                  ]
///  H = [     (rho' u0 + rho0 u') du0/dx  +  (rho' v0 + rho0 v') du0/dy      ]
///      [     (rho' u0 + rho0 u') dv0/dx  +  (rho' v0 + rho0 v') dv0/dy      ]
///      [ (gamma-1) p' (du0/dx + dv0/dy) - (gamma-1) (u' dp0/dx + v' dp0/dy) ]
///
/// with u0 = U0[XX]
/// with v0 = U0[YY]
/// with u' = U[XX] - U0[XX]
/// with v' = U[YY] - U0[YY]
///
/// @author Willem Deconinck
class sdm_lineuler_API SourceMeanflow2D : public SourceTerm< SourceMeanflowPhysData2D >
{
public:
  static std::string type_name() { return "SourceMeanflow2D"; }
  SourceMeanflow2D(const std::string& name) : SourceTerm< PhysData >(name)
  {
    options().add("gamma",m_gamma).link_to(&m_gamma);
    options().add("mean_flow",m_meanflow)
      .description("Field containing the mean flow")
      .link_to(&m_meanflow);

    m_J.resize(NDIM,NDIM);
    m_Jinv.resize(NDIM,NDIM);

  }

  virtual ~SourceMeanflow2D() {}


  void compute_sol_pt_phys_data(const SFDElement& elem, const Uint sol_pt, PhysData& phys_data )
  {
    SourceTerm<PhysData>::compute_sol_pt_phys_data(elem,sol_pt,phys_data);
    mesh::Field::View meanflow_view = m_meanflow->view(elem.space->connectivity()[elem.idx]);

    RealMatrix meanflow(elem.sf->nb_sol_pts(), (int)NEQS);
    for (Uint s=0; s<meanflow.rows(); ++s)
    {
      for (Uint v=0; v<meanflow.cols(); ++v)
      {
        meanflow(s,v) = meanflow_view[s][v];
      }
    }
    for (Uint v=0; v<meanflow.cols(); ++v)
    {
      phys_data.meanflow(v) = meanflow_view[sol_pt][v];
    }

    RealMatrix sf_grad((int)NDIM,elem.sf->nb_sol_pts());
    elem.sf->compute_gradient(elem.sf->sol_pts().row(sol_pt),sf_grad);

    phys_data.meanflow_gradient = sf_grad * meanflow;

    // transform to physical space: (dQ/dx, dQ/dy, dQ/dz)
    RealMatrix geom_coord_matrix = elem.entities->geometry_space().get_coordinates(elem.idx);
    elem.entities->element_type().compute_jacobian(elem.sf->sol_pts().row(sol_pt),geom_coord_matrix, m_J);
    m_Jinv = m_J.inverse();
    phys_data.meanflow_gradient = m_Jinv * phys_data.meanflow_gradient;
  }


  virtual void compute_source(PhysData& data,
                              RealVectorNEQS& source)
  {
    ///      [                                0                                   ]
    ///  H = [    (rho' u0 + rho0 u') du0/dx  +  (rho' v0 + rho0 v') du0/dy       ]
    ///      [    (rho' u0 + rho0 u') dv0/dx  +  (rho' v0 + rho0 v') dv0/dy       ]
    ///      [ (gamma-1) p' (du0/dx + dv0/dy) - (gamma-1) (u' dp0/dx + v' dp0/dy) ]

    const Real&       rho0    = data.meanflow[0];
    const Real&       u0      = data.meanflow[1];
    const Real&       v0      = data.meanflow[2];
    const Real&       p0      = data.meanflow[3];
    const Real&       rho     = data.solution[0];  // rho'
    const Real&       rho0u   = data.solution[1];  // rho0 u'
    const Real&       rho0v   = data.solution[2];  // rho0 v'
    const Real&       p       = data.solution[3];  // p'
    const RealVector& grad_u0 = data.meanflow_gradient.col(1);
    const RealVector& grad_v0 = data.meanflow_gradient.col(2);
    const RealVector& grad_p0 = data.meanflow_gradient.col(3);
    const Real        gm1     = m_gamma-1.;
    const Real        u       = rho0u/rho0;
    const Real        v       = rho0v/rho0;
    source[0] = - (                                   0.                                    );
    source[1] = - (       (rho*u0 + rho0u)*grad_u0[XX] + (rho*v0 + rho0v)*grad_u0[YY]       );
    source[2] = - (       (rho*u0 + rho0u)*grad_v0[XX] + (rho*v0 + rho0v)*grad_v0[YY]       );
    source[3] = - ( gm1*p*(grad_u0[XX] + grad_v0[YY]) - gm1*(u*grad_p0[XX] + v*grad_p0[YY]) );
    // minus signs in front because this definition assumed H is on left hand side of equation, while this will
    // be added to right hand side
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:

  Handle<mesh::Field> m_meanflow;  ///< mean flow field
  Real m_gamma;
  RealMatrix m_J;
  RealMatrix m_Jinv;
};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_SourceMeanflow2D_hpp
