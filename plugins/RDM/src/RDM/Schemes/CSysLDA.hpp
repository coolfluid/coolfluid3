// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_Schemes_CSysLDA_hpp
#define CF_RDM_Schemes_CSysLDA_hpp

#include "RDM/CellTerm.hpp"
#include "RDM/SchemeBase.hpp"

#include "RDM/Schemes/LibSchemes.hpp"

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_SCHEMES_API CSysLDA : public RDM::CellTerm {

public: // typedefs

  /// the actual scheme implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Term;

  typedef boost::shared_ptr< CSysLDA > Ptr;
  typedef boost::shared_ptr< CSysLDA const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CSysLDA ( const std::string& name );

  /// Virtual destructor
  virtual ~CSysLDA();

  /// Get the class name
  static std::string type_name () { return "CSysLDA"; }

  /// Execute the loop for all elements
  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////


template < typename SF, typename QD, typename PHYS >
class RDM_SCHEMES_API CSysLDA::Term : public SchemeBase<SF,QD,PHYS> {

public: // typedefs

  /// base class type
  typedef SchemeBase<SF,QD,PHYS> B;

  /// pointers
  typedef boost::shared_ptr< Term > Ptr;
  typedef boost::shared_ptr< Term const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  Term ( const std::string& name ) : SchemeBase<SF,QD,PHYS>(name)
  {
    regist_typeinfo(this);

    for(Uint n = 0; n < SF::nb_nodes; ++n)
      DvPlus[n].setZero();
  }

  /// Get the class name
  static std::string type_name () { return "CSysLDA.Scheme<" + SF::type_name() + ">"; }

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

template<typename SF,typename QD, typename PHYS>
void CSysLDA::Term<SF,QD,PHYS>::execute()
{
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


  // debug

  //  std::cout << "LDA ELEM [" << idx() << "]" << std::endl;
  //  std::cout << "  Operator:" << std::endl;
  //  std::cout << "               Sum of weights = " << m_quadrature.weights.sum() << std::endl;
  //  std::cout << "               Jacobians in physical space:" << std::endl;
  //  std::cout << jacob << std::endl;
  //  std::cout << "LDA: Area = " << wj.sum() << std::endl;


  //    std::cout << "X    [" << q << "] = " << X_q.row(q)    << std::endl;
  //    std::cout << "U    [" << q << "] = " << U_q.row(q)     << std::endl;
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

  //  std::cout << "solution: " << U_n << std::endl;
  //  std::cout << "phi: " << Phi_n << std::endl;

  //  std::cout << " AREA : " << wj.sum() << std::endl;

  //  std::cout << "phi [";

  //  for (Uint n=0; n < SF::nb_nodes; ++n)
  //    for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
  //      std::cout << Phi_n(n,v) << " ";
  //  std::cout << "]" << std::endl;

  //    if( idx() > 2 ) exit(0);

}

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_Schemes_CSysLDA_hpp
