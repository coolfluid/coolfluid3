// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_navierstokes_BCWall3D_hpp
#define cf3_sdm_navierstokes_BCWall3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "sdm/BCWeak.hpp"
#include "sdm/navierstokes/LibNavierStokes.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace navierstokes {

////////////////////////////////////////////////////////////////////////////////

/// PhysData implements only solution and coords
struct PhysData : PhysDataBase<5u,3u> {};

////////////////////////////////////////////////////////////////////////////////

/// @brief Viscous adiabatic wall boundary condition for Navier-Stokes equations
///
/// The velocity inside the wall is reversed, so that a velocity flux results in zero.
/// The wall temperature is interpolated from inside.
///
/// No configuration is necessary.

/// @todo add option velocity of moving wall
class sdm_navierstokes_API BCWall3D : public BCWeak< PhysDataBase<5u,3u> >
{
private:
  enum {Rho=0, RhoUx=1, RhoUy=2, RhoUz=3, RhoE=4};

public:
  static std::string type_name() { return "BCWall3D"; }
  BCWall3D(const std::string& name) : BCWeak< PhysData >(name)
  {
//      m_wall_velocity = 0.;
//      options().add("wall velocity",m_wall_velocity)
//          .description("The velocity of the wall")
//          .link_to(&m_wall_velocity);
  }
  virtual ~BCWall3D() {}

  virtual void compute_solution(const PhysData& inner_cell_data, const RealVectorNDIM& unit_normal, RealVectorNEQS& boundary_face_pt_data)
  {
    // Calculate outward normal. (should use argument unit_normal later)
//    outward_normal = flx_pt_plane_jacobian_normal->get().plane_unit_normal[cell_flx_pt]*flx_pt_plane_jacobian_normal->get().sf->flx_pt_sign(cell_flx_pt);


    // Set the outside boundary state
    boundary_face_pt_data[Rho  ] =  inner_cell_data.solution[Rho];
//    boundary_face_pt_data[RhoUx] = -inner_cell_data.solution[RhoUx] - 2.*inner_cell_data.solution[Rho]*outward_normal[YY]*m_wall_velocity;
//    boundary_face_pt_data[RhoUy] = -inner_cell_data.solution[RhoUy] + 2.*inner_cell_data.solution[Rho]*outward_normal[XX]*m_wall_velocity;
    boundary_face_pt_data[RhoUx] = -inner_cell_data.solution[RhoUx];
    boundary_face_pt_data[RhoUy] = -inner_cell_data.solution[RhoUy];
    boundary_face_pt_data[RhoUz] = -inner_cell_data.solution[RhoUz];
    boundary_face_pt_data[RhoE ] =  inner_cell_data.solution[RhoE];
//    Real u = (boundary_face_pt_data[RhoUx]+inner_cell_data.solution[RhoUx])/boundary_face_pt_data[Rho  ]/2.;
//    Real v = (boundary_face_pt_data[RhoUy]+inner_cell_data.solution[RhoUy])/boundary_face_pt_data[Rho  ]/2.;
//    std::cout << "velocity on the wall \n" << std::sqrt(u*u+v*v) << std::endl;

  }
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  private:
//      Real m_wall_velocity;
//      RealVectorNDIM rhoU;
//      RealVectorNDIM outward_normal;

      virtual void initialize()
      {
        BCWeak< PhysData >::initialize();
        // all this stuff should dissapear when unit_normal is used
        flx_pt_plane_jacobian_normal = shared_caches().get_cache< FluxPointPlaneJacobianNormal<NDIM> >();
        flx_pt_plane_jacobian_normal->options().set("space",solution_field().dict().handle<mesh::Dictionary>());
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

      // all this stuff should dissapear when unit_normal is used
      Handle<CacheT<FluxPointPlaneJacobianNormal<NDIM> > > flx_pt_plane_jacobian_normal;
};

////////////////////////////////////////////////////////////////////////////////

} // navierstokes
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_BCWall3D_hpp
