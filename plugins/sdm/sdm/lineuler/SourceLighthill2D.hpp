// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_SourceLighthill2D_hpp
#define cf3_sdm_lineuler_SourceLighthill2D_hpp

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

struct SourceLighthillPhysData2D : PhysDataBase<4u,2u>
{
  RealVectorNDIM grad_uu;
  RealVectorNDIM grad_uv;
  RealVectorNDIM grad_vv;
  RealVectorNDIM grad_uu_avg;
  RealVectorNDIM grad_uv_avg;
  RealVectorNDIM grad_vv_avg;

};

////////////////////////////////////////////////////////////////////////////////

/// @brief Source term, using the lighthill tensor
///
/// The Lighthill Tensor T is here defined as  ( u'_i u_'j )
///
/// The computed source:
///
/// short notation:   S = rho0*d( u'_i u'_j )/d( x_j ) - < rho0*d( u'_i u'_j )/d( x_j ) >
///      [                  0                 ]     [                    0                   ]
///  S = [ rho0 ( d(u'u')/dx  +  d(u'v')/dy ) ]  -  [ rho0 ( d(<u'u'>)/dx  +  d(<u'v'>)/dy ) ]
///      [ rho0 ( d(u'v')/dx  +  d(v'v')/dy ) ]     [ rho0 ( d(<v'u'>)/dx  +  d(<v'v'>)/dy ) ]
///      [                  0                 ]     [                    0                   ]
///
/// with u' = U[XX] - U0[XX]
/// with v' = U[YY] - U0[YY]
/// with < lolo > the time-averaged lolo
///
/// @author Willem Deconinck
class sdm_lineuler_API SourceLighthill2D : public SourceTerm< SourceLighthillPhysData2D >
{
public:
  static std::string type_name() { return "SourceLighthill2D"; }
  SourceLighthill2D(const std::string& name) : SourceTerm< PhysData >(name)
  {
    options().add("lighthill_tensor",m_lighthill_tensor_field)
      .description("Field containing lighthill tensor")
      .link_to(&m_lighthill_tensor_field);

    options().add("avg_lighthill_tensor",m_avg_lighthill_tensor_field)
      .description("Field containing time averaged lighthill tensor")
      .link_to(&m_avg_lighthill_tensor_field);

//    m_U0 << 0.,0.;
//    std::vector<Real> U0(NDIM,0.);
//    options().add("U0", U0)
//        .description("Uniform mean velocity")
//        .attach_trigger( boost::bind( &SourceLighthill2D::config_U0, this) );

    m_rho0 = 1.;
    options().add("rho0",m_rho0)
        .description("Uniform mean density")
        .link_to(&m_rho0);
  }

//  void config_U0()
//  {
//    std::vector<Real> U0 = options().value<std::vector<Real> >("U0");
//    m_U0 = RealVectorNDIM::MapType(&U0.front(),U0.size());
//  }

  virtual ~SourceLighthill2D() {}


  void compute_sol_pt_phys_data(const SFDElement& elem, const Uint sol_pt, PhysData& phys_data )
  {
    SourceTerm<PhysData>::compute_sol_pt_phys_data(elem,sol_pt,phys_data);
    mesh::Field::View sol_pt_lighthill_tensor = m_lighthill_tensor_field->view(elem.space->connectivity()[elem.idx]);
    mesh::Field::View sol_pt_avg_lighthill_tensor = m_avg_lighthill_tensor_field->view(elem.space->connectivity()[elem.idx]);

    // allocate beforehand
    RealVector uu(elem.sf->nb_sol_pts());
    RealVector uv(elem.sf->nb_sol_pts());
    RealVector vv(elem.sf->nb_sol_pts());
    RealVector uu_avg(elem.sf->nb_sol_pts());
    RealVector uv_avg(elem.sf->nb_sol_pts());
    RealVector vv_avg(elem.sf->nb_sol_pts());

    RealMatrix sf_grad((int)NDIM,elem.sf->nb_sol_pts());
    elem.sf->compute_gradient(elem.sf->sol_pts().row(sol_pt),sf_grad);
    for (Uint s=0; s<elem.sf->nb_sol_pts(); ++s)
    {
      // instantaneous Lighthill tensor
      uu[s] = sol_pt_lighthill_tensor[s][0]; // XX
      uv[s] = sol_pt_lighthill_tensor[s][1]; // XY
      vv[s] = sol_pt_lighthill_tensor[s][3]; // YY

      // time averaged Lighthill tensor
      uu_avg[s] = sol_pt_avg_lighthill_tensor[s][0]; // XX
      uv_avg[s] = sol_pt_avg_lighthill_tensor[s][1]; // XY
      vv_avg[s] = sol_pt_avg_lighthill_tensor[s][3]; // YY
    }

    // compute mapped gradient using sf_grad
    RealVectorNDIM mapped_grad_uu = sf_grad * uu;
    RealVectorNDIM mapped_grad_uv = sf_grad * uv;
    RealVectorNDIM mapped_grad_vv = sf_grad * vv;

    // compute mapped gradient using sf_grad
    RealVectorNDIM mapped_grad_uu_avg = sf_grad * uu_avg;
    RealVectorNDIM mapped_grad_uv_avg = sf_grad * uv_avg;
    RealVectorNDIM mapped_grad_vv_avg = sf_grad * vv_avg;

    // transform to physical space: (dQ/dx, dQ/dy, dQ/dz)
    RealMatrix geom_coord_matrix = elem.entities->geometry_space().get_coordinates(elem.idx);
    RealMatrix Jinv; Jinv.resize(NDIM,NDIM);
    Jinv = elem.entities->element_type().jacobian(elem.sf->sol_pts().row(sol_pt),geom_coord_matrix).inverse();
    phys_data.grad_uu = Jinv * mapped_grad_uu;
    phys_data.grad_uv = Jinv * mapped_grad_uv;
    phys_data.grad_vv = Jinv * mapped_grad_vv;
    phys_data.grad_uu_avg = Jinv * mapped_grad_uu_avg;
    phys_data.grad_uv_avg = Jinv * mapped_grad_uv_avg;
    phys_data.grad_vv_avg = Jinv * mapped_grad_vv_avg;

  }


  virtual void compute_source(PhysData& data,
                              RealVectorNEQS& source)
  {
    source[0] = 0.;
    source[1] = m_rho0 * (data.grad_uu[XX] + data.grad_uv[YY]);
    source[2] = m_rho0 * (data.grad_uv[XX] + data.grad_vv[YY]);
    source[3] = 0.;
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:

  Real m_rho0;                                   ///< Mean uniform density
  Handle<mesh::Field> m_lighthill_tensor_field;  ///< Source velocity field
  Handle<mesh::Field> m_avg_lighthill_tensor_field;  ///< Source velocity field
//  RealVectorNDIM m_U0;                           ///< Mean uniform velocity

};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_SourceLighthill2D_hpp
