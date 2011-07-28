// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_WallWeakBc_hpp
#define CF_RDM_WallWeakBc_hpp

#include <iostream>

#include "RDM/BoundaryTerm.hpp"
#include "RDM/BcBase.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Mesh { class CMesh; class CField; }

namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_API WallWeakBc : public RDM::BoundaryTerm {

public: // typedefs

  /// the actual BC implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Term;

  /// pointers
  typedef boost::shared_ptr<WallWeakBc> Ptr;
  typedef boost::shared_ptr<WallWeakBc const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  WallWeakBc ( const std::string& name );

  /// Virtual destructor
  virtual ~WallWeakBc() {}

  /// Get the class name
  static std::string type_name () { return "WallWeakBc"; }

  /// execute the action
  virtual void execute ();

  virtual bool is_weak() const { return true; }

}; // !WallWeakBc

//------------------------------------------------------------------------------------------

template < typename SF, typename QD, typename PHYS >
class RDM_API WallWeakBc::Term : public BcBase<SF,QD,PHYS> {

public: // typedefs

  /// base class type
  typedef BcBase<SF,QD,PHYS> B;
  /// pointers
  typedef boost::shared_ptr< Term > Ptr;
  typedef boost::shared_ptr< Term const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
 Term ( const std::string& name ) :
   BcBase<SF,QD,PHYS>(name),
   m_quadrature( QD::instance() )

 {
   regist_typeinfo(this);

   // Gradient of the shape functions in reference space
   typename SF::MappedGradientT GradSF;
   // Values of shape functions in reference space
   typename SF::ShapeFunctionsT ValueSF;

   // initialize the interpolation matrix

   for(Uint q = 0; q < QD::nb_points; ++q)
     for(Uint n = 0; n < SF::nb_nodes; ++n)
     {
        SF::shape_function_gradient( m_quadrature.coords.col(q), GradSF  );
        SF::shape_function_value   ( m_quadrature.coords.col(q), ValueSF );

        Ni(q,n)     = ValueSF[n];
        dNdKSI(q,n) = GradSF[n];
     }

   // reflection matrix

   RM.setIdentity();
 }

 /// Get the class name
 static std::string type_name () { return "WallWeakBc.BC<" + SF::type_name() + ">"; }

protected: // data

 /// helper object to compute the quadrature information
 const QD& m_quadrature;

 typedef Eigen::Matrix<Real, QD::nb_points, 1u>               WeightVT;

 typedef typename SF::NodeMatrixT                             NodeMT;

 typedef Eigen::Matrix<Real, QD::nb_points, SF::nb_nodes >     SFMatrixT;
 typedef Eigen::Matrix<Real, QD::nb_points, PHYS::MODEL::_ndim   >     QCoordMT;
 typedef Eigen::Matrix<Real, SF::nb_nodes,  PHYS::MODEL::_neqs   >     SolutionMT;
 typedef Eigen::Matrix<Real, QD::nb_points, PHYS::MODEL::_neqs   >     QSolutionMT;
 typedef Eigen::Matrix<Real, PHYS::MODEL::_neqs,    PHYS::MODEL::_ndim   >     QSolutionVT;
 typedef Eigen::Matrix<Real, PHYS::MODEL::_neqs,    1u           >     SolutionVT;
 typedef Eigen::Matrix<Real, PHYS::MODEL::_neqs,    PHYS::MODEL::_ndim   >     FluxMT;
 typedef Eigen::Matrix<Real, PHYS::MODEL::_ndim,    1u           >     DimVT;

 /// derivative matrix - values of shapefunction derivative in Ksi at each quadrature point
 SFMatrixT  dNdKSI;
 /// interporlation matrix - values of shapefunction at each quadrature point
 SFMatrixT  Ni;
 /// node values
 NodeMT     X_n;
 /// coordinates of quadrature points in physical space
 QCoordMT   X_q;
 /// derivatives of coordinate transformation physical->reference space
 QCoordMT  dX_q;
 /// Values of the solution located in the dof of the element
 SolutionMT U_n;
 /// solution at quadrature points in physical space
 QSolutionMT U_q;
 /// derivatives of solution to X at each quadrature point, one matrix per dimension
 QSolutionMT dUdX[PHYS::MODEL::_ndim];
 /// derivatives of solution with respect to x,y,z at ONE quadrature point
 QSolutionVT dUdXq;
 /// Integration factor (jacobian multiplied by quadrature weight)
 WeightVT wj;
 /// contribution to nodal residuals
 SolutionMT Phi_n;

 /// flux computed with current solution
 FluxMT Fu_h;
 /// flux computed with boundary value
 FluxMT Fu_g;
 /// ghost solution vector
 SolutionVT u_g;
 /// average solution vector between u_h and u_g
 SolutionVT u_avg;

 /// diagonal matrix with eigen values
 typename B::PhysicsVT Dv;
 /// diagonal matrix with positive eigen values
 typename B::PhysicsVT DvPlus;
 /// matrix of left and right eigenvectors
 typename B::PhysicsMT Lv;
 typename B::PhysicsMT Rv;
 typename B::PhysicsMT P;
 typename B::PhysicsVT dF;

 /// temporary normal on 1 quadrature point
 DimVT dN;

 /// reflexion matrix for setting velocity tangent to wall
 Eigen::Matrix<Real, PHYS::MODEL::_neqs, PHYS::MODEL::_neqs >  RM;

public: // functions

 /// virtual interface to execute the action
 virtual void execute () { executeT(); }

 /// execute the action
 void executeT ()
 {
//    std::cout << "Face [" << B::idx() << "]" << std::endl;

   // get face connectivity

   const Mesh::CTable<Uint>::ConstRow nodes_idx = this->connectivity_table->array()[B::idx()];

   // copy the coordinates from the large array to a small

   Mesh::fill(X_n, *B::coordinates, nodes_idx );

   // copy the solution from the large array to a small

   for(Uint n = 0; n < SF::nb_nodes; ++n)
     for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
       U_n(n,v) = (*B::solution)[ nodes_idx[n] ][v];

   // coordinates of quadrature points in physical space

   X_q  = Ni * X_n;

   // derivatives of coordinate transf.

   dX_q = dNdKSI * X_n;

   // solution at all quadrature points in physical space

   U_q = Ni * U_n;

#if 0

   const Real jacob = std::sqrt( dX_q(0,XX)*dX_q(0,XX)+dX_q(0,YY)*dX_q(0,YY) );

   dN[XX] =  dX_q(0,YY)/jacob;
   dN[YY] = -dX_q(0,XX)/jacob;

   //   if ( X_q.row(0)[XX] > 0.5 && X_q.row(0)[XX] < 1. && X_q.row(0)[YY] < 0.25  )
   //   {
   //       std::cout << "Xq       : " << X_q.row(0) << std::endl;
   //       std::cout << "dN[XX]   : " << dN[XX]     << std::endl;
   //       std::cout << "dN[YY]   : " << dN[YY]     << std::endl;
   //   }

   // modify directly the residual

   for (Uint n=0; n < SF::nb_nodes; ++n)
   {
     const Real un = (*B::residual)[nodes_idx[n]][1] * dN[XX]
                   + (*B::residual)[nodes_idx[n]][2] * dN[YY];

     (*B::residual)[nodes_idx[n]][1] += un * dN[XX];
     (*B::residual)[nodes_idx[n]][2] += un * dN[YY];
   }

#else

   // zero element residuals

   Phi_n.setZero();

   // loop on quadrature points

   for(Uint q=0; q < QD::nb_points; ++q)
   {

    for(Uint dim = 0; dim < PHYS::MODEL::_ndim; ++dim)
    {
      dUdXq.col(dim) = dUdX[dim].row(q).transpose();
    }

    const Real jacob = std::sqrt( dX_q(q,XX)*dX_q(q,XX)+dX_q(q,YY)*dX_q(q,YY) );

    wj[q] = jacob * m_quadrature.weights[q];

    // compute the normal at quadrature point

    dN[XX] =  dX_q(q,YY)/jacob;
    dN[YY] = -dX_q(q,XX)/jacob;

//    std::cout << "Xq       : " << X_q.row(q) << std::endl;
//    std::cout << "dN[XX]   : " << dN[XX] << std::endl;
//    std::cout << "dN[YY]   : " << dN[YY] << std::endl;

    // compute the flux F(u_h) and its correction F(u_g)

    PHYS::compute_properties(X_q.row(q),
                             U_q.row(q),
                             dUdXq,
                             B::phys_props);

    PHYS::flux(B::phys_props, Fu_h);

    // compute the ghost state and correction - Mario:
    const Real u_n = B::phys_props.u*dN[XX] + B::phys_props.v*dN[YY];

#if 1
    u_g[0] = B::phys_props.rho;
    u_g[1] = B::phys_props.rho  * ( B::phys_props.u - 2 * u_n*dN[XX] );
    u_g[2] = B::phys_props.rho  * ( B::phys_props.v - 2 * u_n*dN[YY] );
    u_g[3] = B::phys_props.rhoE; // - 0.5 * B::phys_props.rho * u_n * u_n;
#else
    u_g[0] = B::phys_props.rho;
    u_g[1] = B::phys_props.rho  * ( B::phys_props.u - u_n*dN[XX] );
    u_g[2] = B::phys_props.rho  * ( B::phys_props.v - u_n*dN[YY] );
    u_g[3] = B::phys_props.rhoE - 0.5 * B::phys_props.rho * u_n * u_n;
#endif


    PHYS::compute_properties(X_q.row(q),
                             u_g,
                             dUdXq,
                             B::phys_props);

    PHYS::flux(B::phys_props, Fu_g);

    // compute eigen structure in average state

    u_avg = 0.5 * ( U_q.row(q).transpose() + u_g );

    PHYS::compute_properties(X_q.row(q),
                             u_avg,
                             dUdXq,
                             B::phys_props);

    PHYS::flux_jacobian_eigen_structure(B::phys_props,dN,Rv,Lv,Dv);

    {
      std::cout << "identity? " << Rv*Lv << std::endl;
    }

    for(Uint dim = 0; dim < PHYS::MODEL::_ndim; ++dim)
      if(Dv[dim] >= 0.0)
        Lv.row(dim).setZero();

    P = Rv*Lv;

    dF = ( Fu_g.col(XX) - Fu_h.col(XX) ) * dN[XX] +
         ( Fu_g.col(YY) - Fu_h.col(YY) ) * dN[YY];

// compute the reflection matrix

//    RM(1,1) = 1. - 2*dN[XX]*dN[XX];
//    RM(1,2) =    - 2*dN[XX]*dN[YY];
//    RM(2,1) =    - 2*dN[YY]*dN[XX];
//    RM(2,2) = 1. - 2*dN[YY]*dN[YY];

//    RM(1,1) = dN[YY]*dN[YY] - dN[XX]*dN[XX];
//    RM(1,2) = -2.*dN[XX]*dN[YY];
//    RM(2,1) = -2.*dN[XX]*dN[YY];
//    RM(2,2) = dN[XX]*dN[XX] - dN[YY]*dN[YY];

//    RM(1,1) =  dN[YY]*dN[YY];
//    RM(1,2) = -dN[XX]*dN[YY];
//    RM(2,1) = -dN[XX]*dN[YY];
//    RM(2,2) =  dN[XX]*dN[XX];

// reflect the current solution on the wall to get the ghost solution

//    u_g = RM * U_q.row(q).transpose();

// compute the flux according to the ghost solution u_g


    for(Uint n=0; n < SF::nb_nodes; ++n)
    {
      Phi_n.row(n) += ( P * dF * Ni(q,n) * wj[q] );
    }

#if 0
    // compute the wave_speed for scaling the update

    PHYS::flux_jacobian_eigen_values(B::phys_props,
                                dN,
                                Dv );

    DvPlus = Dv.unaryExpr(std::ptr_fun(plus));

    for(Uint n = 0; n < SF::nb_nodes; ++n)
      (*B::wave_speed)[nodes_idx[n]][0] += DvPlus.array().maxCoeff() * wj[q];
#endif


   } //Loop over quadrature points

   // update the residual

   for (Uint n=0; n<SF::nb_nodes; ++n)
     for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
       (*B::residual)[nodes_idx[n]][v] += Phi_n(n,v);

#endif

 }

}; // !WallWeakBc::Term

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_WallWeakBc_hpp
