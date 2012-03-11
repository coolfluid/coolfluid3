// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file BCSubsonicInlet2D.hpp
/// @brief Two Navierstokes/Euler subsonic inlet boundary conditions
///
/// - BCSubsonicInletTtPtAlpha2D
/// - BCSubsonicInletUT2D

#ifndef cf3_sdm_navierstokes_BCSubsonicInlet2D_hpp
#define cf3_sdm_navierstokes_BCSubsonicInlet2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "sdm/BCWeak.hpp"
#include "sdm/navierstokes/LibNavierStokes.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace navierstokes {

////////////////////////////////////////////////////////////////////////////////

/// @brief Subsonic inlet given total temperature, total pressure and flow angle
///
/// Default configuration: Tt = 25 Celsius, Pt = 1 bar , angle = 0 rad, gas = air
class sdm_navierstokes_API BCSubsonicInletTtPtAlpha2D : public BCWeak< PhysDataBase<4u,2u> >
{
private:
  enum {Rho=0, RhoUx=1, RhoUy=2, RhoE=3};

public:
  static std::string type_name() { return "BCSubsonicInletTtPtAlpha2D"; }
  BCSubsonicInletTtPtAlpha2D(const std::string& name) : BCWeak< PhysData >(name)
  {
    m_Tt=273.15 + 25; // 25 degrees Celcius
    options().add_option("Tt",m_Tt).description("Total Temperature").link_to(&m_Tt);
    m_Pt=100000; // 1 bar
    options().add_option("Pt",m_Pt).description("Total Pressure").link_to(&m_Pt);
    m_alpha=0.; // 0 rad
    options().add_option("alpha",m_alpha).description("flow angle in rad").link_to(&m_alpha);


    m_gamma=1.4;
    m_gamma_minus_1=m_gamma-1.;
    m_R=287.05;

    options().add_option("gamma", m_gamma)
        .description("The heat capacity ratio")
        .attach_trigger( boost::bind( &BCSubsonicInletTtPtAlpha2D::config_gamma, this) );

    options().add_option("R", m_R)
        .description("Gas constant")
        .link_to(&m_R);

  }
  virtual ~BCSubsonicInletTtPtAlpha2D() {}

  void config_gamma()
  {
    m_gamma = options().option("gamma").value<Real>();
    m_gamma_minus_1 = m_gamma - 1.;
  }

  virtual void compute_solution(const PhysData& inner_cell_data, const RealVectorNDIM& unit_normal, RealVectorNEQS& boundary_face_pt_data)
  {
    m_rho_inner       = inner_cell_data.solution[Rho];
    m_uuvv_inner      = (inner_cell_data.solution[RhoUx]*inner_cell_data.solution[RhoUx] + inner_cell_data.solution[RhoUy]*inner_cell_data.solution[RhoUy])/(m_rho_inner*m_rho_inner);
    m_rhoE_inner      = inner_cell_data.solution[RhoE];
    m_p_inner         = m_gamma_minus_1*(m_rhoE_inner - 0.5 * m_rho_inner * m_uuvv_inner);
    m_T_inner         = m_p_inner / (m_R*m_rho_inner);
    m_M2_inner        = m_uuvv_inner/(m_gamma*m_R*m_T_inner);
    m_coeff_inner     = 1. + 0.5*m_gamma_minus_1*m_M2_inner;
    m_pow_coeff_inner = std::pow(m_coeff_inner,m_gamma/m_gamma_minus_1);
    //m_Tt_inner    = m_T_inner*m_coeff_inner;
    //m_Pt_inner    = m_p_inner*m_pow_coeff_inner;

    m_M = sqrt(m_M2_inner);
    m_tan_alpha=std::tan(m_alpha);
    m_T = m_Tt/m_coeff_inner;
    m_p = m_Pt/m_pow_coeff_inner;
    m_rho = m_p/(m_R*m_T);
    m_U[XX] = m_M*std::sqrt(m_gamma*m_R*m_T/(1.+m_tan_alpha*m_tan_alpha));
    m_U[YY] = m_tan_alpha*m_U[XX];
    m_uuvv = m_U[XX]*m_U[XX]+m_U[YY]*m_U[YY];
    m_rhoE = m_p/m_gamma_minus_1 + 0.5*m_rho*m_uuvv;


    boundary_face_pt_data[Rho  ]=m_rho;
    boundary_face_pt_data[RhoUx]=m_rho*m_U[XX];
    boundary_face_pt_data[RhoUy]=m_rho*m_U[YY];
    boundary_face_pt_data[RhoE]=m_rhoE;

  }
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private: // data
  Real m_Tt;
  Real m_Pt;
  Real m_alpha;


  Real m_R;
  Real m_gamma;
  RealVector2 m_U;

  Real m_T_inner;
  Real m_rho_inner;
  Real m_p_inner;
  Real m_rhoE_inner;
  Real m_uuvv_inner;
  Real m_M2_inner;
  Real m_coeff_inner;
  Real m_pow_coeff_inner;

  Real m_M;
  Real m_tan_alpha;
  Real m_T;
  Real m_p;
  Real m_rho;
  Real m_uuvv;
  Real m_rhoE;

  Real m_gamma_minus_1;
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Subsonic inlet given Velocity and Temperature
///
/// Default configuration: Tt = 25 Celsius, U = (1,0), gas = air
/// @note Less performant than BCSubsonicInletTtPtAlpha2D
class sdm_navierstokes_API BCSubsonicInletUT2D : public BCWeak< PhysDataBase<4u,2u> >
{
private:
  enum {Rho=0, RhoUx=1, RhoUy=2, RhoE=3};

public:
  static std::string type_name() { return "BCSubsonicInletUT2D"; }
  BCSubsonicInletUT2D(const std::string& name) : BCWeak< PhysData >(name)
  {
    m_U.resize(1.,0.);
    options().add_option("U",m_U)
        .description("Velocity [m/s]")
        .link_to(&m_U);

    m_T=273.15 + 25;
    options().add_option("T",m_T)
        .description("Temperature [K]")
        .link_to(&m_T);

    m_gamma=1.4;
    m_gamma_minus_1=m_gamma-1.;
    m_R=287.05;

    options().add_option("gamma", m_gamma)
        .description("The heat capacity ratio")
        .attach_trigger( boost::bind( &BCSubsonicInletUT2D::config_gamma, this) );

    options().add_option("R", m_R)
        .description("Gas constant")
        .link_to(&m_R);

  }
  virtual ~BCSubsonicInletUT2D() {}

  void config_gamma()
  {
    m_gamma = options().option("gamma").value<Real>();
    m_gamma_minus_1 = m_gamma - 1.;
  }

  virtual void compute_solution(const PhysData& inner_cell_data, const RealVectorNDIM& unit_normal, RealVectorNEQS& boundary_face_pt_data)
  {
    // solution at inside of face
    m_rho_inner  = inner_cell_data.solution[Rho];
    m_uuvv_inner = (inner_cell_data.solution[RhoUx]*inner_cell_data.solution[RhoUx] + inner_cell_data.solution[RhoUy]*inner_cell_data.solution[RhoUy])/(m_rho_inner*m_rho_inner);
    m_rhoE_inner = inner_cell_data.solution[RhoE];
    m_p_inner    = m_gamma_minus_1*(m_rhoE_inner - 0.5 * m_rho_inner * m_uuvv_inner);

    // compute solution at outside of face
    m_rho = m_p_inner/(m_R*m_T);
    m_uuvv = m_U[XX]*m_U[XX]+m_U[YY]*m_U[YY];
    m_rhoE = m_p_inner/m_gamma_minus_1 + 0.5*m_rho*m_uuvv;

    // set solution at outside of face
    boundary_face_pt_data[Rho  ]=m_rho;
    boundary_face_pt_data[RhoUx]=m_rho*m_U[XX];
    boundary_face_pt_data[RhoUy]=m_rho*m_U[YY];
    boundary_face_pt_data[RhoE ]=m_rhoE;
  }
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private: // data
  Real m_T;
  Real m_R;
  Real m_gamma;
  std::vector<Real> m_U;

  Real m_rho_inner;
  Real m_p_inner;
  Real m_uuvv_inner;
  Real m_rhoE_inner;

  Real m_rho;
  Real m_uuvv;
  Real m_rhoE;

  Real m_gamma_minus_1;
};

////////////////////////////////////////////////////////////////////////////////

} // navierstokes
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_BCSubsonicInlet2D_hpp
