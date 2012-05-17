// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_SourceVelocityField2D_hpp
#define cf3_sdm_lineuler_SourceVelocityField2D_hpp

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

struct SourceVelocityFieldPhysData2D : PhysDataBase<4u,2u>
{
  RealVectorNDIM grad_uu;
  RealVectorNDIM grad_uv;
  RealVectorNDIM grad_vv;
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Source term, using an external velocity field
///
/// The velocity field must contain total velocity:
/// U[XX] , U[YY] , U[ZZ]
///
/// The computed source:
///
///      [                  0                 ]
///  S = [ rho0 ( d(u'u')/dx  +  d(u'v')/dy ) ]
///      [ rho0 ( d(u'v')/dx  +  d(v'v')/dy ) ]
///      [                  0                 ]
///
/// with u' = U[XX] - U0[XX]
/// with v' = U[YY] - U0[YY]
///
/// @author Willem Deconinck
class sdm_lineuler_API SourceVelocityField2D : public SourceTerm< SourceVelocityFieldPhysData2D >
{
public:
  static std::string type_name() { return "SourceVelocityField2D"; }
  SourceVelocityField2D(const std::string& name) : SourceTerm< PhysData >(name)
  {
    options().add_option("velocity_field",m_velocity_field)
      .description("Field containing velocity")
      .link_to(&m_velocity_field);

    m_U0 << 0.,0.;
    std::vector<Real> U0(NDIM,0.);
    options().add_option("U0", U0)
        .description("Uniform mean velocity")
        .attach_trigger( boost::bind( &SourceVelocityField2D::config_U0, this) );

    m_rho0 = 1.;
    options().add_option("rho0",m_rho0)
        .description("Uniform mean density")
        .link_to(&m_rho0);
  }

  void config_U0()
  {
    std::vector<Real> U0 = options().option("U0").value<std::vector<Real> >();
    m_U0 = RealVectorNDIM::MapType(&U0.front(),U0.size());
  }

  virtual ~SourceVelocityField2D() {}


  void compute_sol_pt_phys_data(const SFDElement& elem, const Uint sol_pt, PhysData& phys_data )
  {
    SourceTerm<PhysData>::compute_sol_pt_phys_data(elem,sol_pt,phys_data);
    mesh::Field::View sol_pt_source_velocity = m_velocity_field->view(elem.space->connectivity()[elem.idx]);

    // allocate beforehand
    RealVector uu(elem.sf->nb_sol_pts());
    RealVector uv(elem.sf->nb_sol_pts());
    RealVector vv(elem.sf->nb_sol_pts());

    RealMatrix sf_grad((int)NDIM,elem.sf->nb_sol_pts());
    elem.sf->compute_gradient(elem.sf->sol_pts().row(sol_pt),sf_grad);
    for (Uint s=0; s<elem.sf->nb_sol_pts(); ++s)
    {
      const Real u = sol_pt_source_velocity[s][XX] - m_U0[XX];
      const Real v = sol_pt_source_velocity[s][YY] - m_U0[YY];
      uu[s] = u*u;
      uv[s] = u*v;
      vv[s] = v*v;
    }

    // compute mapped gradient using sf_grad
    RealVectorNDIM mapped_grad_uu = sf_grad * uu;
    RealVectorNDIM mapped_grad_uv = sf_grad * uv;
    RealVectorNDIM mapped_grad_vv = sf_grad * uv;

    // transform to physical space: (dQ/dx, dQ/dy, dQ/dz)
    RealMatrix geom_coord_matrix = elem.entities->geometry_space().get_coordinates(elem.idx);
    RealMatrix Jinv; Jinv.resize(NDIM,NDIM);
    Jinv = elem.entities->element_type().jacobian(elem.sf->sol_pts().row(sol_pt),geom_coord_matrix).inverse();
    phys_data.grad_uu = Jinv * mapped_grad_uu;
    phys_data.grad_uv = Jinv * mapped_grad_uv;
    phys_data.grad_vv = Jinv * mapped_grad_vv;
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

  Real m_rho0;                           ///< Mean uniform density
  Handle<mesh::Field> m_velocity_field;  ///< Source velocity field
  RealVectorNDIM m_U0;                   ///< Mean uniform velocity

};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_SourceVelocityField2D_hpp
