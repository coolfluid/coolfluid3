// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_Schemes_LDA_hpp
#define cf3_RDM_Schemes_LDA_hpp

#include "math/Checks.hpp"

#include "RDM/CellTerm.hpp"
#include "RDM/SchemeBase.hpp"

#include "RDM/Schemes/LibSchemes.hpp"

namespace cf3 {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_SCHEMES_API LDA : public RDM::CellTerm {

public: // typedefs

  /// the actual scheme implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Term;




public: // functions

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  /// Contructor
  /// @param name of the component
  LDA ( const std::string& name );

  /// Virtual destructor
  virtual ~LDA();

  /// Get the class name
  static std::string type_name () { return "LDA"; }

  /// Execute the loop for all elements
  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////


template < typename SF, typename QD, typename PHYS >
class RDM_SCHEMES_API LDA::Term : public SchemeBase<SF,QD,PHYS> {

public: // typedefs

  /// base class type
  typedef SchemeBase<SF,QD,PHYS> B;

  /// pointers
  
  

public: // functions

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  /// Contructor
  /// @param name of the component
  Term ( const std::string& name ) : SchemeBase<SF,QD,PHYS>(name)
  {
    regist_typeinfo(this);

    for(Uint n = 0; n < SF::nb_nodes; ++n)
      DvPlus[n].setZero();
  }

  /// Get the class name
  static std::string type_name () { return "LDA.Term<" + SF::type_name() + "," + PHYS::type_name() + ">"; }

  /// execute the action
  virtual void execute ();

protected: // data

  /// The operator L in the advection equation Lu = f
  /// Matrix Ki_n stores the value L(N_i) at each quadrature point for each shape function N_i
  typename B::PhysicsMT  Ki_n [SF::nb_nodes];
  /// sum of Lplus to be inverted
  typename B::PhysicsMT  sumLplus;
  /// inverse Ki+ matix
  typename B::PhysicsMT  InvKi_n;
  /// right eigen vector matrix
  typename B::PhysicsMT  Rv;
  /// left eigen vector matrix
  typename B::PhysicsMT  Lv;
  /// diagonal matrix with eigen values
  typename B::PhysicsVT  Dv;
  /// temporary hold of Values of the operator L(u) computed in quadrature points.
  typename B::PhysicsVT  LUwq;
  /// diagonal matrix with positive eigen values
  typename B::PhysicsVT  DvPlus [SF::nb_nodes];

};

/////////////////////////////////////////////////////////////////////////////////////

/// function that flushes the value of a real that is too close to zero
inline Real lolo ( Real  x )
{
  std::cout << "x [" << x << "]"  << std::endl;
  Real r = (x < 1E-12) ? 0.0 : x;
  std::cout << "r [" << r << "]"  << std::endl;
  return r;
}

inline Real myplus ( Real x )
{
  std::cout << "x [" << x << "]"  << std::endl;
  Real r =  std::max( 0. , x );
  std::cout << "r [" << r << "]"  << std::endl;
  return r;
}


template<typename SF,typename QD, typename PHYS>
void LDA::Term<SF,QD,PHYS>::execute()
{
  using namespace cf3::math;

  // get element connectivity

  const mesh::Connectivity::ConstRow nodes_idx = (*B::connectivity)[B::idx()];

  B::interpolate( nodes_idx );

  // L(N)+ @ each quadrature point

  for(Uint q=0; q < QD::nb_points; ++q)
  {
    B::sol_gradients_at_qdpoint(q);

    // compute properties in this quadrature point

    PHYS::compute_properties(B::X_q.row(q),
                             B::U_q.row(q),
                             B::dUdXq,
                             B::phys_props);

    // for every state, project the jacob eigen structure
    // onto the direction given by the gradient, computing the Ki+

    for(Uint n=0; n < SF::nb_nodes; ++n)
    {
      B::dN[XX] = B::dNdX[XX](q,n);
      B::dN[YY] = B::dNdX[YY](q,n);

      PHYS::flux_jacobian_eigen_structure(B::phys_props, B::dN, Rv, Lv, Dv);

      // diagonal matrix of positive eigen values

      DvPlus[n] = Dv.unaryExpr(std::ptr_fun(plus)); /* WORKS */

      Ki_n[n] = Rv * DvPlus[n].asDiagonal() * Lv;
    }

    // compute the PDE residual L(u)

    PHYS::residual( B::phys_props, B::dFdU, B::LU );

//    if(B::idx() == 4185 )
//      std::cout << "LU   [" << q << "] = " << B::LU.transpose() << std::endl;

//    B::LU.unaryExpr(std::ptr_fun(lolo)); /* NOT CALLLED ??? */

//    for(Uint eq = 0; eq < PHYS::MODEL::_neqs; ++eq)
//      B::LU[eq] = ( B::LU[eq] < 1E-13 ) ? 0.0 : B::LU[eq];

//    if(B::idx() == 4185 )
//      std::cout << "LU   [" << q << "] = " << B::LU.transpose() << std::endl;

    // compute L(N)+

    sumLplus = Ki_n[0];
    for(Uint n = 1; n < SF::nb_nodes; ++n)
      sumLplus += Ki_n[n];

    // invert the sum L plus

    InvKi_n = sumLplus.inverse();

    // compute the phi_i LDA intergral

    LUwq = InvKi_n * B::LU * B::wj[q];

//    LUwq.unaryExpr(std::ptr_fun(lolo));  /* NOT CALLLED ??? */

    for(Uint n = 0; n < SF::nb_nodes; ++n)
      B::Phi_n.row(n) +=  Ki_n[n] * LUwq;

#if 0  // cell grad check
    if(B::idx() == 9540 ) // 9541 - 1
    {
      std::cout.setf(std::ios::scientific,std::ios::floatfield);
      std::cout.precision(24);

      const Real n0x = B::X_n(1,YY) - B::X_n(2,YY);
      const Real n0y = B::X_n(2,XX) - B::X_n(1,XX);

      const Real n1x = B::X_n(2,YY) - B::X_n(0,YY);
      const Real n1y = B::X_n(0,XX) - B::X_n(2,XX);

      const Real n2x = B::X_n(0,YY) - B::X_n(1,YY);
      const Real n2y = B::X_n(1,XX) - B::X_n(0,XX);

      const Real S = std::abs( 0.5 * ( n0y * n1x - n1y * n0x ) );

      const Real d0dx = n0x / ( 2 * S );
      const Real d0dy = n0y / ( 2 * S );
      const Real d1dx = n1x / ( 2 * S );
      const Real d1dy = n1y / ( 2 * S );
      const Real d2dx = n2x / ( 2 * S );
      const Real d2dy = n2y / ( 2 * S );

      std::cout << "surface S      [" << S           << "]" << std::endl;
      std::cout << "surface sum_wj [" << B::wj.sum() << "]" << std::endl;

      std::cout << std::endl;

      std::cout << " --- d0dx [" << d0dx << "]\n --- dNdX [" << B::dNdX[XX](q,0) << "]"
                << " --- d0dy [" << d0dy << "]\n --- dNdY [" << B::dNdX[YY](q,0) << "]" << std::endl;

      std::cout << " --- d1dx [" << d1dx << "]\n --- dNdX [" << B::dNdX[XX](q,1) << "]"
                << " --- d1dy [" << d1dy << "]\n --- dNdY [" << B::dNdX[YY](q,1) << "]" << std::endl;

      std::cout << " --- d2dx [" << d2dx << "]\n --- dNdX [" << B::dNdX[XX](q,2) << "]"
                << " --- d2dy [" << d2dy << "]\n --- dNdY [" << B::dNdX[YY](q,2) << "]" << std::endl;

      std::cout << std::endl;

      std::cout << "sum dNdx  [" << d0dx + d1dx + d2dx       << "]" << std::endl;
      std::cout << "sum       [" << B::dNdX[XX].row(q).sum() << "]" << std::endl;

      std::cout << std::endl;

      std::cout << "sum dNdy  [" << d0dy + d1dy + d2dy       << "]" << std::endl;
      std::cout << "sum       [" << B::dNdX[YY].row(q).sum() << "]" << std::endl;

      std::cout << std::endl;

      for(Uint eq = 0; eq < PHYS::MODEL::_neqs; ++eq)
        std::cout << " B::dUdXq("<<eq<<",X) " << B::dUdXq(eq,XX) << " " << B::dUdXq(eq,YY) << std::endl;

      std::cout << "-----------------------------------------------------------------------" << std::endl;
    }

#endif



    // check for non-zero grad
#if 0
    {
      bool zero = false;
      for(Uint eq = 0; eq < PHYS::MODEL::_neqs; ++eq)
        for(Uint dim = 0; dim < PHYS::MODEL::_ndim; ++dim)
        {
//          std::cout << "[" << B::dUdXq(eq,dim) << "] diff [" << FloatingPoint<Real>( B::dUdXq(eq,dim) ).diff( FloatingPoint<Real>(0.) ) << "]" << std::endl;

          if( std::abs( B::dUdXq(eq,dim) ) > 1E-12 )
            zero = true;
        }

      if( zero || B::idx() == 4185 )
      {

        std::cout.setf(std::ios::scientific,std::ios::floatfield);
        std::cout.precision(24);

        std::cout << "---------------------------------------------------------------------------"     << std::endl;

        std::cout << "cell  [" << B::idx() << "]" << std::endl;
//        std::cout << "qd pt [" << q        << "]" << std::endl;

        std::cout << std::endl;

      //  std::cout << "  Operator:" << std::endl;
      //  std::cout << "               Sum of weights = " << m_quadrature.weights.sum() << std::endl;
      //  std::cout << "               Jacobians in physical space:" << std::endl;
      //  std::cout << jacob << std::endl;

//      std::cout << "Area = " << B::wj.sum() << std::endl;

//      std::cout << "X_n  : " << B::X_n << std::endl;

      std::cout << "U_n  :\n" << B::U_n << std::endl;
      std::cout << std::endl;

//          std::cout << "X    [" << q << "] = " << B::X_q.row(q)    << std::endl;
          std::cout << "U    [" << q << "] = " << B::U_q.row(q)     << std::endl;
          std::cout << std::endl;
//          std::cout << "wj   [" << q << "] = " << B::wj[q]             << std::endl;
          std::cout << "dUdX[XX] [" << q << "] = " << B::dUdX[XX].row(q)       << std::endl;
          std::cout << "dUdX[YY] [" << q << "] = " << B::dUdX[YY].row(q)       << std::endl;
          std::cout << std::endl;
          std::cout << "LU   [" << q << "] = " << B::LU.transpose() << std::endl;
          std::cout << std::endl;

          for(Uint dim = 0; dim < PHYS::MODEL::_ndim; ++dim)
            std::cout << "dFdU [" << dim << "] : \n" << B::dFdU[dim] << "\n" << std::endl;
          std::cout << std::endl;

          std::cout << "---------------------------------------------------------------------------"     << std::endl;

      //  std::cout << "nodes_idx";
      //  for ( Uint i = 0; i < nodes_idx.size(); ++i)
      //     std::cout << " " << nodes_idx[i];

      //  std::cout << "mesh::fill function" <<  std::endl;
      //  std::cout << "nodes: " << nodes << std::endl;

      //  std::cout << "solution: " << U_n << std::endl;
      //  std::cout << "phi: " << Phi_n << std::endl;

      //  std::cout << " AREA : " << wj.sum() << std::endl;

      //  std::cout << "phi [";

      //  for (Uint n=0; n < SF::nb_nodes; ++n)
      //    for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
      //      std::cout << Phi_n(n,v) << " ";
      //  std::cout << "]" << std::endl;

      }

      if (zero) exit(0);
    }
#endif

    // compute the phi_i LDA intergral

#ifdef NSCHEME
    // N dissipation
    for(Uint i = 0; i < SF::nb_nodes; ++i)
    {
      LUwq.setZero();
      for(Uint j = 0; j < SF::nb_nodes; ++j)
      {
        if (i==j) continue;
        LUwq += Ki_n[j] * ( U_n.row(i).transpose() - U_n.row(j).transpose() );
//        std::cout << "Uj-Ui : " << U_n.row(j).transpose() - U_n.row(i).transpose() << std::endl;
      }

//      std::cout << "LUwq : " << LUwq << std::endl;

      Phi_n.row(i) +=  Ki_n[i] * InvKi_n * LUwq * wj[q];

//      std::cout << "Phi N = " << Ki_n[i] * InvKi_n * LUwq * wj[q] << std::endl;

    }
#endif


    // compute the wave_speed for scaling the update

    for(Uint n = 0; n < SF::nb_nodes; ++n)
      (*B::wave_speed)[nodes_idx[n]][0] += DvPlus[n].maxCoeff() * B::wj[q];


  } // loop qd points

  // update the residual

  for (Uint n=0; n<SF::nb_nodes; ++n)
    for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
      (*B::residual)[nodes_idx[n]][v] += B::Phi_n(n,v);

}

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // cf3_RDM_Schemes_LDA_hpp
