// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file BCSubsonicInlet3D.hpp
/// @brief Two Navierstokes/Euler subsonic inlet boundary conditions
///
/// - BCSubsonicInletTtPtAlpha3D
/// - BCSubsonicInletUT3D

#ifndef cf3_sdm_navierstokesmovingreference_BCSubsonicInlet3D_hpp
#define cf3_sdm_navierstokesmovingreference_BCSubsonicInlet3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>

#include "math/AnalyticalFunction.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "sdm/BCWeak.hpp"
#include "sdm/navierstokesmovingreference/LibNavierStokesMovingReference.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace navierstokesmovingreference {

////////////////////////////////////////////////////////////////////////////////

/// @brief Subsonic inlet given total temperature, total pressure and flow angle
///
/// Default configuration: Tt = 25 Celsius, Pt = 1 bar , angle = 0 rad, gas = air
class sdm_navierstokes_API BCSubsonicInletTtPtAlpha3D : public BCWeak< PhysDataBase<5u,3u> >
{
private:
  enum {Rho=0, RhoUx=1, RhoUy=2, RhoUz=3, RhoE=4};

public:
  static std::string type_name() { return "BCSubsonicInletTtPtAlpha3D"; }
  BCSubsonicInletTtPtAlpha3D(const std::string& name) : BCWeak< PhysData >(name)
  {
    m_function_Tt.parse("298.15","x,y,z");  // 25 degrees Celcius
    m_function_Pt.parse("100000","x,y,z");  // 1 bar
    m_function_alphaxy.parse("0","x,y,z");  // 0 radians
    m_function_alphaxz.parse("0","x,y,z");  // 0 radians

    options().add("Tt",m_function_Tt.function()).description("Total Temperature in relative frame of reference")
        .attach_trigger( boost::bind( &BCSubsonicInletTtPtAlpha3D::config_Tt, this) );
    options().add("Pt",m_function_Pt.function()).description("Total Pressure in relative frame of reference")
        .attach_trigger( boost::bind( &BCSubsonicInletTtPtAlpha3D::config_Pt, this) );
    options().add("alphaxy",m_function_alphaxy.function()).description("flow angle projected on xy-plane in rad")
        .attach_trigger( boost::bind( &BCSubsonicInletTtPtAlpha3D::config_alphaxy, this) );
    options().add("alphaxz",m_function_alphaxz.function()).description("flow angle projected on xz-plane in rad")
        .attach_trigger( boost::bind( &BCSubsonicInletTtPtAlpha3D::config_alphaxz, this) );

    m_omega.setZero();
    m_Vtrans.setZero();

    m_Vt.setZero();

    dummy.setZero();
    dummy_coord.setZero();

    m_gamma=1.4;
    m_gamma_minus_1=m_gamma-1.;
    m_R=287.05;

    options().add("gamma", m_gamma)
        .description("The heat capacity ratio")
        .attach_trigger( boost::bind( &BCSubsonicInletTtPtAlpha3D::config_gamma, this) );

    options().add("R", m_R)
        .description("Gas constant")
        .link_to(&m_R);

    std::vector<Real> OmegaDefault (3,0), VtransDefault(3,0);
    OmegaDefault[0] = m_omega[0];
    OmegaDefault[1] = m_omega[1];
    OmegaDefault[2] = m_omega[2];

    VtransDefault[0] = m_Vtrans[0];
    VtransDefault[1] = m_Vtrans[1];
    VtransDefault[2] = m_Vtrans[2];

    options().add("Omega", OmegaDefault)
        .description("Rotation vector")
        .mark_basic()
        .attach_trigger(boost::bind( &BCSubsonicInletTtPtAlpha3D::config_Omega, this));

    options().add("Vtrans", VtransDefault)
        .description("Vector of the translation speeds")
        .mark_basic()
        .attach_trigger( boost::bind( &BCSubsonicInletTtPtAlpha3D::config_Vtrans, this));

  }
  virtual ~BCSubsonicInletTtPtAlpha3D() {}

  void config_gamma()
  {
    m_gamma = options().value<Real>("gamma");
    m_gamma_minus_1 = m_gamma - 1.;
  }

  void config_Omega()
  {
      std::vector<Real> Omega_vec= options().value< std::vector<Real> >("Omega");
      cf3_assert(Omega_vec.size() == 3);
//      cf3_assert(Omega_vec[0] == 0);
//      cf3_assert(Omega_vec[1] == 0);
      m_omega[0] = Omega_vec[0];
      m_omega[1] = Omega_vec[1];
      m_omega[2] = Omega_vec[2];
  }

  void config_Vtrans()
  {
      std::vector<Real> Vtrans_vec= options().value< std::vector<Real> >("Vtrans");
      cf3_assert(Vtrans_vec.size() == 3);
      m_Vtrans[0] = Vtrans_vec[0];
      m_Vtrans[1] = Vtrans_vec[1];
      m_Vtrans[2] = Vtrans_vec[2];
  }

  void config_Tt()      { m_function_Tt     .parse(options().option("Tt").value_str()); }
  void config_Pt()      { m_function_Pt     .parse(options().option("Pt").value_str()); }
  void config_alphaxy() { m_function_alphaxy.parse(options().option("alphaxy").value_str()); }
  void config_alphaxz() { m_function_alphaxz.parse(options().option("alphaxz").value_str()); }

  void compute_transformation_velocity(const RealVector& coord, RealVectorNDIM& Vt)
  {
      dummy_coord[XX] = coord[XX];
      dummy_coord[YY] = coord[YY];
      dummy_coord[ZZ] = coord[ZZ];

      Vt = m_Vtrans;
      dummy = m_omega.cross(dummy_coord);

      Vt[XX] += dummy[XX];
      Vt[YY] += dummy[YY];
      Vt[ZZ] += dummy[ZZ];
  }

  virtual void compute_solution(const PhysData& inner_cell_data, const RealVectorNDIM& unit_normal, RealVectorNEQS& boundary_face_pt_data)
  {
    // Evaluate analytical functions
    m_function_Tt.evaluate(inner_cell_data.coord,m_Tt);
    m_function_Pt.evaluate(inner_cell_data.coord,m_Pt);
    m_function_alphaxy.evaluate(inner_cell_data.coord,m_alphaxy);
    m_function_alphaxz.evaluate(inner_cell_data.coord,m_alphaxz);

    compute_transformation_velocity(inner_cell_data.coord,m_Vt);

    // Compute inner cell data
    m_x               = inner_cell_data.coord[XX];
    m_y               = inner_cell_data.coord[YY];
    m_z               = inner_cell_data.coord[ZZ];
    m_Vt2             = m_Vt[XX]*m_Vt[XX]+m_Vt[YY]*m_Vt[YY]+m_Vt[ZZ]*m_Vt[ZZ];
    m_rho_inner       = inner_cell_data.solution[Rho];
    m_uuvvww_inner      = (inner_cell_data.solution[RhoUx]*inner_cell_data.solution[RhoUx] + inner_cell_data.solution[RhoUy]*inner_cell_data.solution[RhoUy] + inner_cell_data.solution[RhoUz]*inner_cell_data.solution[RhoUz])/(m_rho_inner*m_rho_inner);
    m_rhoE_inner      = inner_cell_data.solution[RhoE];
    m_p_inner         = m_gamma_minus_1*(m_rhoE_inner - 0.5*m_rho_inner*m_uuvvww_inner + 0.5*m_rho_inner*m_Vt2);
    m_T_inner         = m_p_inner / (m_R*m_rho_inner);
    m_c2_inner        = (m_gamma*m_R*m_T_inner);
    m_M2_inner        = m_uuvvww_inner/m_c2_inner;
    m_coeff_inner     = 1. + 0.5*m_gamma_minus_1*m_M2_inner - 0.5*m_gamma_minus_1*(m_Vt2)/m_c2_inner;
    m_pow_coeff_inner = std::pow(m_coeff_inner,m_gamma/m_gamma_minus_1);
    //m_Tt_inner    = m_T_inner*m_coeff_inner;
    //m_Pt_inner    = m_p_inner*m_pow_coeff_inner;

    // Compute values to impose on boundary
    m_M = sqrt(m_M2_inner);
    m_tan_alphaxy=std::tan(m_alphaxy);
    m_tan_alphaxz=std::tan(m_alphaxz);
    m_T = m_Tt/m_coeff_inner;
    m_p = m_Pt/m_pow_coeff_inner;
    m_rho = m_p/(m_R*m_T);
    m_U[XX] = m_M*std::sqrt(m_gamma*m_R*m_T/(1.+m_tan_alphaxy*m_tan_alphaxy + m_tan_alphaxz*m_tan_alphaxz));
    m_U[YY] = m_tan_alphaxy*m_U[XX];
    m_U[ZZ] = m_tan_alphaxz*m_U[XX];
    m_uuvvww = m_U[XX]*m_U[XX]+m_U[YY]*m_U[YY]+m_U[ZZ]*m_U[ZZ];
    m_rhoE = m_p/m_gamma_minus_1 + 0.5*m_rho*m_uuvvww - 0.5*m_rho*(m_Vt2);

    boundary_face_pt_data[Rho  ]=m_rho;
    boundary_face_pt_data[RhoUx]=m_rho*m_U[XX];
    boundary_face_pt_data[RhoUy]=m_rho*m_U[YY];
    boundary_face_pt_data[RhoUz]=m_rho*m_U[ZZ];
    boundary_face_pt_data[RhoE]=m_rhoE;

  }
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private: // data
  math::AnalyticalFunction m_function_Tt;
  math::AnalyticalFunction m_function_Pt;
  math::AnalyticalFunction m_function_alphaxy;
  math::AnalyticalFunction m_function_alphaxz;

  Real m_x;
  Real m_y;
  Real m_z;

  Real m_Tt;
  Real m_Pt;
  Real m_alphaxy;
  Real m_alphaxz;

  RealVector3 dummy, dummy_coord;

  RealVector3 m_omega;
  RealVectorNDIM m_Vtrans;
  RealVectorNDIM m_Vt;
  Real m_Vt2;

  Real m_R;
  Real m_gamma;
  RealVectorNDIM m_U;

  Real m_T_inner;
  Real m_rho_inner;
  Real m_p_inner;
  Real m_rhoE_inner;
  Real m_uuvvww_inner;
  Real m_M2_inner;
  Real m_coeff_inner;
  Real m_pow_coeff_inner;
  Real m_c2_inner;

  Real m_M;
  Real m_tan_alphaxy;
  Real m_tan_alphaxz;
  Real m_T;
  Real m_p;
  Real m_rho;
  Real m_uuvvww;
  Real m_rhoE;

  Real m_gamma_minus_1;
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Subsonic inlet given Velocity and Temperature
///
/// Default configuration: Tt = 25 Celsius, U = (1,0), gas = air
/// @note Less performant than BCSubsonicInletTtPtAlpha2D
class sdm_navierstokes_API BCSubsonicInletUT3D : public BCWeak< PhysDataBase<5u,3u> >
{
private:
  enum {Rho=0, RhoUx=1, RhoUy=2, RhoUz=3, RhoE=4};

public:
  static std::string type_name() { return "BCSubsonicInletUT3D"; }
  BCSubsonicInletUT3D(const std::string& name) : BCWeak< PhysData >(name)
  {
    m_function_T.parse("298.15","x,y,z");

    m_U.resize(2.,0.);
    options().add("U",m_U)
        .description("Velocity [m/s]")
        .link_to(&m_U);


    m_gamma=1.4;
    m_gamma_minus_1=m_gamma-1.;
    m_R=287.05;

    m_omega.setZero();
    m_Vtrans.setZero();

    m_Vt.setZero();

    dummy.setZero();
    dummy_coord.setZero();

    options().add("T",m_function_T.function()).description("Temperature")
        .attach_trigger( boost::bind( &BCSubsonicInletUT3D::config_T, this) );

    options().add("gamma", m_gamma)
        .description("The heat capacity ratio")
        .attach_trigger( boost::bind( &BCSubsonicInletUT3D::config_gamma, this) );

    options().add("R", m_R)
        .description("Gas constant")
        .link_to(&m_R);

    std::vector<Real> OmegaDefault (3,0), VtransDefault(3,0);
    OmegaDefault[XX] = m_omega[XX];
    OmegaDefault[YY] = m_omega[YY];
    OmegaDefault[ZZ] = m_omega[ZZ];

    VtransDefault[XX] = m_Vtrans[XX];
    VtransDefault[YY] = m_Vtrans[YY];
    VtransDefault[ZZ] = m_Vtrans[ZZ];

    options().add("Omega", OmegaDefault)
        .description("Rotation vector")
        .mark_basic()
        .attach_trigger(boost::bind( &BCSubsonicInletUT3D::config_Omega, this));

    options().add("Vtrans", VtransDefault)
        .description("Vector of the translation speeds")
        .mark_basic()
        .attach_trigger( boost::bind( &BCSubsonicInletUT3D::config_Vtrans, this));

  }
  virtual ~BCSubsonicInletUT3D() {}

  void config_gamma()
  {
    m_gamma = options().value<Real>("gamma");
    m_gamma_minus_1 = m_gamma - 1.;
  }
  void config_T()    { m_function_T.parse(options().option("T").value_str()); }

  void config_Omega()
  {
      std::vector<Real> Omega_vec= options().value< std::vector<Real> >("Omega");
      cf3_assert(Omega_vec.size() == 3);
//      cf3_assert(Omega_vec[0] == 0);
//      cf3_assert(Omega_vec[1] == 0);
      m_omega[XX] = Omega_vec[XX];
      m_omega[YY] = Omega_vec[YY];
      m_omega[ZZ] = Omega_vec[ZZ];
  }

  void config_Vtrans()
  {
      std::vector<Real> Vtrans_vec= options().value< std::vector<Real> >("Vtrans");
      cf3_assert(Vtrans_vec.size() == 3);
      m_Vtrans[XX] = Vtrans_vec[XX];
      m_Vtrans[YY] = Vtrans_vec[YY];
      m_Vtrans[ZZ] = Vtrans_vec[ZZ];
  }


  void compute_transformation_velocity(const RealVector& coord, RealVectorNDIM& Vt)
  {
      dummy_coord[XX] = coord[XX];
      dummy_coord[YY] = coord[YY];
      dummy_coord[ZZ] = coord[ZZ];

      Vt = m_Vtrans;
      dummy = m_omega.cross(dummy_coord);

      Vt[XX] += dummy[XX];
      Vt[YY] += dummy[YY];
      Vt[ZZ] += dummy[ZZ];
  }

  virtual void compute_solution(const PhysData& inner_cell_data, const RealVectorNDIM& unit_normal, RealVectorNEQS& boundary_face_pt_data)
  {
    // Evaluate analytical functions
    m_function_T.evaluate(inner_cell_data.coord,m_T);

    compute_transformation_velocity(inner_cell_data.coord,m_Vt);

    // solution at inside of face
    /// @todo must be dx and dy to centre of rotation (check other locations also)
    m_x          = inner_cell_data.coord[XX];
    m_y          = inner_cell_data.coord[YY];
    m_z          = inner_cell_data.coord[ZZ];
    m_Vt2        = m_Vt[XX]*m_Vt[XX]+m_Vt[YY]*m_Vt[YY]+m_Vt[ZZ]*m_Vt[ZZ];
    m_rho_inner  = inner_cell_data.solution[Rho];
    m_uuvvww_inner = (inner_cell_data.solution[RhoUx]*inner_cell_data.solution[RhoUx] + inner_cell_data.solution[RhoUy]*inner_cell_data.solution[RhoUy] + inner_cell_data.solution[RhoUz]*inner_cell_data.solution[RhoUz])/(m_rho_inner*m_rho_inner);
    m_rhoE_inner = inner_cell_data.solution[RhoE];
    m_p_inner    = m_gamma_minus_1*(m_rhoE_inner - 0.5 * m_rho_inner * m_uuvvww_inner + 0.5*m_rho*(m_Vt2));

    // compute solution at outside of face
    m_rho = m_p_inner/(m_R*m_T);
    m_uuvvww = m_U[XX]*m_U[XX]+m_U[YY]*m_U[YY]+m_U[ZZ]*m_U[ZZ];
    m_rhoE = m_p_inner/m_gamma_minus_1 + 0.5*m_rho*m_uuvvww - 0.5*m_rho*(m_Vt2);

    // set solution at outside of face
    boundary_face_pt_data[Rho  ]=m_rho;
    boundary_face_pt_data[RhoUx]=m_rho*m_U[XX];
    boundary_face_pt_data[RhoUy]=m_rho*m_U[YY];
    boundary_face_pt_data[RhoUz]=m_rho*m_U[ZZ];
    boundary_face_pt_data[RhoE ]=m_rhoE;

//    std::cout << "rho = " << m_rho << std::endl;
//    std::cout << "U = " << m_U[XX]<< std::endl;
//    std::cout << "V = " << m_U[YY]<< std::endl;
//    std::cout << "rhoE = " << m_rhoE<< std::endl;
//    cf3_always_assert(std::abs(m_U[YY]) < 50.);
  }
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private: // data
  math::AnalyticalFunction m_function_T;

  Real m_T;
  Real m_R;
  Real m_gamma;
  std::vector<Real> m_U;

  RealVector3 dummy, dummy_coord;

  RealVector3 m_omega;
  RealVectorNDIM m_Vtrans;
  RealVectorNDIM m_Vt;
  Real m_Vt2;

  Real m_x;
  Real m_y;
  Real m_z;

  Real m_rho_inner;
  Real m_p_inner;
  Real m_uuvvww_inner;
  Real m_rhoE_inner;

  Real m_rho;
  Real m_uuvvww;
  Real m_rhoE;

  Real m_gamma_minus_1;
};

////////////////////////////////////////////////////////////////////////////////

} // navierstokesmovingreference
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_BCSubsonicInlet3D_hpp
