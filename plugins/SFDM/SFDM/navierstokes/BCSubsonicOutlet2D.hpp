// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_navierstokes_BCSubsonicOutlet2D_hpp
#define cf3_SFDM_navierstokes_BCSubsonicOutlet2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "SFDM/BCWeak.hpp"
#include "SFDM/navierstokes/LibNavierStokes.hpp"
#include "Physics/NavierStokes/Cons2D.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {
namespace navierstokes {

////////////////////////////////////////////////////////////////////////////////

class SFDM_navierstokes_API BCSubsonicOutlet2D : public BCWeak< BCPointData<4u,2u> >
{
  typedef physics::NavierStokes::Cons2D PHYS;
public:
  static std::string type_name() { return "BCSubsonicOutlet2D"; }
  BCSubsonicOutlet2D(const std::string& name) : BCWeak(name)
  {
    m_p = 101300.;
    m_gamma = 1.4;
    m_gamma_minus_1 = m_gamma-1.;
    options().add_option("p",m_p).link_to(&m_p);
    options().add_option("gamma",m_gamma).attach_trigger( boost::bind( &BCSubsonicOutlet2D::configure_gamma , this));
  }
  virtual ~BCSubsonicOutlet2D() {}
  virtual void configure_gamma()
  {
    m_gamma = options().option("gamma").value<Real>();
    m_gamma_minus_1 = m_gamma - 1.;
  }

  virtual void compute_solution(const BCPointData<4u,2u>& inner_cell_data, Eigen::Matrix<Real,NEQS,1>& boundary_face_pt_data)
  {

    m_rho_inner  = inner_cell_data.solution[physics::NavierStokes::Cons2D::Rho];
    m_u_inner    = inner_cell_data.solution[physics::NavierStokes::Cons2D::RhoU]/m_rho_inner;
    m_v_inner    = inner_cell_data.solution[physics::NavierStokes::Cons2D::RhoV]/m_rho_inner;
    m_uuvv_inner = m_u_inner*m_u_inner + m_v_inner*m_v_inner;

    boundary_face_pt_data[physics::NavierStokes::Cons2D::Rho ]=inner_cell_data.solution[physics::NavierStokes::Cons2D::Rho];
    boundary_face_pt_data[physics::NavierStokes::Cons2D::RhoU]=inner_cell_data.solution[physics::NavierStokes::Cons2D::RhoU];
    boundary_face_pt_data[physics::NavierStokes::Cons2D::RhoV]=inner_cell_data.solution[physics::NavierStokes::Cons2D::RhoV];
    boundary_face_pt_data[physics::NavierStokes::Cons2D::RhoE]=m_p/m_gamma_minus_1 + 0.5 * m_rho_inner * m_uuvv_inner;
  }


private: // data

  Real m_p;
  Real m_gamma;
  Real m_gamma_minus_1;

  Real m_u_inner;
  Real m_v_inner;
  Real m_rho_inner;
  Real m_uuvv_inner;
};

////////////////////////////////////////////////////////////////////////////////

} // navierstokes
} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_BCSubsonicOutlet2D_hpp
