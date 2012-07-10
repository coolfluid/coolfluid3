// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_navierstokesmovingreference_BCSubsonicOutlet2D_hpp
#define cf3_sdm_navierstokesmovingreference_BCSubsonicOutlet2D_hpp

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

class sdm_navierstokes_API BCSubsonicOutlet2D : public BCWeak< PhysDataBase<4u,2u> >
{
  enum {Rho=0, RhoUx=1, RhoUy=2, RhoE=3};

public:
  static std::string type_name() { return "BCSubsonicOutlet2D"; }
  BCSubsonicOutlet2D(const std::string& name) : BCWeak< PhysData >(name)
  {
    m_function_P.parse("100000","x,y"); // 1bar

    options().add("P",m_function_P.function()).description("Pressure")
        .attach_trigger( boost::bind( &BCSubsonicOutlet2D::config_P, this) );

    m_gamma=1.4;
    m_gamma_minus_1=m_gamma-1.;

    m_omega.setZero();
    m_Vtrans.setZero();

    m_Vt.setZero();

    dummy.setZero();
    dummy_coord.setZero();

    options().add("gamma", m_gamma)
        .description("The heat capacity ratio")
        .attach_trigger( boost::bind( &BCSubsonicOutlet2D::config_gamma, this) );

    std::vector<Real> OmegaDefault (3,0), VtransDefault(2,0);
    OmegaDefault[0] = m_omega[0];
    OmegaDefault[1] = m_omega[1];
    OmegaDefault[2] = m_omega[2];

    VtransDefault[0] = m_Vtrans[0];
    VtransDefault[1] = m_Vtrans[1];

    options().add("Omega", OmegaDefault)
        .description("Rotation vector")
        .mark_basic()
        .attach_trigger(boost::bind( &BCSubsonicOutlet2D::config_Omega, this));

    options().add("Vtrans", VtransDefault)
        .description("Vector of the translation speeds")
        .mark_basic()
        .attach_trigger( boost::bind( &BCSubsonicOutlet2D::config_Vtrans, this));
  }
  virtual ~BCSubsonicOutlet2D() {}

  void config_gamma()
  {
    m_gamma = options().value<Real>("gamma");
    m_gamma_minus_1 = m_gamma - 1.;
  }

  void config_Omega()
  {
      std::vector<Real> Omega_vec= options().value< std::vector<Real> >("Omega");
      cf3_assert(Omega_vec.size() == 3);
      cf3_assert(Omega_vec[0] == 0);
      cf3_assert(Omega_vec[1] == 0);
      m_omega[0] = Omega_vec[0];
      m_omega[1] = Omega_vec[1];
      m_omega[2] = Omega_vec[2];
  }

  void config_Vtrans()
  {
      std::vector<Real> Vtrans_vec= options().value< std::vector<Real> >("Vtrans");
      cf3_assert(Vtrans_vec.size() == 2);
      m_Vtrans[0] = Vtrans_vec[0];
      m_Vtrans[1] = Vtrans_vec[1];
  }

  void config_P()    { m_function_P   .parse(options().option("P").value_str()); }

  void compute_transformation_velocity(const RealVector& coord, RealVectorNDIM& Vt)
  {
      dummy_coord[0] = coord[0];
      dummy_coord[1] = coord[1];
      dummy_coord[2] = 0.;

      Vt = m_Vtrans;
      dummy = m_omega.cross(dummy_coord);

      Vt[0] += dummy[0];
      Vt[1] += dummy[1];
  }

  virtual void compute_solution(const PhysData& inner_cell_data, const RealVectorNDIM& unit_normal, RealVectorNEQS& boundary_face_pt_data)
  {
    m_function_P.evaluate(inner_cell_data.coord,m_p);

    compute_transformation_velocity(inner_cell_data.coord,m_Vt);

    m_x = inner_cell_data.coord[0];
    m_y = inner_cell_data.coord[1];

    m_rho_inner = inner_cell_data.solution[Rho];
    m_u_inner = inner_cell_data.solution[RhoUx]/m_rho_inner;
    m_v_inner = inner_cell_data.solution[RhoUy]/m_rho_inner;
    m_uuvv_inner = m_u_inner*m_u_inner + m_v_inner*m_v_inner;
    m_Vt2 = m_Vt[XX]*m_Vt[XX]+m_Vt[YY]*m_Vt[YY];

    boundary_face_pt_data[Rho ]=inner_cell_data.solution[Rho];
    boundary_face_pt_data[RhoUx]=inner_cell_data.solution[RhoUx];
    boundary_face_pt_data[RhoUy]=inner_cell_data.solution[RhoUy];
    boundary_face_pt_data[RhoE ]=m_p/m_gamma_minus_1 + 0.5 * m_rho_inner * m_uuvv_inner - 0.5*m_rho_inner*m_Vt2;
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

  Real m_p;
  Real m_gamma;
  Real m_gamma_minus_1;

  Real m_u_inner;
  Real m_v_inner;
  Real m_rho_inner;
  Real m_uuvv_inner;

};

////////////////////////////////////////////////////////////////////////////////

} // navierstokesmovingreference
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_BCSubsonicOutlet2D_hpp
