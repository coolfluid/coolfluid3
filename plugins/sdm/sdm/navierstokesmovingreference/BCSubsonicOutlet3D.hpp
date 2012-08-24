// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_navierstokesmovingreference_BCSubsonicOutlet3D_hpp
#define cf3_sdm_navierstokesmovingreference_BCSubsonicOutlet3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>

#include "math/AnalyticalFunction.hpp"

#include "sdm/BCWeak.hpp"
#include "sdm/navierstokesmovingreference/LibNavierStokesMovingReference.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace navierstokesmovingreference {

////////////////////////////////////////////////////////////////////////////////

class sdm_navierstokes_API BCSubsonicOutlet3D : public BCWeak< PhysDataBase<5u,3u> >
{
  enum {Rho=0, RhoUx=1, RhoUy=2, RhoUz=3, RhoE=4};

public:
  static std::string type_name() { return "BCSubsonicOutlet3D"; }
  BCSubsonicOutlet3D(const std::string& name) : BCWeak< PhysData >(name)
  {
    m_function_P.parse("100000","x,y,z"); // 1bar

    options().add("P",m_function_P.function()).description("Pressure")
        .attach_trigger( boost::bind( &BCSubsonicOutlet3D::config_P, this) );

    m_gamma=1.4;
    m_gamma_minus_1=m_gamma-1.;

    m_omega.setZero();
    m_Vtrans.setZero();

    m_Vt.setZero();

    dummy.setZero();
    dummy_coord.setZero();

    options().add("gamma", m_gamma)
        .description("The heat capacity ratio")
        .attach_trigger( boost::bind( &BCSubsonicOutlet3D::config_gamma, this) );

    std::vector<Real> OmegaDefault (3,0), VtransDefault(3,0);
    OmegaDefault[XX] = m_omega[XX];
    OmegaDefault[YY] = m_omega[YY];
    OmegaDefault[ZZ] = m_omega[ZZ];

    VtransDefault[XX] = m_Vtrans[ZZ];
    VtransDefault[YY] = m_Vtrans[YY];
    VtransDefault[ZZ] = m_Vtrans[ZZ];

    options().add("Omega", OmegaDefault)
        .description("Rotation vector")
        .mark_basic()
        .attach_trigger(boost::bind( &BCSubsonicOutlet3D::config_Omega, this));

    options().add("Vtrans", VtransDefault)
        .description("Vector of the translation speeds")
        .mark_basic()
        .attach_trigger( boost::bind( &BCSubsonicOutlet3D::config_Vtrans, this));
  }
  virtual ~BCSubsonicOutlet3D() {}

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

  void config_P()    { m_function_P   .parse(options().option("P").value_str()); }

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
    m_function_P.evaluate(inner_cell_data.coord,m_p);

    compute_transformation_velocity(inner_cell_data.coord,m_Vt);

    m_x = inner_cell_data.coord[XX];
    m_y = inner_cell_data.coord[YY];
    m_z = inner_cell_data.coord[ZZ];

    m_rho_inner = inner_cell_data.solution[Rho];
    m_u_inner = inner_cell_data.solution[RhoUx]/m_rho_inner;
    m_v_inner = inner_cell_data.solution[RhoUy]/m_rho_inner;
    m_w_inner = inner_cell_data.solution[RhoUz]/m_rho_inner;
    m_uuvvww_inner = m_u_inner*m_u_inner + m_v_inner*m_v_inner + m_w_inner*m_w_inner;
    m_Vt2 = m_Vt[XX]*m_Vt[XX] + m_Vt[YY]*m_Vt[YY] + m_Vt[ZZ]*m_Vt[ZZ];

    boundary_face_pt_data[Rho ]=inner_cell_data.solution[Rho];
    boundary_face_pt_data[RhoUx]=inner_cell_data.solution[RhoUx];
    boundary_face_pt_data[RhoUy]=inner_cell_data.solution[RhoUy];
    boundary_face_pt_data[RhoUz]=inner_cell_data.solution[RhoUz];
    boundary_face_pt_data[RhoE ]=m_p/m_gamma_minus_1 + 0.5 * m_rho_inner * m_uuvvww_inner - 0.5*m_rho_inner*m_Vt2;
  }
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW


private: // data

  math::AnalyticalFunction m_function_P;

  RealVector3 m_omega;
  RealVectorNDIM m_Vtrans;
  RealVectorNDIM m_Vt;
  Real m_Vt2;

  RealVector3 dummy, dummy_coord;

  Real m_x;
  Real m_y;
  Real m_z;

  Real m_p;
  Real m_gamma;
  Real m_gamma_minus_1;

  Real m_u_inner;
  Real m_v_inner;
  Real m_w_inner;
  Real m_rho_inner;
  Real m_uuvvww_inner;

};

////////////////////////////////////////////////////////////////////////////////

} // navierstokesmovingreference
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_BCSubsonicOutlet3D_hpp
