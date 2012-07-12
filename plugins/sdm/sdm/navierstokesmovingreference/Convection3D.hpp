// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_navierstokesmovingreference_Convection3D_hpp
#define cf3_sdm_navierstokesmovingreference_Convection3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "sdm/ConvectiveTerm.hpp"
#include "sdm/navierstokesmovingreference/LibNavierStokesMovingReference.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace navierstokesmovingreference {

////////////////////////////////////////////////////////////////////////////////

class sdm_navierstokes_API Convection3D : public ConvectiveTerm< PhysDataBase<5u,3u> >
{
private:
    Real gamma;
    RealVector3 Vtrans;
    RealVector3 Omega;

    void config_Omega()
    {
        std::vector<Real> Omega_vec= options().value< std::vector<Real> >("Omega");
        cf3_assert(Omega_vec.size() == 3);
//        cf3_assert(Omega_vec[XX] == 0);
//        cf3_assert(Omega_vec[YY] == 0);
        Omega[0] = Omega_vec[0];
        Omega[1] = Omega_vec[1];
        Omega[2] = Omega_vec[2];
    }

    void config_Vtrans()
    {
        std::vector<Real> Vtrans_vec= options().value< std::vector<Real> >("Vtrans");
        cf3_assert(Vtrans_vec.size() == 3);
        Vtrans[XX] = Vtrans_vec[XX];
        Vtrans[YY] = Vtrans_vec[YY];
        Vtrans[ZZ] = Vtrans_vec[ZZ];
    }

public:
  static std::string type_name() { return "Convection3D"; }
  Convection3D(const std::string& name) : ConvectiveTerm< PhysData >(name),
                                          gamma(1.4)
  {
    Omega.setZero();
    Vtrans.setZero();

    m_Vt.setZero();

    properties_left.setZero();
    properties_right.setZero();
    properties_roe.setZero();

    roe_avg.setZero();
    roe_var_left.setZero();
    roe_var_right.setZero();

    flux_left.setZero();
    flux_right.setZero();
    flux.setZero();

    dummy.setZero();
    dummy_coord.setZero();

    eigenvalues.setZero();
    right_eigenvectors.setZero();
    left_eigenvectors.setZero();
    abs_jacobian.setZero();


    std::vector<Real> OmegaDefault (3,0), VtransDefault(3,0);
    OmegaDefault[XX] = Omega[XX];
    OmegaDefault[YY] = Omega[YY];
    OmegaDefault[ZZ] = Omega[ZZ];

    VtransDefault[XX] = Vtrans[XX];
    VtransDefault[YY] = Vtrans[YY];
    VtransDefault[ZZ] = Vtrans[ZZ];

    options().add("Omega", OmegaDefault)
        .description("Rotation vector")
        .mark_basic()
        .attach_trigger(boost::bind( &Convection3D::config_Omega, this));

    options().add("Vtrans", VtransDefault)
        .description("Vector of the translation speeds")
        .mark_basic()
        .attach_trigger( boost::bind( &Convection3D::config_Vtrans, this));

    options().add("gamma", gamma)
        .description("The heat capacity ratio")
        .link_to(&gamma);
  }

  virtual ~Convection3D() {}

  void compute_transformation_velocity(const RealVector& coord, RealVectorNDIM& Vt)
  {
      dummy_coord[XX] = coord[XX];
      dummy_coord[YY] = coord[YY];
      dummy_coord[ZZ] = coord[ZZ];

      Vt = Vtrans;
      dummy = Omega.cross(dummy_coord);

      Vt[XX] += dummy[XX];
      Vt[YY] += dummy[YY];
      Vt[ZZ] += dummy[ZZ];
  }

  virtual void compute_analytical_flux(PhysData& data, const RealVectorNDIM& unit_normal,
                                       RealVectorNEQS& flux, Real& wave_speed)
  {
      Real rho, rhou, rhov, rhow, rhoE;
      Real u, v, w, H, P;
      Real um;
      Real a; // speed of sound

      compute_transformation_velocity(data.coord,m_Vt);

      rho   = data.solution[0];
      rhou  = data.solution[1];
      rhov  = data.solution[2];
      rhow  = data.solution[3];
      rhoE  = data.solution[4];

      cf3_assert(rho>0);
      u     = rhou / rho;
      v     = rhov / rho;
      w     = rhow / rho;
      P     = (gamma - 1) * (rhoE - 0.5 * rho *(u*u + v*v + w*w) + 0.5 * rho * ( m_Vt.dot(m_Vt)));
      H     = rhoE / rho + P / rho;

      um    = u * unit_normal[XX] + v * unit_normal[YY] + w * unit_normal[ZZ];

      a     = std::sqrt(gamma * P / rho);


      flux[0] = rho * um;
      flux[1] = rho * um * u + P * unit_normal[XX];
      flux[2] = rho * um * v + P * unit_normal[YY];
      flux[3] = rho * um * w + P * unit_normal[ZZ];
      flux[4] = rho * um * H;

      wave_speed = std::max(std::abs(um + a), std::abs(um - a));
  }

  void compute_properties(const PhysData& data, RealVectorNEQS& properties)
  {
      Real P;
      compute_transformation_velocity(data.coord,m_Vt);
      cf3_assert(data.solution[0]>0);
      properties[0]  = data.solution[0];                //rho
      properties[1]  = data.solution[1]/properties[0];  //u
      properties[2]  = data.solution[2]/properties[0];  //v
      properties[3]  = data.solution[3]/properties[0];  //w
      cf3_assert(data.solution[3]>0);
      cf3_assert(properties[0]>0);
      P = (gamma-1.)*(data.solution[4]-0.5*properties[0]*(properties[1]*properties[1]+properties[2]*properties[2]+properties[3]*properties[3])+0.5*properties[0]*(m_Vt[0] * m_Vt[0] + m_Vt[1] * m_Vt[1] + m_Vt[2] * m_Vt[2]));
      properties[4]  = (data.solution[4] + P) / properties[0];            //H

//      std::cout << "P = " << P << std::endl;
//      std::cout << "data.solution[0] = " << data.solution[0] << std::endl;
//      std::cout << "data.solution[1] = " << data.solution[1] << std::endl;
//      std::cout << "data.solution[2] = " << data.solution[2] << std::endl;
//      std::cout << "data.solution[3] = " << data.solution[3] << std::endl;
//      std::cout << "properties[0] = " << properties[0] << std::endl;
//      std::cout << "properties[1] = " << properties[1] << std::endl;
//      std::cout << "properties[2] = " << properties[2] << std::endl;
//      std::cout << "properties[3] = " << properties[3] << std::endl;
//      std::cout << "data.coord = " << data.coord.transpose() << std::endl;
      cf3_assert(P>0);
      cf3_assert(properties[4]>0);

  }

  virtual void compute_numerical_flux(PhysData& left, PhysData& right, const RealVectorNDIM& unit_normal,
                                      RealVectorNEQS& flux, Real& wave_speed)
  {
      Real P;
      compute_transformation_velocity(left.coord,m_Vt);

      // computation of the left and right properties
//      std::cout << "left" << std::endl;
      compute_properties(left, properties_left);
//      std::cout << "right " << std::endl;
      compute_properties(right, properties_right);

      cf3_assert(properties_left[0]>0);
      // computation of the left and right roe variables and roe average
      roe_var_left[0] = std::sqrt(properties_left[0]);                // sqrt(rho)
      roe_var_left[1] = roe_var_left[0] * properties_left[1];         // sqrt(rho)*u
      roe_var_left[2] = roe_var_left[0] * properties_left[2];         // sqrt(rho)*v
      roe_var_left[3] = roe_var_left[0] * properties_left[3];         // sqrt(rho)*w
      roe_var_left[4] = roe_var_left[0] * properties_left[4];         // sqrt(rho)*H

      cf3_assert(properties_right[0]>0);

      roe_var_right[0] = std::sqrt(properties_right[0]);              // sqrt(rho)
      roe_var_right[1] = roe_var_right[0] * properties_right[1];      // sqrt(rho)*u
      roe_var_right[2] = roe_var_right[0] * properties_right[2];      // sqrt(rho)*v
      roe_var_right[3] = roe_var_right[0] * properties_right[3];      // sqrt(rho)*w
      roe_var_right[4] = roe_var_right[0] * properties_right[4];      // sqrt(rho)*H

      roe_avg.noalias() = 0.5*(roe_var_left + roe_var_right);

      cf3_assert(roe_avg[0]>0);
      cf3_assert(roe_avg[4]>=0);

      properties_roe[0] = roe_avg[0]*roe_avg[0];            //rho
      properties_roe[1] = roe_avg[1]/roe_avg[0];            //u
      properties_roe[2] = roe_avg[2]/roe_avg[0];            //v
      properties_roe[3] = roe_avg[3]/roe_avg[0];            //w
      properties_roe[4] = roe_avg[4]/roe_avg[0];            //H
//      P = properties_roe[0] * (gamma - 1.)/gamma*(properties_roe[3]-0.5
//                                                 *(properties_roe[1]*properties_roe[1] + properties_roe[2]*properties_roe[2]) + 0.5*(Vt[0] * Vt[0] + Vt[1] * Vt[1]));
//                                                 //(gamma-1)/gamma * rho (H-0.5*(uu+vv)+0.5*Vt*Vt)
      P = properties_roe[0]*(properties_roe[4] - 0.5*(properties_roe[1]*properties_roe[1] + properties_roe[2]*properties_roe[2]+ properties_roe[3]*properties_roe[3]) + 0.5*(m_Vt[0]*m_Vt[0] + m_Vt[1]*m_Vt[1] + m_Vt[2]*m_Vt[2]));
      P *= ((gamma-1)/gamma);

      cf3_assert(P>0);

      const Real nx = unit_normal[XX];
      const Real ny = unit_normal[YY];
      const Real nz = unit_normal[ZZ];

      const Real rho = properties_roe[0];
      cf3_assert(rho>0);

      const Real u = properties_roe[1];
      const Real v = properties_roe[2];
      const Real w = properties_roe[3];
      const Real H = properties_roe[4];
      const Real a = std::sqrt(gamma * P / rho);
      const Real a2 = gamma * P / rho;

      cf3_assert(a2>=0);
      cf3_assert(a>=0);

      const Real gamma_minus_1 = gamma - 1.;
      const Real uuvvww = u*u + v*v + w*w;
      const Real half_gm1_v2 = 0.5 * gamma_minus_1 * uuvvww;
      const Real inv_rho = 1./rho;


      const Real inv_a  = 1. / a;
      const Real inv_a2 = inv_a * inv_a;

      const Real um = u * nx + v * ny + w * nz;
      const Real ra = 0.5 * rho * inv_a;

      const Real coeffM2 = half_gm1_v2 * inv_a2;
      const Real uDivA = gamma_minus_1 * u * inv_a;
      const Real vDivA = gamma_minus_1 * v * inv_a;
      const Real wDivA = gamma_minus_1 * w * inv_a;
      const Real rho_a = rho * a;

      const Real k1 = gamma_minus_1*0.5*uuvvww/a2;
      const Real k2 = 1.0 - k1;
      const Real k3 = -gamma_minus_1/a2;

      cf3_assert(rho_a>=0);

      const Real gm1_ov_rhoa = gamma_minus_1 / rho_a;

      // matrix of right eigen vectors R

      right_eigenvectors(0,0) = nx;
      right_eigenvectors(0,1) = ny;
      right_eigenvectors(0,2) = nz;
      right_eigenvectors(0,3) = ra;
      right_eigenvectors(0,4) = ra;
      right_eigenvectors(1,0) = u*nx;
      right_eigenvectors(1,1) = u*ny - rho*nz;
      right_eigenvectors(1,2) = u*nz + rho*ny;
      right_eigenvectors(1,3) = ra*(u + a*nx);
      right_eigenvectors(1,4) = ra*(u - a*nx);
      right_eigenvectors(2,0) = v*nx + rho*nz;
      right_eigenvectors(2,1) = v*ny;
      right_eigenvectors(2,2) = v*nz - rho*nx;
      right_eigenvectors(2,3) = ra*(v + a*ny);
      right_eigenvectors(2,4) = ra*(v - a*ny);
      right_eigenvectors(3,0) = w*nx - rho*ny;
      right_eigenvectors(3,1) = w*ny + rho*nx;
      right_eigenvectors(3,2) = w*nz;
      right_eigenvectors(3,3) = ra*(w + a*nz);
      right_eigenvectors(3,4) = ra*(w - a*nz);
      right_eigenvectors(4,0) = 0.5*uuvvww*nx + rho*(v*nz - w*ny);
      right_eigenvectors(4,1) = 0.5*uuvvww*ny + rho*(w*nx - u*nz);
      right_eigenvectors(4,2) = 0.5*uuvvww*nz + rho*(u*ny - v*nx);
      right_eigenvectors(4,3) = ra*(H + a*um);
      right_eigenvectors(4,4) = ra*(H - a*um);

      // matrix of left eigen vectors L = R.inverse();

//      left_eigenvectors(0,0) = 1.- coeffM2;
//      left_eigenvectors(0,1) = uDivA*inv_a;
//      left_eigenvectors(0,2) = vDivA*inv_a;
//      left_eigenvectors(0,3) = -gamma_minus_1 * inv_a2;
//      left_eigenvectors(1,0) = inv_rho * (v*nx - u*ny);
//      left_eigenvectors(1,1) = inv_rho * ny;
//      left_eigenvectors(1,2) = -inv_rho * nx;
//      left_eigenvectors(1,3) = 0.0;
//      left_eigenvectors(2,0) = a*inv_rho * (coeffM2 - um*inv_a);
//      left_eigenvectors(2,1) = inv_rho * (nx - uDivA);
//      left_eigenvectors(2,2) = inv_rho * (ny - vDivA);
//      left_eigenvectors(2,3) = gm1_ov_rhoa;
//      left_eigenvectors(3,0) = a*inv_rho*(coeffM2 + um*inv_a);
//      left_eigenvectors(3,1) = -inv_rho*(nx + uDivA);
//      left_eigenvectors(3,2) = -inv_rho*(ny + vDivA);
//      left_eigenvectors(3,3) = gm1_ov_rhoa;
      left_eigenvectors(0,0) = nx*k2 - inv_rho*(v*nz - w*ny);
      left_eigenvectors(0,1) = uDivA*inv_a*nx;
      left_eigenvectors(0,2) = vDivA*inv_a*nx + nz*inv_rho;
      left_eigenvectors(0,3) = wDivA*inv_a*nx - ny*inv_rho;
      left_eigenvectors(0,4) = k3*nx;
      left_eigenvectors(1,0) = ny*k2 - inv_rho*(w*nx - u*nz);
      left_eigenvectors(1,1) = uDivA*inv_a*ny - nz*inv_rho;
      left_eigenvectors(1,2) = vDivA*inv_a*ny;
      left_eigenvectors(1,3) = wDivA*inv_a*ny + nx*inv_rho;
      left_eigenvectors(1,4) = k3*ny;
      left_eigenvectors(2,0) = nz*k2 - inv_rho*(u*ny - v*nx);
      left_eigenvectors(2,1) = uDivA*inv_a*nz + ny*inv_rho;
      left_eigenvectors(2,2) = vDivA*inv_a*nz - nx*inv_rho;
      left_eigenvectors(2,3) = wDivA*inv_a*nz;
      left_eigenvectors(2,4) = k3*nz;
      left_eigenvectors(3,0) = a*inv_rho*(k1 - um/a);
      left_eigenvectors(3,1) = inv_rho*(nx - uDivA);
      left_eigenvectors(3,2) = inv_rho*(ny - vDivA);
      left_eigenvectors(3,3) = inv_rho*(nz - wDivA);
      left_eigenvectors(3,4) = gamma_minus_1/rho_a;
      left_eigenvectors(4,0) = a*inv_rho*(k1 + um/a);
      left_eigenvectors(4,1) = inv_rho*(-nx - uDivA);
      left_eigenvectors(4,2) = inv_rho*(-ny - vDivA);
      left_eigenvectors(4,3) = inv_rho*(-nz - wDivA);
      left_eigenvectors(4,4) = gm1_ov_rhoa;

      // diagonal matrix of eigen values

      eigenvalues[0] = um;
      eigenvalues[1] = um;
      eigenvalues[2] = um;
      eigenvalues[3] = um + a;
      eigenvalues[4] = um - a;

      //abs_jacobian nog te definieren
      abs_jacobian.noalias() = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;

      // Compute left and right fluxes
      Real um_left = (properties_left[1]*nx + properties_left[2]*ny + properties_left[3]*nz);
      Real um_right = (properties_right[1]*nx + properties_right[2]*ny + properties_right[3]*nz);
      Real uuvvww_left = properties_left[1]*properties_left[1] + properties_left[2]*properties_left[2] + properties_left[3]*properties_left[3];
      Real uuvvww_right = properties_right[1]*properties_right[1] + properties_right[2]*properties_right[2] + properties_right[3]*properties_right[3];
      Real P_left = (gamma - 1.)/gamma * properties_left[0]*(properties_left[4] - 0.5*uuvvww_left + 0.5 * (m_Vt[0] * m_Vt[0] + m_Vt[1] * m_Vt[1] + m_Vt[2] * m_Vt[2]));
      Real P_right = (gamma - 1.)/gamma * properties_right[0]*(properties_right[4] - 0.5*uuvvww_right + 0.5 * (m_Vt[0] * m_Vt[0] + m_Vt[1] * m_Vt[1] + m_Vt[2] * m_Vt[2]));

      flux_left[0] = properties_left[0] * um_left;
      flux_left[1] = flux_left[0] * properties_left[1] + P_left*nx;
      flux_left[2] = flux_left[0] * properties_left[2] + P_left*ny;
      flux_left[3] = flux_left[0] * properties_left[3] + P_left*nx;
      flux_left[4] = flux_left[0] * properties_left[4];

      flux_right[0] = properties_right[0] * um_right;
      flux_right[1] = flux_right[0] * properties_right[1] + P_right*nx;
      flux_right[2] = flux_right[0] * properties_right[2] + P_right*ny;
      flux_right[3] = flux_right[0] * properties_right[3] + P_right*nz;
      flux_right[4] = flux_right[0] * properties_right[4] ;

      flux.noalias() = 0.5*(flux_left + flux_right);
      flux.noalias() -= 0.5*abs_jacobian*(right.solution-left.solution);
      wave_speed = eigenvalues.cwiseAbs().maxCoeff();
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:

  RealVectorNDIM m_Vt;

  RealVectorNEQS properties_left, properties_right;
  RealVectorNEQS properties_roe;

  RealVectorNEQS roe_avg;
  RealVectorNEQS roe_var_left;
  RealVectorNEQS roe_var_right;

  RealVectorNEQS flux_left;
  RealVectorNEQS flux_right;
  RealVectorNEQS flux;

  RealVector3 dummy, dummy_coord;

  RealVectorNEQS eigenvalues;
  Eigen::Matrix<Real, NEQS, NEQS> right_eigenvectors;
  Eigen::Matrix<Real, NEQS, NEQS> left_eigenvectors;
  Eigen::Matrix<Real, NEQS, NEQS> abs_jacobian;
};

////////////////////////////////////////////////////////////////////////////////

} // navierstokesmovingreferece
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_navierstokesmovingreference_Convection3D_hpp
