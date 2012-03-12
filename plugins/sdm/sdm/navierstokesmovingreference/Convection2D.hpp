// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_navierstokesmovingreference_Convection2D_hpp
#define cf3_sdm_navierstokesmovingreference_Convection2D_hpp

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

class sdm_navierstokes_API Convection2D : public ConvectiveTerm< PhysDataBase<4u,2u> >
{
private:
    Real gamma;
    RealVector2 Vtrans;
    RealVector3 Omega;

    void config_Omega()
    {
        std::vector<Real> Omega_vec= options().option("Omega").value< std::vector<Real> >();
        cf3_assert(Omega_vec.size() == 3);
        cf3_assert(Omega_vec[0] == 0);
        cf3_assert(Omega_vec[1] == 0);
        Omega[0] = Omega_vec[0];
        Omega[1] = Omega_vec[1];
        Omega[2] = Omega_vec[2];
    }

    void config_Vtrans()
    {
        std::vector<Real> Vtrans_vec= options().option("Vtrans").value< std::vector<Real> >();
        cf3_assert(Vtrans_vec.size() == 2);
        Vtrans[0] = Vtrans_vec[0];
        Vtrans[1] = Vtrans_vec[1];
    }

public:
  static std::string type_name() { return "Convection2D"; }
  Convection2D(const std::string& name) : ConvectiveTerm< PhysData >(name),
                                          gamma(1.4)
  {
        std::vector<Real> OmegaDefault (3,0), VtransDefault(2,0);
        OmegaDefault[0] = Omega[0];
        OmegaDefault[1] = Omega[1];
        OmegaDefault[2] = Omega[2];

        VtransDefault[0] = Vtrans[0];
        VtransDefault[1] = Vtrans[1];

        options().add_option("Omega", OmegaDefault)
            .description("Rotation vector")
            .mark_basic()
            .attach_trigger(boost::bind( &Convection2D::config_Omega, this));

        options().add_option("Vtrans", VtransDefault)
            .description("Vector of the translation speeds")
            .mark_basic()
            .attach_trigger( boost::bind( &Convection2D::config_Vtrans, this));

      options().add_option("gamma", gamma)
          .description("The heat capacity ratio")
          .link_to(&gamma);
  }

  virtual ~Convection2D() {}

  RealVector2 transformation_velocity(const RealVector& coord) const
  {
      RealVector2 Vt;
      RealVector3 dummy, dummy_coord;

      dummy_coord[0] = coord[0];
      dummy_coord[1] = coord[1];
      dummy_coord[2] = 0.;

      Vt = Vtrans;
      dummy = Omega.cross(dummy_coord);

      Vt[0] += dummy[0];
      Vt[1] += dummy[1];

      return Vt;
  }

  virtual void compute_analytical_flux(PhysData& data, const RealVectorNDIM& unit_normal,
                                       RealVectorNEQS& flux, Real& wave_speed)
  {
      Real rho, rhou, rhov, rhoE;
      Real u, v, H, P;
      Real um;
      Real a; // speed of sound

      RealVector2 Vt = transformation_velocity(data.coord);

      rho   = data.solution[0];
      rhou  = data.solution[1];
      rhov  = data.solution[2];
      rhoE  = data.solution[3];

      u     = rhou / rho;
      v     = rhov / rho;
      P     = (gamma - 1) * (rhoE - 0.5 * rho *(u*u + v*v) + 0.5 * rho * ( Vt.dot(Vt)));
      H     = rhoE / rho + P / rho;

      um    = u * unit_normal[XX] + v * unit_normal[YY];

      a     = std::sqrt(gamma * P / rho);


      flux[0] = rho * um;
      flux[1] = rho * um * u + P * unit_normal[XX];
      flux[2] = rho * um * v + P * unit_normal[YY];
      flux[3] = rho * um * H;

      wave_speed = std::max(std::abs(um + a), std::abs(um - a));
  }

  void compute_properties(const PhysData& data, RealVectorNEQS& properties)
  {
      Real P;
      RealVector2 Vt= transformation_velocity(data.coord);

      properties[0]  = data.solution[0];                //rho
      properties[1]  = data.solution[1]/properties[0];  //u
      properties[2]  = data.solution[2]/properties[0];  //v
      P = (gamma-1.)*(data.solution[3]-0.5*properties[0]*(properties[1]*properties[1]+properties[2]*properties[2])+0.5*properties[0]*(Vt[0] * Vt[0] + Vt[1] * Vt[1]));
      properties[3]  = (data.solution[3] + P) / properties[0];            //H

  }

  virtual void compute_numerical_flux(PhysData& left, PhysData& right, const RealVectorNDIM& unit_normal,
                                      RealVectorNEQS& flux, Real& wave_speed)
  {
      RealVectorNEQS properties_left, properties_right;
      RealVectorNEQS properties_roe;
      Real P;
      RealVector Vt = transformation_velocity(0.5*(left.coord+right.coord));//welke coord moet ik nemen?

      // computation of the left and right properties
      compute_properties(left, properties_left);
      compute_properties(right, properties_right);

      // computation of the left and right roe variables and roe average
      roe_var_left[0] = std::sqrt(properties_left[0]);                // sqrt(rho)
      roe_var_left[1] = roe_var_left[0] * properties_left[1];         // sqrt(rho)*u
      roe_var_left[2] = roe_var_left[0] * properties_left[2];         // sqrt(rho)*v
      roe_var_left[3] = roe_var_left[0] * properties_left[3];         // sqrt(rho)*H

      roe_var_right[0] = std::sqrt(properties_right[0]);              // sqrt(rho)
      roe_var_right[1] = roe_var_right[0] * properties_right[1];      // sqrt(rho)*u
      roe_var_right[2] = roe_var_right[0] * properties_right[2];      // sqrt(rho)*v
      roe_var_right[3] = roe_var_right[0] * properties_right[3];      // sqrt(rho)*H

      roe_avg.noalias() = 0.5*(roe_var_left + roe_var_right);

      properties_roe[0] = roe_avg[0]*roe_avg[0];            //rho
      properties_roe[1] = roe_avg[1]/roe_avg[0];            //u
      properties_roe[2] = roe_avg[2]/roe_avg[0];            //v
      properties_roe[3] = roe_avg[3]/roe_avg[0];            //H
      P = properties_roe[0] * (gamma - 1.)/gamma*(properties_roe[3]-0.5
                                                 *(properties_roe[1]*properties_roe[1] + properties_roe[2]*properties_roe[2]) + 0.5*(Vt[0] * Vt[0] + Vt[1] * Vt[1]));
                                                 //(gamma-1)/gamma * rho (H-0.5*(uu+vv)+0.5*Vt*Vt)

      const Real nx = unit_normal[XX];
      const Real ny = unit_normal[YY];

      const Real rho = properties_roe[0];
      const Real u = properties_roe(1);
      const Real v = properties_roe(2);
      const Real H = properties_roe(3);
      const Real a = std::sqrt(gamma * P / rho);

      const Real gamma_minus_1 = gamma - 1.;
      const Real uuvv = u*u + v*v;
      const Real half_gm1_v2 = 0.5 * gamma_minus_1 * uuvv;
      const Real inv_rho = 1./rho;

      const Real inv_a  = 1. / a;
      const Real inv_a2 = inv_a * inv_a;

      const Real um = u * nx + v * ny;
      const Real ra = 0.5 * rho * inv_a;

      const Real coeffM2 = half_gm1_v2 * inv_a2;
      const Real uDivA = gamma_minus_1 * u * inv_a;
      const Real vDivA = gamma_minus_1 * v * inv_a;
      const Real rho_a = rho * a;

      const Real gm1_ov_rhoa = gamma_minus_1 / rho_a;

      // matrix of right eigen vectors R

      Eigen::Matrix<Real, NEQS, NEQS> righteigenvectors, lefteigenvectors;
      RealVectorNEQS matrixeigenvalues;

      righteigenvectors(0,0) = 1.;
      righteigenvectors(0,1) = 0.;
      righteigenvectors(0,2) = ra;
      righteigenvectors(0,3) = ra;
      righteigenvectors(1,0) = u;
      righteigenvectors(1,1) = rho * ny;
      righteigenvectors(1,2) = ra*(u + a*nx);
      righteigenvectors(1,3) = ra*(u - a*nx);
      righteigenvectors(2,0) = v;
      righteigenvectors(2,1) = -rho*nx;
      righteigenvectors(2,2) = ra*(v + a*ny);
      righteigenvectors(2,3) = ra*(v - a*ny);
      righteigenvectors(3,0) = 0.5 * uuvv;
      righteigenvectors(3,1) = rho * (u*ny - v*nx);
      righteigenvectors(3,2) = ra*(H + a*um);
      righteigenvectors(3,3) = ra*(H - a*um);

      // matrix of left eigen vectors L = R.inverse();

      lefteigenvectors(0,0) = 1.- coeffM2;
      lefteigenvectors(0,1) = uDivA*inv_a;
      lefteigenvectors(0,2) = vDivA*inv_a;
      lefteigenvectors(0,3) = -gamma_minus_1 * inv_a2;
      lefteigenvectors(1,0) = inv_rho * (v*nx - u*ny);
      lefteigenvectors(1,1) = inv_rho * ny;
      lefteigenvectors(1,2) = -inv_rho * nx;
      lefteigenvectors(1,3) = 0.0;
      lefteigenvectors(2,0) = a*inv_rho * (coeffM2 - um*inv_a);
      lefteigenvectors(2,1) = inv_rho * (nx - uDivA);
      lefteigenvectors(2,2) = inv_rho * (ny - vDivA);
      lefteigenvectors(2,3) = gm1_ov_rhoa;
      lefteigenvectors(3,0) = a*inv_rho*(coeffM2 + um*inv_a);
      lefteigenvectors(3,1) = -inv_rho*(nx + uDivA);
      lefteigenvectors(3,2) = -inv_rho*(ny + vDivA);
      lefteigenvectors(3,3) = gm1_ov_rhoa;

      // diagonal matrix of eigen values

      matrixeigenvalues[0] = um;
      matrixeigenvalues[1] = um;
      matrixeigenvalues[2] = um + a;
      matrixeigenvalues[3] = um - a;

      //abs_jacobian nog te definieren
      abs_jacobian.noalias() = righteigenvectors * matrixeigenvalues.cwiseAbs().asDiagonal() * lefteigenvectors;

      // Compute left and right fluxes
      RealVectorNEQS flux_left, flux_right;
      Real um_left = (properties_left[1]*nx + properties_left[2]*ny);
      Real um_right = (properties_right[1]*nx + properties_right[2]*ny);
      Real uuvv_left = properties_left[1]*properties_left[1] + properties_left[2]*properties_left[2];
      Real uuvv_right = properties_right[1]*properties_right[1] + properties_right[2]*properties_right[2];
      Real P_left = (gamma - 1.)/gamma * properties_left[0]*(properties_left(3) - 0.5*uuvv_left + 0.5 * (Vt[0] * Vt[0] + Vt[1] * Vt[1]));
      Real P_right = (gamma - 1.)/gamma * properties_right[0]*(properties_right(3) - 0.5*uuvv_right + 0.5 * (Vt[0] * Vt[0] + Vt[1] * Vt[1]));

      flux_left[0] = properties_left[0] * um_left;
      flux_left[1] = flux_left[0] * properties_left[1] + P_left*nx;
      flux_left[2] = flux_left[0] * properties_left[2] + P_left*ny;
      flux_left[3] = flux_left[0] * properties_left[3];

      flux_right[0] = properties_right[0] * um_right;
      flux_right[1] = flux_right[0] * properties_right[1] + P_right*nx;
      flux_right[2] = flux_right[0] * properties_right[2] + P_right*ny;
      flux_right[3] = flux_right[0] * properties_right[3] ;

      flux.noalias() = 0.5*(flux_left + flux_right);
      flux.noalias() -= 0.5*abs_jacobian*(right.solution-left.solution);
      wave_speed = eigenvalues.cwiseAbs().maxCoeff();

  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:

  RealVectorNEQS roe_avg;
  RealVectorNEQS roe_var_left;
  RealVectorNEQS roe_var_right;

  RealVectorNEQS flux_left;
  RealVectorNEQS flux_right;
  RealVectorNEQS flux;

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

#endif // cf3_sdm_navierstokesmovingreference_Convection2D_hpp
