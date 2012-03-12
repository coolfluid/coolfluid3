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

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace navierstokes {

////////////////////////////////////////////////////////////////////////////////

/// PhysData implements only solution and coords
struct PhysData : PhysDataBase<4u,2u> {};

////////////////////////////////////////////////////////////////////////////////

/// @brief Inviscid wall boundary condition (slip) for Euler equations
///
/// No configuration necessary
/// @todo Replace outward_normal by passed unit_normal, when unit_normal is correctly
/// calculated. Then the extra virtual functions can dissapear.
class sdm_navierstokes_API BCWallEuler2D : public BCWeak< PhysDataBase<4u,2u> >
{
private:
  enum {Rho=0, RhoUx=1, RhoUy=2, RhoE=3};
public:
  static std::string type_name() { return "BCWallEuler2D"; }
  BCWallEuler2D(const std::string& name) : BCWeak< PhysData >(name)
  {
  }
  virtual ~BCWallEuler2D() {}

  virtual void compute_solution(const PhysData& inner_cell_data, const RealVectorNDIM& unit_normal, RealVectorNEQS& boundary_face_pt_data)
  {
    // Calculate outward normal. (should use argument unit_normal later)
    outward_normal = flx_pt_plane_jacobian_normal->get().plane_unit_normal[cell_flx_pt]*flx_pt_plane_jacobian_normal->get().sf->flx_pt_sign(cell_flx_pt);

    // velocity on the inside of the face
    rhoU[XX] = inner_cell_data.solution[RhoUx];
    rhoU[YY] = inner_cell_data.solution[RhoUy];

    // velocity in outward_normal direction
    rhoU_normal = rhoU.transpose()*outward_normal;

    // Modify velocity to become the outside velocity of the face,
    // and being the mirror of the inside velocity
    rhoU.noalias() -= 2.*rhoU_normal*outward_normal;

    // Set the outside boundary state
    boundary_face_pt_data[Rho  ]=inner_cell_data.solution[Rho];
    boundary_face_pt_data[RhoUx]=rhoU[XX];
    boundary_face_pt_data[RhoUy]=rhoU[YY];
    boundary_face_pt_data[RhoE ]=inner_cell_data.solution[RhoE];
  }
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:

  virtual void initialize()
  {
    BCWeak< PhysData >::initialize();
    // all this stuff should dissapear when unit_normal is used
    flx_pt_plane_jacobian_normal = shared_caches().get_cache< FluxPointPlaneJacobianNormal<NDIM> >();
    flx_pt_plane_jacobian_normal->options().configure_option("space",solution_field().dict().handle<mesh::Dictionary>());
  }

  virtual void set_inner_cell()
  {
    BCWeak< PhysData >::set_inner_cell();
    // all this stuff should dissapear when unit_normal is used
    flx_pt_plane_jacobian_normal->cache(cell_entities,cell_idx);
  }
  virtual void unset_inner_cell()
  {
    BCWeak< PhysData >::unset_inner_cell();
    // all this stuff should dissapear when unit_normal is used
    flx_pt_plane_jacobian_normal->get().unlock();
  }

  RealVectorNDIM rhoU;
  RealVectorNDIM outward_normal;
  Real rhoU_normal;

  // all this stuff should dissapear when unit_normal is used
  Handle<CacheT<FluxPointPlaneJacobianNormal<NDIM> > > flx_pt_plane_jacobian_normal;
};

////////////////////////////////////////////////////////////////////////////////

} // navierstokes
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_BCWallEuler2D_hpp
