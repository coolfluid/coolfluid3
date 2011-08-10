// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_WallEdwinBc_hpp
#define CF_RDM_WallEdwinBc_hpp

#include "RDM/BoundaryTerm.hpp"
#include "RDM/BcBase.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Mesh { class CMesh; class Field; }

namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_API WallEdwinBc : public RDM::BoundaryTerm {
public: // typedefs

  /// the actual BC implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Term;

  /// pointers
  typedef boost::shared_ptr<WallEdwinBc> Ptr;
  typedef boost::shared_ptr<WallEdwinBc const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  WallEdwinBc ( const std::string& name );

  /// Virtual destructor
  virtual ~WallEdwinBc() {}

  /// Get the class name
  static std::string type_name () { return "WallEdwinBc"; }

  /// execute the action
  virtual void execute ();

  virtual bool is_weak() const { return true; }

private: // helper functions

public: // data

  /// access to the solution field on the mesh
  boost::weak_ptr<Mesh::Field> solution;

}; // !WallEdwinBc

//------------------------------------------------------------------------------------------

template < typename SF, typename QD, typename PHYS >
class RDM_API WallEdwinBc::Term : public BcBase<SF,QD,PHYS> {

public: // typedefs

  /// base class type
  typedef BcBase<SF,QD,PHYS> B;
  /// pointers
  typedef boost::shared_ptr< Term > Ptr;
  typedef boost::shared_ptr< Term const> ConstPtr;

public: // functions

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

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
 static std::string type_name () { return "WallEdwinBc.Term<" + SF::type_name() + "," + PHYS::type_name() + ">"; }

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

 /// diagonal matrix with eigen values
 typename B::PhysicsVT Dv;
 /// diagonal matrix with positive eigen values
 typename B::PhysicsVT DvPlus;

 /// temporary normal on 1 quadrature point
 DimVT dN;

 /// corrective flux
 SolutionMT F_c;

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

   const Mesh::CConnectivity::ConstRow nodes_idx = (*connectivity)[B::idx()];

   // copy the coordinates from the large array to a small

   Mesh::fill(X_n, *B::coordinates, nodes_idx );

   // copy the solution from the large array to a small

   for(Uint n = 0; n < SF::nb_nodes; ++n)
     for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
       U_n(n,v) = (*B::solution)[ nodes_idx[n] ][v];


   const Real dx = X_n(1,XX) - X_n(0,XX);
   const Real dy = X_n(1,YY) - X_n(0,YY);

   const Real jacob = std::sqrt( dx*dx + dy*dy );

   dN[XX] =  dy/jacob;
   dN[YY] = -dx/jacob;

   Phi_n.setZero();
   F_c.setZero();

   const Real alpha = 0.75;
   const Real coeff_state =        alpha /SF::nb_nodes;
   const Real coeff_other = (1.0 - alpha)/SF::nb_nodes;

   /// For the moment, the corrective flux is equal to the solution vector at each node
//   F_c = U_n;

   for(Uint ni = 0; ni < SF::nb_nodes; ++ni)
   {
     for(Uint dim = 0; dim < PHYS::MODEL::_ndim; ++dim)
     {
       dUdXq.col(dim) = dUdX[dim].row(0).transpose();
     }


     PHYS::compute_properties(X_n.row(ni),
                              U_n.row(ni),
                              dUdXq,
                              B::phys_props);

     const Real vn = dN[XX] * B::phys_props.u + dN[YY] * B::phys_props.v;

     F_c(ni,0) = B::phys_props.rho  * vn;
     F_c(ni,1) = B::phys_props.rhou * vn;
     F_c(ni,2) = B::phys_props.rhov * vn;
     F_c(ni,3) = B::phys_props.H    * vn;

//     F_c(ni,3) += B::phys_props.P;
//     F_c.row(ni) *= vn;

     for(Uint nj=0; nj < SF::nb_nodes; ++nj)
     {
       if( ni == nj)
       {
         for(Uint v=0; v < PHYS::MODEL::_neqs; ++v)
         {
           Phi_n.row(nj)[v] -= coeff_state * F_c(nj,v);
         }
       }
       else
       {
         for(Uint v=0; v < PHYS::MODEL::_neqs; ++v)
         {
           Phi_n.row(nj)[v] -= coeff_other * F_c(nj,v);
         }
       }

     } // Loop over nodes (nj)


     // compute the wave_speed for scaling the update

//     PHYS::flux_jacobian_eigen_values(B::phys_props,
//                                 dN,
//                                 Dv );

//     DvPlus = Dv.unaryExpr(std::ptr_fun(plus));

//     for(Uint n = 0; n < SF::nb_nodes; ++n)
//       (*B::wave_speed)[nodes_idx[n]][0] += DvPlus.array().maxCoeff() * wj[q];


    } // Loop over nodes (ni)


   // update the residual

   for (Uint n=0; n<SF::nb_nodes; ++n)
     for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
       (*B::residual)[nodes_idx[n]][v] += Phi_n(n,v);

 }


}; // !WallEdwinBc::Term

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_WallEdwinBc_hpp
