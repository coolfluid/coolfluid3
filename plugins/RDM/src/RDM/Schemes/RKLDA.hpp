// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_Schemes_RKLDA_hpp
#define cf3_RDM_Schemes_RKLDA_hpp

#include <iostream>

#include "common/Log.hpp"

#include "common/PropertyList.hpp"
#include "common/StringConversion.hpp"

#include "mesh/Field.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/IterativeSolver.hpp"
#include "RDM/TimeStepping.hpp"

#include "RDM/CellTerm.hpp"
#include "RDM/SchemeBase.hpp"

#include "RDM/Schemes/LibSchemes.hpp"

namespace cf3 {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_SCHEMES_API RKLDA : public RDM::CellTerm {

public: // typedefs

  /// the actual scheme implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Term;




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



public: // functions

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  /// Contructor
  /// @param name of the component
  Term ( const std::string& name ) : SchemeBase<SF,QD,PHYS>(name)
  {
    regist_typeinfo(this);

    for(Uint n = 0; n < SF::nb_nodes; ++n)
      DvPlus[n].setZero();

    this->options().option("elements").attach_trigger ( boost::bind ( &RKLDA::Term<SF,QD,PHYS>::config_coeffs, this ) );

  }

  /// Get the class name
  static std::string type_name () { return "RKLDA.Term<" + SF::type_name() + "," + PHYS::type_name() + ">"; }

  /// execute the action
  virtual void execute ();

protected: // helper function

  void config_coeffs()
  {
    using namespace common;

//    CFinfo << "CHANGE ELEMS" << CFendl;
    //change_elements();

    RDSolver& mysolver = *this->parent()->template handle<CellTerm>()->solver().template handle<RDSolver>();
    rkorder = mysolver.properties().template value<Uint>("rkorder");
    step    = mysolver.iterative_solver().properties().template value<Uint>("iteration");
    dt      = mysolver.time_stepping().get_child("Time")->options().option("time_step").template value<Real>();

    k = step - 1;

    ksolutions.clear();
    ksolutions.push_back( follow_link( mysolver.fields().get_child( Tags::solution() ))->handle<mesh::Field>() );

    for ( Uint kstep = 1; kstep < rkorder; ++kstep)
    {
      ksolutions.push_back( follow_link( mysolver.fields().get_child( std::string(Tags::solution()) + "-" + to_str(kstep) + "dt" ))->handle<mesh::Field>() );
    }

    std::cout << "RKLDA   rkorder : " << rkorder << std::endl;
    std::cout << "RKLDA   step    : " << step    << std::endl;

    rkalphas.resize(rkorder,rkorder);
    rkbetas.resize(rkorder,rkorder);

    switch( rkorder )
    {
    case 1:

      rkalphas(0,0) = 1.;

      rkbetas(0,0) =  0.;

      break;

    case 2:

      rkalphas(0,0) =  1.  ;
      rkalphas(0,1) =  0.  ;
      rkalphas(1,0) =  0.5 ;
      rkalphas(1,1) =  0.5 ;

      rkbetas (0,0) =  0.0 ;
      rkbetas (0,1) =  0.0 ;
      rkbetas (1,0) = -1.0 ;
      rkbetas (1,1) =  1.0 ;

      break;

    case 3:

      rkalphas(0,0) = 1.    ;
      rkalphas(0,1) = 0.    ;
      rkalphas(0,2) = 0.    ;
      rkalphas(1,0) = 0.25  ;
      rkalphas(1,1) = 0.25  ;
      rkalphas(1,2) = 0.    ;
      rkalphas(2,0) = 1./6. ;
      rkalphas(2,1) = 1./6. ;
      rkalphas(2,2) = 2./3. ;

      rkbetas(0,0) =  0.  ;
      rkbetas(0,1) =  0.  ;
      rkbetas(0,2) =  0.  ;
      rkbetas(1,0) = -0.5 ;
      rkbetas(1,1) =  0.5 ;
      rkbetas(1,2) =  0.  ;
      rkbetas(2,0) = -2.  ;
      rkbetas(2,1) =  0.  ;
      rkbetas(2,2) =  2.  ;

      break ;

    }

    // initialize dFdU to zero
    for(Uint dim = 0; dim < PHYS::MODEL::_ndim; ++dim)
      B::dFdU[dim].setZero();

  }

protected: // data

  Uint rkorder; ///< order of the RK method
  Uint step;    ///< current RK step
  Uint k;       ///< current RK step - 1 (index to access coefficients)
  Real dt;      ///< time step size

  RealMatrix rkalphas;  ///< matrix with alpha coefficients of RK method
  RealMatrix rkbetas;   ///< matrix with beta  coefficients of RK method

  std::vector< Handle< mesh::Field > > ksolutions;  ///< solution fields at different k steps

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
  /// temporary hold of du values times mass matrix
  typename B::PhysicsVT  du_l;
  /// diagonal matrix with positive eigen values
  typename B::PhysicsVT  DvPlus [SF::nb_nodes];
  /// Eigen structure for the solutions for the different k steps (defined for RK4 but works with lower orders)
  typename B::SolutionMT sols_l [4];

};

/////////////////////////////////////////////////////////////////////////////////////

template<typename SF,typename QD, typename PHYS>
void RKLDA::Term<SF,QD,PHYS>::execute()
{
  // get element connectivity

  const mesh::Connectivity::ConstRow nodes_idx = (*B::connectivity)[B::idx()];

  // fill sols_l with the solutions until the current step

  for ( Uint l = 0 ; l < step ; ++l) // loop until current RK step
    for(Uint n = 0; n < SF::nb_nodes; ++n)
      for (Uint eq = 0; eq < PHYS::MODEL::_neqs; ++eq)
        sols_l[l](n,eq) = (*ksolutions[l])[ nodes_idx[n] ][eq];

  /// @todo must be tested for 3D

  // copy the coordinates from the large array to a small

  mesh::fill(B::X_n, *B::coordinates, nodes_idx );

  // coordinates of quadrature points in physical space

  B::X_q  = B::Ni * B::X_n;

  // Jacobian of transformation phys -> ref:
  //    |   dx/dksi    dx/deta    |
  //    |   dy/dksi    dy/deta    |

  // dX[XX].col(KSI) has the values of dx/dksi at all quadrature points
  // dX[XX].col(ETA) has the values of dx/deta at all quadrature points

  for(Uint dimx = 0; dimx < PHYS::MODEL::_ndim; ++dimx)
    for(Uint dimksi = 0; dimksi < PHYS::MODEL::_ndim; ++dimksi)
      B::dX[dimx].col(dimksi) = B::dNdKSI[dimksi] * B::X_n.col(dimx);

  // compute element-wise transformations that depend solely on geometry

  for(Uint q = 0; q < QD::nb_points; ++q)
  {
    for(Uint dimx = 0; dimx < PHYS::MODEL::_ndim; ++dimx)
      for(Uint dimksi = 0; dimksi < PHYS::MODEL::_ndim; ++dimksi)
        B::JM(dimksi,dimx) = B::dX[dimx](q,dimksi);

    // compute the gradients of of all shape functions in phys. space

    B::jacob[q] = B::JM.determinant();
    B::JMinv = B::JM.inverse();

    for(Uint n = 0; n < SF::nb_nodes; ++n)
    {
      for(Uint dimksi = 0; dimksi < PHYS::MODEL::_ndim; ++ dimksi)
        B::dNref[dimksi] = B::dNdKSI[dimksi](q,n);

      B::dNphys = B::JMinv * B::dNref;

      for(Uint dimx = 0; dimx < PHYS::MODEL::_ndim; ++ dimx)
        B::dNdX[dimx](q,n) = B::dNphys[dimx];
    }

    // compute transformed integration weights (sum is element area)

    B::wj[q] = B::jacob[q] * B::m_quadrature.weights[q];

  } // loop quadrature points

  // zero element residuals

  B::Phi_n.setZero();

  for ( Uint l = 0 ; l < step ; ++l) // loop until current RK step
  {
    // copy the solution from the global array to a local (Eigen) matrix

    B::U_n = sols_l[l];

    // interpolate solution at all quadrature points in physical space

    B::U_q = B::Ni * B::U_n;

    // solution derivatives in physical space at quadrature point

    for(Uint dim = 0; dim < PHYS::MODEL::_ndim; ++dim)
      B::dUdX[dim] = B::dNdX[dim] * B::U_n;

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

      PHYS::residual(B::phys_props, B::dFdU, B::LU );

      // compute L(N)+

      sumLplus = Ki_n[0];
      for(Uint n = 1 ; n < SF::nb_nodes ; ++n)
        sumLplus += Ki_n[n];

      // invert the sum L plus

      InvKi_n = sumLplus.inverse();

      // compute du_l

      du_l.setZero();

      for (Uint j = 0; j < SF::nb_nodes; ++j)   // loop over each node
        du_l += B::Ni(q,j) * rkbetas(k,l) * sols_l[l].row(j);

      for(Uint i = 0 ; i < SF::nb_nodes ; ++i)
        B::Phi_n.row(i) += (
                              Ki_n[i] * InvKi_n * ( du_l + dt * rkalphas(k,l) * B::LU )
                             -
                              B::Ni(q,i) * rkbetas(k,l) * sols_l[l].row(i).transpose()
                           )
                           * B::wj[q];

       // if last k step, compute the wave_speed ( in case we need to adapt the dt )

      if( l == (rkorder - 1) )
        for(Uint n = 0 ; n < SF::nb_nodes ; ++n)
          (*B::wave_speed)[nodes_idx[n]][0] += DvPlus[n].maxCoeff() * B::wj[q];

    } // loop over each quadrature point

  } // loop over each l for the same step

  // sum the local residual to the global residual

  for (Uint n=0; n<SF::nb_nodes; ++n)
    for (Uint eq=0; eq < PHYS::MODEL::_neqs; ++eq)
      (*B::residual)[nodes_idx[n]][eq] += B::Phi_n(n,eq) ;

} // !RKLDA::Term<SF,QD,PHYS>::execute()

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // cf3_RDM_Schemes_RKLDA_hpp
