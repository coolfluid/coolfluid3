// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_WallWeakBc_hpp
#define CF_RDM_WallWeakBc_hpp

#include <iostream> // to remove

#include "RDM/Core/BoundaryTerm.hpp"
#include "RDM/Core/BcBase.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Mesh { class CMesh; class CField; }

namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_CORE_API WallWeakBc : public RDM::BoundaryTerm {
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
  virtual ~WallWeakBc() {};

  /// Get the class name
  static std::string type_name () { return "WallWeakBc"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_mesh();

public: // data

  /// access to the solution field on the mesh
  boost::weak_ptr<Mesh::CField> solution;

}; // !WallWeakBc

//------------------------------------------------------------------------------------------

template < typename SF, typename QD, typename PHYS >
class RDM_CORE_API WallWeakBc::Term : public BcBase<SF,QD,PHYS> {

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
        SF::mapped_gradient( m_quadrature.coords.col(q), GradSF  );
        SF::shape_function ( m_quadrature.coords.col(q), ValueSF );

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
 typedef Eigen::Matrix<Real, QD::nb_points, PHYS::ndim   >     QCoordMT;
 typedef Eigen::Matrix<Real, SF::nb_nodes,  PHYS::neqs   >     SolutionMT;
 typedef Eigen::Matrix<Real, QD::nb_points, PHYS::neqs   >     QSolutionMT;
 typedef Eigen::Matrix<Real, PHYS::neqs,    1u           >     SolutionVT;
 typedef Eigen::Matrix<Real, PHYS::neqs,    PHYS::ndim   >     FluxMT;
 typedef Eigen::Matrix<Real, PHYS::ndim,    1u           >     DimVT;

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
 QSolutionMT dUdX[PHYS::ndim];
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

 /// diagonal matrix with eigen values
 typename B::PhysicsVT Dv;
 /// diagonal matrix with positive eigen values
 typename B::PhysicsVT DvPlus;

 /// temporary normal on 1 quadrature point
 DimVT dN;

 /// reflexion matrix for setting velocity tangent to wall
 Eigen::Matrix<Real, PHYS::neqs, PHYS::neqs >  RM;

public: // functions

 /// virtual interface to execute the action
 virtual void execute () { executeT(); }

 /// execute the action
 void executeT ()
 {
   // std::cout << "Face [" << B::idx() << "]" << std::endl;

   // get face connectivity

   const Mesh::CTable<Uint>::ConstRow nodes_idx = this->connectivity_table->array()[B::idx()];

   // copy the coordinates from the large array to a small

   Mesh::fill(X_n, *B::coordinates, nodes_idx );

   // copy the solution from the large array to a small

   for(Uint n = 0; n < SF::nb_nodes; ++n)
     for (Uint v=0; v < PHYS::neqs; ++v)
       U_n(n,v) = (*B::solution)[ nodes_idx[n] ][v];

   // coordinates of quadrature points in physical space

   X_q  = Ni * X_n;

   // derivatives of coordinate transf.

   dX_q = dNdKSI * X_n;

   // solution at all quadrature points in physical space

   U_q = Ni * U_n;

   // zero element residuals

   Phi_n.setZero();

   // loop on quadrature points

   for(Uint q=0; q < QD::nb_points; ++q)
   {
//    std::cout << "Gradient of face shape functions: " << std::endl;
//    std::cout << dNdKSI << std::endl;
//    std::cout << "nr. of rows = " << dNdKSI.rows() << std::endl;
//    std::cout << "nr. of cols = " << dNdKSI.cols() << std::endl;

    const Real jacob = std::sqrt( dX_q(q,XX)*dX_q(q,XX)+dX_q(q,YY)*dX_q(q,YY) );

    wj[q] = jacob * m_quadrature.weights[q];

    // compute the normal at quadrature point

    dN[XX] = -dX_q(q,YY)/jacob;
    dN[YY] =  dX_q(q,XX)/jacob;

//    std::cout << "n (generic) [" << nx << "," << ny << "]" << std::endl;

    // compute the flux F(u_h) and its correction F(u_g)

    PHYS::compute_properties(X_q.row(q),
                             U_q.row(q),
                             dUdX[XX].row(q).transpose(),
                             dUdX[YY].row(q).transpose(),
                             B::phys_props);

    PHYS::flux(B::phys_props,
               X_q.row(q),
               U_q.row(q),
               Fu_h);

    // std::cout << "Fu_h [" << Fu_h_x << "," << Fu_h_y << "]" << std::endl;

    // compute the reflection matrix

    RM(1,1) = 1. - 2*dN[XX]*dN[XX];
    RM(1,2) =    - 2*dN[XX]*dN[YY];
    RM(2,1) =    - 2*dN[YY]*dN[XX];
    RM(2,2) = 1. - 2*dN[YY]*dN[YY];

    // reflect the current solution on the wall to get the ghost solution

    u_g = RM * U_q.row(q).transpose();

    // compute the flux according to the ghost solution u_g

    PHYS::flux(B::phys_props,
               X_q.row(q),
               u_g,
               Fu_g);

    // std::cout << "Fu_g [" << Fu_g_x << "," << Fu_g_y << "]" << std::endl;

    for(Uint n=0; n < SF::nb_nodes; ++n)
      for(Uint v=0; v < PHYS::neqs; ++v)
      {
        Phi_n.row(n)[v] -= ( ( Fu_g(v,XX) - Fu_h(v,XX) ) * dN[XX] +
                             ( Fu_g(v,YY) - Fu_h(v,YY) ) * dN[YY] )
                           *   Ni(q,n) * wj[q];
      }


    // compute the wave_speed for scaling the update

    PHYS::jacobian_eigen_values(B::phys_props,
                                X_q.row(q),
                                U_q.row(q),
                                dN,
                                Dv );

    DvPlus = Dv.unaryExpr(std::ptr_fun(plus));

    for(Uint n = 0; n < SF::nb_nodes; ++n)
      (*B::wave_speed)[nodes_idx[n]][0] += DvPlus.array().maxCoeff() * wj[q];


   } //Loop over quadrature points

   // update the residual

   for (Uint n=0; n<SF::nb_nodes; ++n)
     for (Uint v=0; v < PHYS::neqs; ++v)
       (*B::residual)[nodes_idx[n]][v] += Phi_n(n,v);

 }

}; // !WallWeakBc::Term

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_WallWeakBc_hpp
