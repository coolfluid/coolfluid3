// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CSysFluxOp2D_hpp
#define CF_Solver_CSysFluxOp2D_hpp

#include <iostream>

#include <Eigen/Dense>

#include "Math/MatrixTypes.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
class RDM_API CSysFluxOp2D
{
public: // typedefs

  /// output matrix types
  typedef Eigen::Matrix<Real, QUADRATURE::nb_points, PHYSICS::nb_eqs>     ResidualMT;
  typedef Eigen::Matrix<Real, SHAPEFUNC::nb_nodes,   PHYSICS::nb_eqs>     SolutionMT;
  typedef Eigen::Matrix<Real, PHYSICS::nb_eqs, PHYSICS::nb_eqs>           PhysicsMT;
  typedef Eigen::Matrix<Real, PHYSICS::nb_eqs, 1u>                        PhysicsVT;
  typedef Eigen::Matrix<Real, QUADRATURE::nb_points, SHAPEFUNC::nb_nodes> SFMatrixT;

  typedef Eigen::Matrix<Real, 1u, PHYSICS::nb_eqs >                       SolutionVT;
  typedef Eigen::Matrix<Real, 1u, SHAPEFUNC::nb_nodes >                   SFVectorT;

  typedef Eigen::Matrix<Real, PHYSICS::nb_eqs, PHYSICS::nb_eqs >          EigenValueMT;

  typedef typename SHAPEFUNC::NodeMatrixT                                 NodeMT;


public: // functions
  /// Contructor
  /// @param name of the component
  CSysFluxOp2D ( );

  /// Virtual destructor
  virtual ~CSysFluxOp2D() {};

  /// Get the class name
  static std::string type_name () { return "CSysFluxOp2D<" + SHAPEFUNC::type_name() + "," + QUADRATURE::type_name() +  ">"; }

  /// Compute the operator values at all quadrature nodes
  void compute(const NodeMT& nodes,
               const SolutionMT& solution,
               PhysicsMT    Kiq[],
               PhysicsVT    LU[],
               PhysicsMT    DvPlus[],
               Eigen::Matrix<Real, QUADRATURE::nb_points, 1u>& wj);
    
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

  Eigen::Matrix<Real,QUADRATURE::nb_points, PHYSICS::nb_eqs> dUdx; //derivatives of solution x
  Eigen::Matrix<Real,QUADRATURE::nb_points, PHYSICS::nb_eqs> dUdy; //derivatives of solution y

  Eigen::Matrix<Real,QUADRATURE::nb_points, DIM_2D> m_dx; // stores dx/dksi and dx/deta at each quadrature point
  Eigen::Matrix<Real,QUADRATURE::nb_points, DIM_2D> m_dy; // stores dy/dksi and dy/deta at each quadrature point

  Eigen::Matrix<Real,QUADRATURE::nb_points, 1u> m_j;      // jacobian of transformation phys->ref at each qd. pt

  Eigen::Matrix<Real,QUADRATURE::nb_points, SHAPEFUNC::nb_nodes> m_dNdx; //Derivatives of shape functions
                                                                        //at all quadrature points in phys. space
  Eigen::Matrix<Real,QUADRATURE::nb_points, SHAPEFUNC::nb_nodes> m_dNdy;

  /// flux jacobians
  PhysicsMT dFdU[DIM_2D];
  /// right eigen vector matrix
  PhysicsMT Rv;
  /// left eigen vector matrix
  PhysicsMT Lv;
  /// diagonal matrix with eigen values
  PhysicsMT Dv;
  /// gradient of shape function
  RealVector2 dN;

};


template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
CSysFluxOp2D<SHAPEFUNC,QUADRATURE,PHYSICS>::CSysFluxOp2D() : m_quadrature( QUADRATURE::instance() )
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

   Dv.setZero();
}



template < typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
void CSysFluxOp2D<SHAPEFUNC,QUADRATURE,PHYSICS>::compute(const NodeMT& nodes,
                                                         const SolutionMT& solution,
                                                         PhysicsMT   Kiq[],
                                                         PhysicsVT   LU[],
                                                         PhysicsMT   DvPlus[],
                                                         Eigen::Matrix<Real, QUADRATURE::nb_points, 1u>& wj)
{
   //Coordinates of quadrature points in physical space
   m_qdpos  = m_N * nodes;
   //Solution atall quadrature points in physical space
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

   dUdx = m_dNdx * solution;
   dUdy = m_dNdy * solution;

  for(Uint q=0; q < QUADRATURE::nb_points; ++q)
  {
    for(Uint n=0; n < SHAPEFUNC::nb_nodes; ++n)
    {
      dN[XX] = m_dNdx(q,n);
      dN[YY] = m_dNdy(q,n);

      PHYSICS::jacobian_eigen_structure(m_qdpos.row(q),
                                        m_u_qd.row(q),
                                        dN,
                                        Rv,
                                        Lv,
                                        Dv,
                                        DvPlus [ q * QUADRATURE::nb_points + n ],
                                        Kiq    [ q * QUADRATURE::nb_points + n ] );
    }

    PHYSICS::Lu(m_qdpos.row(q),
                m_u_qd.row(q),
                dUdx.row(q).transpose(),
                dUdy.row(q).transpose(),
                dFdU,
                LU[q]);

    wj[q] = m_j[q]*m_quadrature.weights[q];

//    std::cout << "X    [" << q << "] = " << m_qdpos.row(q)    << std::endl;
//    std::cout << "U    [" << q << "] = " << m_u_qd.row(q)     << std::endl;
//    std::cout << "dUdx [" << q << "] = " << dUdx.row(q)       << std::endl;
//    std::cout << "dUdy [" << q << "] = " << dUdy.row(q)       << std::endl;
//    std::cout << "LU   [" << q << "] = " << LU[q].transpose() << std::endl;
//    std::cout << "wj   [" << q << "] = " << wj[q]             << std::endl;
//    std::cout << "--------------------------------------"     << std::endl;

  } // loop over quadrature points

}


////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CSysFluxOp2D_hpp
