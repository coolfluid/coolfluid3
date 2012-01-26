// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_navierstokes_BCWallEuler2D_hpp
#define cf3_sdm_navierstokes_BCWallEuler2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "sdm/BCWeak.hpp"
#include "sdm/navierstokes/LibNavierStokes.hpp"
#include "Physics/NavierStokes/Cons2D.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace navierstokes {

////////////////////////////////////////////////////////////////////////////////

/// PhysData implements only solution and coords
struct PhysData : PhysDataBase<4u,2u> {};

////////////////////////////////////////////////////////////////////////////////

class sdm_navierstokes_API BCWallEuler2D : public BCWeak< PhysDataBase<4u,2u> >
{
public:
  static std::string type_name() { return "BCWallEuler2D"; }
  BCWallEuler2D(const std::string& name) : BCWeak< PhysData >(name)
  {
  }
  virtual ~BCWallEuler2D() {}

  virtual void compute_solution(const PhysData& inner_cell_data, const RealVectorNDIM& unit_normal, RealVectorNEQS& boundary_face_pt_data)
  {
    rhoU[XX] = inner_cell_data.solution[physics::NavierStokes::Cons2D::RhoU];
    rhoU[YY] = inner_cell_data.solution[physics::NavierStokes::Cons2D::RhoV];
    rhoU_normal = rhoU.transpose()*flx_pt_plane_jacobian_normal->get().plane_unit_normal[cell_flx_pt];
    rhoU_normal *= flx_pt_plane_jacobian_normal->get().sf->flx_pt_sign(cell_flx_pt);
//    std::cout << "rhoU = " << rhoU.transpose() << std::endl;
//    std::cout << "normalrhoU = " << flx_pt_plane_jacobian_normal->get().plane_unit_normal[cell_flx_pt].transpose()*flx_pt_plane_jacobian_normal->get().sf->flx_pt_sign(cell_flx_pt)*rhoU_normal << std::endl;
    rhoU -= flx_pt_plane_jacobian_normal->get().plane_unit_normal[cell_flx_pt]*2.*rhoU_normal*flx_pt_plane_jacobian_normal->get().sf->flx_pt_sign(cell_flx_pt);
//    std::cout << "rhoU mirror = " << rhoU.transpose() << std::endl;

    boundary_face_pt_data[physics::NavierStokes::Cons2D::Rho ]=inner_cell_data.solution[physics::NavierStokes::Cons2D::Rho];
    boundary_face_pt_data[physics::NavierStokes::Cons2D::RhoU]=rhoU[XX];
    boundary_face_pt_data[physics::NavierStokes::Cons2D::RhoV]=rhoU[YY];
    boundary_face_pt_data[physics::NavierStokes::Cons2D::RhoE]=inner_cell_data.solution[physics::NavierStokes::Cons2D::RhoE];
  }
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:

  virtual void initialize()
  {
    BCWeak< PhysData >::initialize();
    flx_pt_plane_jacobian_normal = shared_caches().get_cache< FluxPointPlaneJacobianNormal<NDIM> >();
    flx_pt_plane_jacobian_normal->options().configure_option("space",solution_field().dict().handle<mesh::Dictionary>());
  }

  virtual void set_inner_cell()
  {
    BCWeak< PhysData >::set_inner_cell();
    flx_pt_plane_jacobian_normal->cache(cell_entities,cell_idx);
  }
  virtual void unset_inner_cell()
  {
    BCWeak< PhysData >::unset_inner_cell();
    flx_pt_plane_jacobian_normal->get().unlock();
  }

  Handle<CacheT<FluxPointPlaneJacobianNormal<NDIM> > > flx_pt_plane_jacobian_normal;
  RealVectorNDIM rhoU;
  Real rhoU_normal;
};

////////////////////////////////////////////////////////////////////////////////

} // navierstokes
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_BCWallEuler2D_hpp
