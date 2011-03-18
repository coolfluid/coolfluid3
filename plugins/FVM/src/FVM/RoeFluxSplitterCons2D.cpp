// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/Log.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/LibMesh.hpp"

#include "FVM/RoeFluxSplitterCons2D.hpp"

namespace CF {
namespace FVM {

using namespace Common;

Common::ComponentBuilder < RoeFluxSplitterCons2D, Component, LibFVM > RoeFluxSplitterCons2D_Builder;

////////////////////////////////////////////////////////////////////////////////

RoeFluxSplitterCons2D::RoeFluxSplitterCons2D ( const std::string& name  ) 
: Component(name),
  m_g(1.4),
  m_gm1(m_g-1.),
  m_roe_avg(4)
{
  properties()["brief"] = std::string("Roe Flux Splitter");
  properties()["description"] = std::string("Solves the Riemann problem using the Roe scheme");
}

////////////////////////////////////////////////////////////////////////////////

RoeFluxSplitterCons2D::~RoeFluxSplitterCons2D()
{
}

////////////////////////////////////////////////////////////////////////////////

RealVector RoeFluxSplitterCons2D::interface_flux(const RealVector& left, const RealVector& right, const RealVector& normal)
{
  RealVector interface_flux(4);
  Real dummy; // not interested in wavespeeds

  solve( 
          //input
          left,right,normal,
          //output
          interface_flux,dummy,dummy
        );

  return interface_flux;
}

////////////////////////////////////////////////////////////////////////////////

void RoeFluxSplitterCons2D::solve(const RealVector& left, const RealVector& right, const RealVector& normal , 
                            RealVector& interface_flux, Real& left_wave_speed, Real& right_wave_speed)
{
  // compute the roe average
  compute_roe_average(left,right,m_roe_avg);

  const Real r=m_roe_avg[0];
  const Real u=m_roe_avg[1]/r;
  const Real v=m_roe_avg[2]/r;
  const Real h = m_g*m_roe_avg[3]/r - 0.5*m_gm1*u*u;
  const Real a = sqrt(m_gm1*(h-0.5*(u*u+v*v)));

  const Real nx = normal[XX];
  const Real ny = normal[YY];
  const Real un = u*nx + v*ny; // normal roe-average speed 
  
  // right eigenvectors
  RealMatrix4 right_eigenvectors; right_eigenvectors << 
  
        1.,             0.,             0.5*r/a,             0.5*r/a,
        u,              r*ny,           0.5*r/a*(u+a*nx),    0.5*r/a*(u-a*nx),
        v,              -r*nx,          0.5*r/a*(v+a*ny),    0.5*r/a*(v-a*ny),
        0.5*(u*u+v*v),  r*(u*ny-v*nx),  0.5*r/a*(h+a*un),    0.5*r/a*(h-a*un);


  // left eigenvectors = inverse(rc)
  RealMatrix4 left_eigenvectors; left_eigenvectors << 
  
        1.-0.5*m_gm1*(u*u+v*v)/(a*a),            m_gm1*u/(a*a),           m_gm1*v/(a*a),        -m_gm1/(a*a),
        1./r*(v*nx-u*ny),                        1./r*ny,                -1./r*nx,               0.,
        a/r*(0.5*m_gm1*(u*u+v*v)/(a*a)-un/a),    1./r*(nx-m_gm1*u/a),     1./r*(ny-m_gm1*v/a),   m_gm1/(r*a),
        a/r*(0.5*m_gm1*(u*u+v*v)/(a*a)+un/a),   -1./r*(nx+m_gm1*u/a),    -1./r*(ny+m_gm1*v/a),   m_gm1/(r*a);
  

  // eigenvalues
  RealVector4 eigenvalues; eigenvalues <<    
  
        un, un,  un+a,  un-a;

  
  // calculate absolute jacobian
  RealMatrix4 abs_jacobian = right_eigenvectors * abs_eigenvalues(eigenvalues).asDiagonal() * left_eigenvectors;

  // flux = central part + upwind part
  interface_flux = 0.5*(flux(left,normal)+flux(right,normal)) - 0.5*abs_jacobian*(right-left);

  left_wave_speed  =  un+a;
  right_wave_speed = -un+a;
}

////////////////////////////////////////////////////////////////////////////////

void RoeFluxSplitterCons2D::compute_roe_average(const RealVector& left, const RealVector& right, RealVector& roe_avg) const
{  
  const Real rho_L  = left[0];       const Real rho_R  = right[0];
  const Real rhou_L = left[1];       const Real rhou_R = right[1];
  const Real rhov_L = left[2];       const Real rhov_R = right[2];
  const Real rhoE_L = left[3];       const Real rhoE_R = right[3];
  
  // convert to roe variables
  const Real u_L = rhou_L/rho_L;     const Real u_R = rhou_R/rho_R;
  const Real v_L = rhov_L/rho_L;     const Real v_R = rhov_R/rho_R;
  const Real h_L = m_g*rhoE_L/rho_L - 0.5*m_gm1*(u_L*u_L+v_L*v_L);
  const Real h_R = m_g*rhoE_R/rho_R - 0.5*m_gm1*(u_R*u_R+v_R*v_R);
  
  const Real sqrt_rho_L = sqrt(rho_L);
  const Real sqrt_rho_R = sqrt(rho_R);

  // compute roe average quantities
  const Real rho_A = sqrt_rho_L * sqrt_rho_R;
  const Real u_A   = (sqrt_rho_L*u_L + sqrt_rho_R*u_R) / (sqrt_rho_L + sqrt_rho_R);
  const Real v_A   = (sqrt_rho_L*v_L + sqrt_rho_R*v_R) / (sqrt_rho_L + sqrt_rho_R);
  const Real h_A   = (sqrt_rho_L*h_L + sqrt_rho_R*h_R) / (sqrt_rho_L + sqrt_rho_R);
  
  // return as conserved variables
  roe_avg[0] = rho_A;
  roe_avg[1] = rho_A * u_A;
  roe_avg[2] = rho_A * v_A;
  roe_avg[3] = rho_A/m_g * (h_A + 0.5*(m_gm1*(u_A*u_A+v_A*v_A) ) );

}

////////////////////////////////////////////////////////////////////////////////

RealVector RoeFluxSplitterCons2D::flux(const RealVector& state, const RealVector& normal) const
{
  const Real r=state[0];
  const Real u=state[1]/r;
  const Real v=state[2]/r;
  const Real rE = state[3];
  const Real p = m_gm1*(rE-0.5*r*(u*u+v*v));
  const Real un = u*normal[XX] + v*normal[YY]; // normal speed
  RealVector4 F;
  F <<     r*un,   
           r*u*un + p*normal[XX],
           r*v*un + p*normal[YY],
           (rE+p)*un;
  return F;
}

////////////////////////////////////////////////////////////////////////////////

 // //compute eigen values of the left state
 //   vector<RealVector>& pdata = getMethodData().getPolyReconstructor()->getExtrapolatedPhysicaData();
 //   SafePtr<ConvectiveVarSet> updateVarSet = getMethodData().getUpdateVar();
 //   const RealVector& unitNormal = getMethodData().getUnitNormal();
 //   
 //   updateVarSet->computeEigenValues(pdata[0],
 //             unitNormal,
 //             _leftEvalues);
 //   
 //   //compute eigen values of the right state
 //   updateVarSet->computeEigenValues(pdata[1],
 //             unitNormal,
 //             _rightEvalues);
 //   
 //   CFreal lambdaCorr = 0.0;
 //   computeLambdaCorr(lambdaCorr);
 //   lambdaCorr *= 0.5;
 //   const CFuint nbEqs = PhysicalModelStack::getActive()->getNbEq();
 //   
 //   if (_entropyFixID == 1) {
 //     for (CFuint i = 0; i < nbEqs; ++i) {
 //       _absEvalues[i] = max(std::abs(_eValues[i]), lambdaCorr);
 //     }
 //   }
 //   else if (_entropyFixID == 2) {
 //     const CFreal twoLambdaCorr = 2.0*lambdaCorr;
 //     for (CFuint i = 0; i < nbEqs; ++i) {
 //       const CFreal absLambda = std::abs(_eValues[i]);
 //       _absEvalues[i] = (!(std::abs(_eValues[i]) < twoLambdaCorr)) ?
 //    absLambda : (absLambda*absLambda/(4.0*lambdaCorr) + lambdaCorr);
 //     }
 //   }
 //   
 //   
 //   
 //   void computeLambdaCorr(CFreal& lambdaCorr) const
 //   {
 //     const CFuint nbEqs = Framework::PhysicalModelStack::getActive()->getNbEq();
 //     for (CFuint i = 0; i < nbEqs; ++i) {
 //       lambdaCorr = std::max(lambdaCorr,
 //        std::abs(_rightEvalues[i] - _leftEvalues[i]));
 //     }
 //   }
 // 
 // 
 //   lambdastar[0]=ustar-astar;
 //   lambdastar[1]=ustar;
 //   lambdastar[2]=ustar+astar;
 // 
 //   double thetastar[3], deltastar[3];
 // 
 //   thetastar[0]=max(0 , 2.0*( (uR-aR) - (uL-aL) ) );
 //   thetastar[1]=max(0 , 2.0*(uR-uL) );
 //   thetastar[2]=max(0 , 2.0*( (uR+aR) - (uL+aL) ) );
 // 
 //   for(int i=0; i<3; i++)
 //   {
 //     if(fabs(lambdastar[i])>=thetastar[i] || thetastar[i]==0 )
 //       deltastar[i]=0;
 //     else if( fabs(lambdastar[i])<thetastar[i])
 //       deltastar[i]= 1.0/2.0*(pow(lambdastar[i],2)/thetastar[i] + thetastar[i] ) - fabs(lambdastar[i]);
 //   }
 // 
 //   for(int i=0; i<3; i++)
 //     lambdastar[i] += deltastar[i];

////////////////////////////////////////////////////////////////////////////////

RealVector RoeFluxSplitterCons2D::abs_eigenvalues(const RealVector& eigenvalues) const
{
  return eigenvalues.cwiseAbs();
}
////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF
