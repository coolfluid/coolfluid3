// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_SysFluxOp2D_hpp
#define CF_RDM_SysFluxOp2D_hpp

#include <iostream>

#include <Eigen/Dense>

#include "Math/MatrixTypes.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
class RDM_API SysFluxOp2D
{
public: // typedefs

  /// output matrix types
  typedef Eigen::Matrix<Real, QUADRATURE::nb_points, PHYSICS::nb_eqs>     ResidualMatrixT;
  typedef Eigen::Matrix<Real, SHAPEFUNC::nb_nodes,   PHYSICS::nb_eqs>     SolutionMatrixT;
  typedef Eigen::Matrix<Real, QUADRATURE::nb_points, SHAPEFUNC::nb_nodes> SFMatrixT;

  typedef Eigen::Matrix<Real, 1u, PHYSICS::nb_eqs >                       SolutionVectorT;
  typedef Eigen::Matrix<Real, 1u, SHAPEFUNC::nb_nodes >                   SFVectorT;


public: // functions
  /// Contructor
  /// @param name of the component
  SysFluxOp2D ( );

  /// Virtual destructor
  virtual ~SysFluxOp2D() {};

  /// Get the class name
  static std::string type_name () { return "SysFluxOp2D<" + SHAPEFUNC::type_name() + "," + QUADRATURE::type_name() +  ">"; }

  /// Compute the operator values at all quadrature nodes
  void compute(const typename SHAPEFUNC::NodeMatrixT& nodes,
               const SolutionMatrixT& solution,
               SolutionMatrixT   sf_qd[],
               ResidualMatrixT&  Lu_qd,
               Eigen::Matrix<Real, QUADRATURE::nb_points, 1u> & wj);
    
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

  Eigen::Matrix<Real,QUADRATURE::nb_points, DIM_2D> m_qdpos; //coordinates of quadrature points in physical space
  Eigen::Matrix<Real,QUADRATURE::nb_points, PHYSICS::nb_eqs > m_u_qd; //solution at quadrature points in physical space

  Eigen::Matrix<Real,QUADRATURE::nb_points, PHYSICS::nb_eqs> m_gradu_x; //derivatives of solution x
  Eigen::Matrix<Real,QUADRATURE::nb_points, PHYSICS::nb_eqs> m_gradu_y; //derivatives of solution y

  Eigen::Matrix<Real,QUADRATURE::nb_points, DIM_2D> m_dx; // stores dx/dksi and dx/deta at each quadrature point
  Eigen::Matrix<Real,QUADRATURE::nb_points, DIM_2D> m_dy; // stores dy/dksi and dy/deta at each quadrature point

  Eigen::Matrix<Real,QUADRATURE::nb_points, 1u> m_j;      // jacobian of transformation phys->ref at each qd. pt

  Eigen::Matrix<Real,QUADRATURE::nb_points, SHAPEFUNC::nb_nodes> m_dNdx; //Derivatives of shape functions
                                                                        //at all quadrature points in phys. space
  Eigen::Matrix<Real,QUADRATURE::nb_points, SHAPEFUNC::nb_nodes> m_dNdy;

};


template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
SysFluxOp2D<SHAPEFUNC,QUADRATURE,PHYSICS>::SysFluxOp2D() : m_quadrature( QUADRATURE::instance() )
{
   ///Set up the matrices to be used for interpolation
   for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
   {
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



template < typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
void SysFluxOp2D<SHAPEFUNC,QUADRATURE,PHYSICS>::compute(const typename SHAPEFUNC::NodeMatrixT& nodes,
                                                     const SolutionMatrixT& solution,
                                                     SolutionMatrixT   sf_qd[],
                                                     ResidualMatrixT&  Lu_qd,
                                                     Eigen::Matrix<Real, QUADRATURE::nb_points, 1u> & wj)
{
   //Coordinates of quadrature points in physical space
   m_qdpos  = m_N * nodes;
   //Solution at all quadrature points in physical space
   m_u_qd = m_N * solution;

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

   // Shape function derivatives in physical space at quadrature pts
   for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
     for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
     {
       m_dNdx(q,n) = 1.0/m_j[q] * (  m_dNdksi(q,n)*m_dy(q,YY) - m_dNdeta(q,n) * m_dy(q,XX));
       m_dNdy(q,n) = 1.0/m_j[q] * ( -m_dNdksi(q,n)*m_dx(q,YY) + m_dNdeta(q,n) * m_dx(q,XX));
     }

   m_gradu_x = m_dNdx * solution;
   m_gradu_y = m_dNdy * solution;

  RealVector2      dN;
  SolutionVectorT  Lqn;
  SolutionVectorT  Fq;

  for(Uint q=0; q < QUADRATURE::nb_points; ++q)
  {
    for(Uint n=0; n < SHAPEFUNC::nb_nodes; ++n)
    {
      dN[XX] = m_dNdx(q,n);
      dN[YY] = m_dNdy(q,n);

      PHYSICS::Lu(m_qdpos.row(q),
                  m_u_qd.row(q),
                  dN,
                  Lqn );

      sf_qd[q].row(n) = Lqn;
    }

    PHYSICS::flux(m_qdpos.row(q),
                  m_u_qd.row(q),
                  m_gradu_x.row(q),
                  m_gradu_y.row(q),
                  Fq);

    Lu_qd.row(q) = Fq;

//    std::cout << "X [" << q << "] = " << qd_pt << std::endl;
//    std::cout << "dU[" << q << "] = " << du << std::endl;
//    std::cout << "U [" << q << "] = " << m_qdsol_phys[q] << std::endl;
//    std::cout << "J [" << q << "] = " << m_j[q] << std::endl;

//       std::cout << du << std::endl;

   wj[q] = m_j[q]*m_quadrature.weights[q];

  } // loop over quadrature points
}


////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_SysFluxOp2D_hpp
