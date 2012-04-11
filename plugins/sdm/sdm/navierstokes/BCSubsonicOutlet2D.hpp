// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_navierstokes_BCSubsonicOutlet2D_hpp
#define cf3_sdm_navierstokes_BCSubsonicOutlet2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "sdm/BCWeak.hpp"
#include "sdm/navierstokes/LibNavierStokes.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace navierstokes {

////////////////////////////////////////////////////////////////////////////////

class sdm_navierstokes_API BCSubsonicOutlet2D : public BCWeak< PhysDataBase<4u,2u> >
{
  enum {Rho=0, RhoUx=1, RhoUy=2, RhoE=3};

public:
  static std::string type_name() { return "BCSubsonicOutlet2D"; }
  BCSubsonicOutlet2D(const std::string& name) : BCWeak< PhysData >(name)
  {
    m_p = 101300.;
    options().add_option("p",m_p).link_to(&m_p);

    m_gamma=1.4;
    m_gamma_minus_1=m_gamma-1.;

    options().add_option("gamma", m_gamma)
        .description("The heat capacity ratio")
        .attach_trigger( boost::bind( &BCSubsonicOutlet2D::config_gamma, this) );
  }
  virtual ~BCSubsonicOutlet2D() {}

  void config_gamma()
  {
    m_gamma = options().option("gamma").value<Real>();
    m_gamma_minus_1 = m_gamma - 1.;
  }

  virtual void compute_solution(const PhysData& inner_cell_data, const RealVectorNDIM& unit_normal, RealVectorNEQS& boundary_face_pt_data)
  {

    m_rho_inner  = inner_cell_data.solution[Rho];
    m_u_inner    = inner_cell_data.solution[RhoUx]/m_rho_inner;
    m_v_inner    = inner_cell_data.solution[RhoUy]/m_rho_inner;
    m_uuvv_inner = m_u_inner*m_u_inner + m_v_inner*m_v_inner;

    boundary_face_pt_data[Rho  ]=inner_cell_data.solution[Rho];
    boundary_face_pt_data[RhoUx]=inner_cell_data.solution[RhoUx];
    boundary_face_pt_data[RhoUy]=inner_cell_data.solution[RhoUy];
    boundary_face_pt_data[RhoE ]=m_p/m_gamma_minus_1 + 0.5 * m_rho_inner * m_uuvv_inner;
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
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_BCSubsonicOutlet2D_hpp
