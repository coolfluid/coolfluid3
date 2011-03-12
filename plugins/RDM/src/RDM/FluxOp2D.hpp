// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_FluxOp2D_hpp
#define CF_Solver_FluxOp2D_hpp

#include <Eigen/Dense>
#include "Math/MatrixTypes.hpp"
#include <iostream>

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
class RDM_API FluxOp2D
{
public: // typedefs

  /// output matrix types
  typedef Eigen::Matrix<Real, QUADRATURE::nb_points, SHAPEFUNC::nb_nodes> SFMatrixT;
  typedef Eigen::Matrix<Real, SHAPEFUNC::nb_nodes, PHYSICS::nbeqs()> SolutionMatrixT;

public: // functions
  /// Contructor
  /// @param name of the component
  FluxOp2D ( );

  /// Virtual destructor
  virtual ~FluxOp2D() {};

  /// Get the class name
  static std::string type_name () { return "FluxOp2D<" + SHAPEFUNC::type_name() + "," + QUADRATURE::type_name() +  ">"; }

  /// Compute the operator values at all quadrature nodes
  void compute(const typename SHAPEFUNC::NodeMatrixT & nodes,
               const SolutionMatrixT & solution,
               SFMatrixT& sf_oper_values,
               RealVector& flux_oper_values,Eigen::Matrix<Real,QUADRATURE::nb_points, 1u> & wj);
    
protected: // data

  const QUADRATURE& m_quadrature;

  RealVector2 m_qd_pt; //Quadrature point in physical space
  RealVector2 m_dN;    //Derivatives of shape functions in physical space

  Real m_u_q; //Solution value at quadrature point

  Eigen::Matrix<Real,QUADRATURE::nb_points, SHAPEFUNC::nb_nodes> m_N;
  Eigen::Matrix<Real,QUADRATURE::nb_points, SHAPEFUNC::nb_nodes> m_dNdksi;
  Eigen::Matrix<Real,QUADRATURE::nb_points, SHAPEFUNC::nb_nodes> m_dNdeta;

  typename SHAPEFUNC::MappedGradientT m_sf_grad_ref; //Gradient of the shape functions in reference space
  typename SHAPEFUNC::ShapeFunctionsT m_sf_ref;   //Values of shape functions in reference space

  Eigen::Matrix<Real,QUADRATURE::nb_points, DIM_2D> m_qdpt_phys; //coordinates of quadrature points in physical space
  Eigen::Matrix<Real,QUADRATURE::nb_points, PHYSICS::nbeqs() > m_u_phys; //solution at quadrature points in physical space
  Eigen::Matrix<Real,QUADRATURE::nb_points, DIM_2D> m_u_grad_phys; //derivatives of solution

  Eigen::Matrix<Real,QUADRATURE::nb_points, DIM_2D> m_dx; //Stores dx/dksi and dx/deta at each quadrature point
  Eigen::Matrix<Real,QUADRATURE::nb_points, DIM_2D> m_dy; //Stores dy/dksi and dy/deta at each quadrature point

  Eigen::Matrix<Real,QUADRATURE::nb_points, 1u> m_j;      //jacobian of transformation phys->ref at each qd. pt

  Eigen::Matrix<Real,QUADRATURE::nb_points, SHAPEFUNC::nb_nodes> m_dNdx; //Derivatives of shape functions
                                                                        //at all quadrature points in phys. space
  Eigen::Matrix<Real,QUADRATURE::nb_points, SHAPEFUNC::nb_nodes> m_dNdy;

};


template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
FluxOp2D<SHAPEFUNC,QUADRATURE,PHYSICS>::FluxOp2D() : m_quadrature( QUADRATURE::instance() )
{
   ///Set up the matrices to be used for interpolation
   for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
   {
//     std::cout << "W[" << q << "] = " << m_quadrature.weights[q] << std::endl;

     for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
     {
        SHAPEFUNC::mapped_gradient( m_quadrature.coords.col(q), m_sf_grad_ref );
        SHAPEFUNC::shape_function ( m_quadrature.coords.col(q), m_sf_ref   );

        m_N(q,n) = m_sf_ref[n];
        m_dNdksi(q,n) = m_sf_grad_ref(KSI,n);
        m_dNdeta(q,n) = m_sf_grad_ref(ETA,n);
     }
   }
}



template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
void FluxOp2D<SHAPEFUNC,QUADRATURE,PHYSICS>::compute(const typename SHAPEFUNC::NodeMatrixT & nodes,
                                                     const SolutionMatrixT& solution,
                                                     SFMatrixT& sf_oper_values,
                                                     RealVector& flux_oper_values, Eigen::Matrix<Real, QUADRATURE::nb_points, 1u> & wj)
{
   //Coordinates of quadrature points in physical space
   m_qdpt_phys  = m_N * nodes;
   //Solution at all quadrature points in physical space
   m_u_phys = m_N * solution;

   //Jacobian of transformation phys -> ref:
   //    |   dx/dksi    dx/deta    |
   //    |   dy/dksi    dy/deta    |
   // m_dx.col(KSI) has the values of dx/dksi at all quadrature points
   // m_dx.col(ETA) has the values of dx/deta at all quadrature points
   m_dx.col(KSI) = m_dNdksi * nodes.col(XX);
   m_dx.col(ETA) = m_dNdeta * nodes.col(XX);
   m_dy.col(KSI) = m_dNdksi * nodes.col(YY);
   m_dy.col(ETA) = m_dNdeta * nodes.col(YY);

   for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
       m_j[q] = m_dx(q,XX) * m_dy(q,YY) - m_dx(q,YY) * m_dy(q,XX);

  //m_dN_phys.setZero();

   // Shape function derivatives in physical space at quadrature pts
   for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
     for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
     {
       m_dNdx(q,n) = 1.0/m_j[q] * (  m_dNdksi(q,n)*m_dy(q,YY) - m_dNdeta(q,n) * m_dy(q,XX));
       m_dNdy(q,n) = 1.0/m_j[q] * ( -m_dNdksi(q,n)*m_dx(q,YY) + m_dNdeta(q,n) * m_dx(q,XX));
     }

   m_u_grad_phys.col(XX) = m_dNdx * solution;
   m_u_grad_phys.col(YY) = m_dNdy * solution;

  RealVector2 coord_q;
  RealVector2 dN;
  RealVector2 du;

  for(Uint q=0; q < QUADRATURE::nb_points; ++q)
  {
    coord_q[XX] = m_qdpt_phys(q,XX);
    coord_q[YY] = m_qdpt_phys(q,YY);
    du[XX]      = m_u_grad_phys(q,XX);
    du[YY]      = m_u_grad_phys(q,YY);

    for(Uint n=0; n < SHAPEFUNC::nb_nodes; ++n)
    {
      dN[XX]    = m_dNdx(q,n);
      dN[YY]    = m_dNdy(q,n);

      sf_oper_values(q,n) = PHYSICS::Lu(coord_q,m_u_phys[q],dN);
    }

    flux_oper_values[q] = PHYSICS::flux(coord_q,m_u_phys[q],du);
//    std::cout << "X [" << q << "] = " << qd_pt << std::endl;
//    std::cout << "dU[" << q << "] = " << du << std::endl;
//    std::cout << "U [" << q << "] = " << m_qdsol_phys[q] << std::endl;
//    std::cout << "J [" << q << "] = " << m_j[q] << std::endl;

//       std::cout << du << std::endl;

   wj[q] = m_j[q]*m_quadrature.weights[q];

  } //Loop over quadrature points
}


////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_FluxOp2D_hpp
