// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_navierstokes_BCWall2D_hpp
#define cf3_sdm_navierstokes_BCWall2D_hpp

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

/// @brief Viscous adiabatic wall boundary condition for Navier-Stokes equations
///
/// The velocity inside the wall is reversed, so that a velocity flux results in zero.
/// The wall temperature is interpolated from inside.
///
/// No configuration is necessary.
class sdm_navierstokes_API BCWall2D : public BCWeak< PhysDataBase<4u,2u> >
{
private:
  enum {Rho=0, RhoUx=1, RhoUy=2, RhoE=3};
public:
  static std::string type_name() { return "BCWall2D"; }
  BCWall2D(const std::string& name) : BCWeak< PhysData >(name)
  {
  }
  virtual ~BCWall2D() {}

  virtual void compute_solution(const PhysData& inner_cell_data, const RealVectorNDIM& unit_normal, RealVectorNEQS& boundary_face_pt_data)
  {
    // Set the outside boundary state
    boundary_face_pt_data[Rho  ] =  inner_cell_data.solution[Rho];
    boundary_face_pt_data[RhoUx] = -inner_cell_data.solution[RhoUx];
    boundary_face_pt_data[RhoUy] = -inner_cell_data.solution[RhoUy];
    boundary_face_pt_data[RhoE ] =  inner_cell_data.solution[RhoE];
  }
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

////////////////////////////////////////////////////////////////////////////////

} // navierstokes
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_BCWall2D_hpp
