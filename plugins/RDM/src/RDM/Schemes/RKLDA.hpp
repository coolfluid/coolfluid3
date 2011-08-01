// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_Schemes_RKLDA_hpp
#define CF_RDM_Schemes_RKLDA_hpp

#include "Common/StringConversion.hpp"

#include "Mesh/CField.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/IterativeSolver.hpp"
#include "RDM/TimeStepping.hpp"

#include "RDM/CellTerm.hpp"
#include "RDM/SchemeBase.hpp"

#include "RDM/Schemes/LibSchemes.hpp"

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_SCHEMES_API RKLDA : public RDM::CellTerm {

public: // typedefs

  /// the actual scheme implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Term;

  typedef boost::shared_ptr< RKLDA > Ptr;
  typedef boost::shared_ptr< RKLDA const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  RKLDA ( const std::string& name );

  /// Virtual destructor
  virtual ~RKLDA();

  /// Get the class name
  static std::string type_name () { return "RKLDA"; }

  /// Execute the loop for all elements
  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////


template < typename SF, typename QD, typename PHYS >
class RDM_SCHEMES_API RKLDA::Term : public SchemeBase<SF,QD,PHYS> {

public: // typedefs

  /// base class type
  typedef SchemeBase<SF,QD,PHYS> B;

  /// pointers
  typedef boost::shared_ptr< Term > Ptr;
  typedef boost::shared_ptr< Term const> ConstPtr;

public: // functions

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  /// Contructor
  /// @param name of the component
  Term ( const std::string& name ) : SchemeBase<SF,QD,PHYS>(name)
  {
    regist_typeinfo(this);

    for(Uint n = 0; n < SF::nb_nodes; ++n)
      DvPlus[n].setZero();

    m_options["Elements"].attach_trigger ( boost::bind ( &RKLDA::Term<SF,QD,PHYS>::config_coeffs, this ) );

  }

  /// Get the class name
  static std::string type_name () { return "RKLDA.Term<" + SF::type_name() + "," + PHYS::type_name() + ">"; }

  /// execute the action
  virtual void execute ();

protected: // helper function

  void config_coeffs()
  {
    using namespace Common;

    RDSolver& mysolver = this->parent().as_type<CellTerm>().solver().as_type<RDSolver>();
    rkorder = mysolver.properties().template value<Uint>("rkorder");
    step    = mysolver.iterative_solver().properties().template value<Uint>("iteration");
    dt      = mysolver.time_stepping().get_child("Time").option("time_step").template value<Real>();

    ksolutions.clear();
    ksolutions.push_back( mysolver.fields().get_child( Tags::solution() ).follow()->as_ptr_checked<Mesh::CField>() );
    for ( Uint k = 1; k <= rkorder; ++k)
    {
      ksolutions.push_back( mysolver.fields().get_child( Tags::solution() + to_str(k) ).follow()->as_ptr_checked<Mesh::CField>() );
    }

    std::cout << "RKLDA   rkorder : " << rkorder << std::endl;
    std::cout << "RKLDA   step    : " << step    << std::endl;

    rkalphas.resize(rkorder,rkorder);
    rkbetas.resize(rkorder,rkorder);
    if ( rkorder == 1)
        rkcoeff.resize(1);
    else
        rkcoeff.resize(rkorder-1);

    switch( rkorder )
    {
    case 1:

      rkalphas(0,0) = 1.;

      rkcoeff[0] =  1.;

      rkbetas(0,0) =  0.;

      break;

    case 2:

      rkalphas(0,0) =  0.  ;
      rkalphas(1,0) =  0.  ;
      rkalphas(0,1) =  0.5 ;
      rkalphas(1,1) =  0.5 ;

      rkcoeff[0] =   1.0;

      rkbetas (0,0) =  0.0;
      rkbetas (0,1) = -1.0;
      rkbetas (1,0) =  0.0;
      rkbetas (1,1) =  1.0;

      break;

    case 3:

      rkalphas(0,0) = 0.    ;
      rkalphas(1,0) = 0.    ;
      rkalphas(2,0) = 0.    ;
      rkalphas(0,1) = 0.25  ;
      rkalphas(1,1) = 0.25  ;
      rkalphas(2,1) = 0.    ;
      rkalphas(0,2) = 1./6. ;
      rkalphas(1,2) = 1./6. ;
      rkalphas(2,2) = 2./3. ;

      rkcoeff[0] =    0.5 ;
      rkcoeff[1] =    2.0 ;

      rkbetas(0,0) = 0.  ;
      rkbetas(0,1) = -0.5 ;
      rkbetas(0,2) = -2. ;
      rkbetas(1,0) = 0. ;
      rkbetas(1,1) = 0. ;
      rkbetas(1,2) = 0. ;
      rkbetas(2,0) = 0. ;
      rkbetas(2,1) = 0.5 ;
      rkbetas(2,2) = 2. ;

      break ;

    }

    std::cout << "rkalphas  :\n" << rkalphas << std::endl;
    std::cout << "rkbetas   :\n" << rkbetas  << std::endl;
    std::cout << "rkcoeff   :\n" << rkcoeff  << std::endl;

  }

protected: // data

  Uint rkorder; ///< order of the RK method
  Uint step;    ///< current RK step
  Real dt;      ///< time step size

  RealMatrix rkalphas;  ///< matrix with alpha coefficients of RK method
  RealMatrix rkbetas;   ///< matrix with beta  coefficients of RK method
  RealVector rkcoeff;   ///< matrix with the coefficients that multiply du_s

  std::vector< Mesh::CField::Ptr > ksolutions;  ///< solution fields at different k steps

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
  /// du_s
  typename B::SolutionMT du;

};

/////////////////////////////////////////////////////////////////////////////////////

template<typename SF,typename QD, typename PHYS>
void RKLDA::Term<SF,QD,PHYS>::execute()
{

  std::cout << "cell [" << B::idx() << "]" << std::endl;

  // get element connectivity

  const Mesh::CTable<Uint>::ConstRow nodes_idx = this->connectivity_table->array()[B::idx()];

  //compute delta u_s

  std::cout << FromHere().short_str() << std::endl;

  du.setZero();

  for ( Uint r = 0 ; r < rkorder ; ++r)
  {
    std::cout << "field size : " << ksolutions[r]->data().size()
              << " x " << ksolutions[r]->data().row_size()
              << std::endl;

    for ( Uint n = 0 ; n < SF::nb_nodes ; ++n)
    {
      for ( Uint eq = 0 ; eq < PHYS::MODEL::_neqs; ++eq)
      {
        std::cout << "accessing [" << n << "] [" << eq << "]" << std::endl;

        du(nodes_idx[n],eq) += rkbetas(r,step-1) * ksolutions[r]->data()[ nodes_idx[n] ][eq];
      }
    }
  }

  std::cout << FromHere().short_str() << std::endl;

  {
    // add mass matrix contribution
    for (Uint n=0; n<SF::nb_nodes; ++n)
    {
      for ( Uint s=0; s< SF::nb_nodes; ++s)
      {
        for ( Uint eq=0 ; eq<PHYS::MODEL::_neqs ; ++eq)
        {
          if (step == 1 && rkorder == 1)
          {
            if ( n==s )
            {
              (*B::residual)[nodes_idx[n]][eq] = 1. / 12. * 2. * rkcoeff[0] * du(s,eq);
            }
            else
            {
              (*B::residual)[nodes_idx[n]][eq] = 1. / 12. * 1. * rkcoeff[0] * du(s,eq);
            }
          }
          else
          {
            if ( n==s )
            {
              (*B::residual)[nodes_idx[n]][eq] = 1. / 12. * 2. * rkcoeff[step - 2] * du(s,eq);
            }
            else
            {
              (*B::residual)[nodes_idx[n]][eq] = 1. / 12. * 1. * rkcoeff[step - 2] * du(s,eq);
            }
          }
        }
      }
    }
  }

  std::cout << FromHere().short_str() << std::endl;

  // there isn't a way to compute it automatically, yet
  // it's necessary to define s
  /// @todo lumped and not lumped in the same line

  for ( Uint r = 0 ; r < step ; ++r)
  {
    ///////     ////////////////////////////////////////////////////////       ///////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @todo must be tested for 3D

    // copy the coordinates from the large array to a small

    Mesh::fill(B::X_n, *B::coordinates, nodes_idx );

    // copy the solution from the large array to a small

    for(Uint n = 0; n < SF::nb_nodes; ++n)
      for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
      {
        B::U_n(n,v) = ksolutions[r]->data()[ nodes_idx[n] ][v];
      }

    // coordinates of quadrature points in physical space

    B::X_q  = B::Ni * B::X_n;

    // solution at all quadrature points in physical space

    B::U_q = B::Ni * B::U_n;

    // Jacobian of transformation phys -> ref:
    //    |   dx/dksi    dx/deta    |
    //    |   dy/dksi    dy/deta    |

    // dX[XX].col(KSI) has the values of dx/dksi at all quadrature points
    // dX[XX].col(ETA) has the values of dx/deta at all quadrature points

    for(Uint dimx = 0; dimx < PHYS::MODEL::_ndim; ++dimx)
    {
      for(Uint dimksi = 0; dimksi < PHYS::MODEL::_ndim; ++dimksi)
      {
        B::dX[dimx].col(dimksi) = B::dNdKSI[dimksi] * B::X_n.col(dimx);
      }
    }

    // Fill Jacobi matrix (matrix of transformation phys. space -> ref. space) at qd. point q
    for(Uint q = 0; q < QD::nb_points; ++q)
    {
      for(Uint dimx = 0; dimx < PHYS::MODEL::_ndim; ++dimx)
      {
        for(Uint dimksi = 0; dimksi < PHYS::MODEL::_ndim; ++dimksi)
        {
          B::JM(dimksi,dimx) = B::dX[dimx](q,dimksi);
        }
      }

      // Once the jacobi matrix at one quadrature point is assembled, let's re-use it
      // to compute the gradients of of all shape functions in phys. space
      B::jacob[q] = B::JM.determinant();
      B::JMinv = B::JM.inverse();

      for(Uint n = 0; n < SF::nb_nodes; ++n)
      {
        for(Uint dimksi = 0; dimksi < PHYS::MODEL::_ndim; ++ dimksi)
        {
          B::dNref[dimksi] = B::dNdKSI[dimksi](q,n);
        }

        B::dNphys = B::JMinv * B::dNref;

        for(Uint dimx = 0; dimx < PHYS::MODEL::_ndim; ++ dimx)
        {
          B::dNdX[dimx](q,n) = B::dNphys[dimx];
        }

      }

    } // loop over quadrature points

    // compute transformed integration weights (sum is element area)

    for(Uint q = 0; q < QD::nb_points; ++q)
      B::wj[q] = B::jacob[q] * B::m_quadrature.weights[q];

    // solution derivatives in physical space at quadrature point

    B::dUdX[XX] = B::dNdX[XX] * B::U_n;
    B::dUdX[YY] = B::dNdX[YY] * B::U_n;

    // zero element residuals

    B::Phi_n.setZero();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // L(N)+ @ each quadrature point

    for(Uint q = 0 ; q < QD::nb_points ; ++q) // loop over each quadrature point
    {
      // compute the contributions to phi_i
      B::sol_gradients_at_qdpoint(q);

      PHYS::compute_properties(B::X_q.row(q),
                               B::U_q.row(q),
                               B::dUdXq,
                               B::phys_props);

      for( Uint n = 0 ; n < SF::nb_nodes ; ++n)
      {
        B::dN[XX] = B::dNdX[XX](q,n);
        B::dN[YY] = B::dNdX[YY](q,n);

        PHYS::flux_jacobian_eigen_structure(B::phys_props,
                                            B::dN,
                                            Rv,
                                            Lv,
                                            Dv );

        // diagonal matrix of positive eigen values

        DvPlus[n] = Dv.unaryExpr(std::ptr_fun(plus));

        Ki_n[n] = Rv * DvPlus[n].asDiagonal() * Lv;
      }

      // compute L(u)

      PHYS::residual(B::phys_props,
                     B::dFdU,
                     B::LU );

      // compute L(N)+

      sumLplus = Ki_n[0];
      for(Uint n = 1 ; n < SF::nb_nodes ; ++n)
        sumLplus += Ki_n[n];

      // invert the sum L plus

      InvKi_n = sumLplus.inverse();

      // compute the phi_i LDA integral

      LUwq = InvKi_n * B::LU * B::wj[q];

      for(Uint n = 0 ; n < SF::nb_nodes ; ++n)
        B::Phi_n.row(n) +=  Ki_n[n] * LUwq;
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // save the phi_i
      // save B::Phi_n.row
      //Phin.row() += Phi_n.row(n) ;
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // compute contribution to wave_speed

      for(Uint n = 0 ; n < SF::nb_nodes ; ++n)
        (*B::wave_speed)[nodes_idx[n]][0] += DvPlus[n].maxCoeff() * B::wj[q];
    } // loop over each quadrature point
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // update the residual (according to k)
    if ( step > 1)
    {
      for (Uint n=0; n<SF::nb_nodes; ++n)
      {
        for (Uint eq=0; eq < PHYS::MODEL::_neqs; ++eq)
          (*B::residual)[nodes_idx[n]][eq] += rkalphas(r,step - 1) * dt * B::Phi_n(n,eq);
      }
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  }
}
//#endif

#if 0
    // get element connectivity

  const Mesh::CTable<Uint>::ConstRow nodes_idx = this->connectivity_table->array()[B::idx()];

  B::interpolate( nodes_idx );

  // L(N)+ @ each quadrature point

  for(Uint q=0; q < QD::nb_points; ++q)
  {

    B::sol_gradients_at_qdpoint(q);

    PHYS::compute_properties(B::X_q.row(q),
                             B::U_q.row(q),
                             B::dUdXq,
                             B::phys_props);

    for(Uint n=0; n < SF::nb_nodes; ++n)
    {
      B::dN[XX] = B::dNdX[XX](q,n);
      B::dN[YY] = B::dNdX[YY](q,n);

      PHYS::flux_jacobian_eigen_structure(B::phys_props,
                                     B::dN,
                                     Rv,
                                     Lv,
                                     Dv );

      // diagonal matrix of positive eigen values

      DvPlus[n] = Dv.unaryExpr(std::ptr_fun(plus));

      Ki_n[n] = Rv * DvPlus[n].asDiagonal() * Lv;
    }

    // compute L(u)

    PHYS::residual(B::phys_props,
             B::dFdU,
             B::LU );

    // compute L(N)+

    sumLplus = Ki_n[0];
    for(Uint n = 1; n < SF::nb_nodes; ++n)
      sumLplus += Ki_n[n];

    // invert the sum L plus

    InvKi_n = sumLplus.inverse();

    // compute the phi_i LDA intergral

    LUwq = InvKi_n * B::LU * B::wj[q];

    for(Uint n = 0; n < SF::nb_nodes; ++n)
      B::Phi_n.row(n) +=  Ki_n[n] * LUwq;

    // compute the wave_speed for scaling the update

    for(Uint n = 0; n < SF::nb_nodes; ++n)
      (*B::wave_speed)[nodes_idx[n]][0] += DvPlus[n].maxCoeff() * B::wj[q];


  } // loop qd points

  // update the residual

  for (Uint n=0; n<SF::nb_nodes; ++n)
    for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
      (*B::residual)[nodes_idx[n]][v] += B::Phi_n(n,v);

#endif
  // debug

  //  std::cout << "LDA ELEM [" << idx() << "]" << std::endl;
  //  std::cout << "  Operator:" << std::endl;
  //  std::cout << "               Sum of weights = " << m_quadrature.weights.sum() << std::endl;
  //  std::cout << "               Jacobians in physical space:" << std::endl;
  //  std::cout << jacob << std::endl;
  //  std::cout << "LDA: Area = " << wj.sum() << std::endl;


  //    std::cout << "X    [" << q << "] = " << B::X_q.row(q)    << std::endl;
  //    std::cout << "U    [" << q << "] = " << B::U_q.row(q)     << std::endl;
  //    std::cout << "dUdX[XX] [" << q << "] = " << dUdX[XX].row(q)       << std::endl;
  //    std::cout << "dUdX[YY] [" << q << "] = " << dUdX[YY].row(q)       << std::endl;
  //    std::cout << "LU   [" << q << "] = " << Lu.transpose() << std::endl;
  //    std::cout << "wj   [" << q << "] = " << wj[q]             << std::endl;
  //    std::cout << "--------------------------------------"     << std::endl;

  //  std::cout << "nodes_idx";
  //  for ( Uint i = 0; i < nodes_idx.size(); ++i)
  //     std::cout << " " << nodes_idx[i];

  //  std::cout << "mesh::fill function" <<  std::endl;
  //  std::cout << "nodes: " << nodes << std::endl;

  //  std::cout << "solution: " << B::U_n << std::endl;
  //  std::cout << "phi: " << Phi_n << std::endl;

  //  std::cout << " AREA : " << wj.sum() << std::endl;

  //  std::cout << "phi [";

  //  for (Uint n=0; n < SF::nb_nodes; ++n)
  //    for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
  //      std::cout << Phi_n(n,v) << " ";
  //  std::cout << "]" << std::endl;

  //    if( idx() > 2 ) exit(0);



/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_Schemes_RKLDA_hpp
