// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_SchemeCSysN_hpp
#define CF_RDM_SchemeCSysN_hpp

#include "RDM/Core/SchemeBase.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template < typename SF, typename QD, typename PHYS >
class RDM_SCHEMES_API CSysN::Term : public SchemeBase<SF,QD,PHYS> {

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
      DvP[n].setZero();
  }

  /// Get the class name
  static std::string type_name () { return "CSysN.Scheme<" + SF::type_name() + ">"; }

  /// execute the action
  virtual void execute ();

protected: // data

  /// Matrix KiP_n stores the value L(N_i)+ at each quadrature point for each shape function N_i
  typename B::PhysicsMT  KiP_n [SF::nb_nodes];
  /// Matrix KiM_n stores the value L(N_i)- at each quadrature point for each shape function N_i
  typename B::PhysicsMT  KiM_n [SF::nb_nodes];
  /// sum of Lplus to be inverted
  typename B::PhysicsMT  sumKmin;
  /// inverse Ki+ matix
  typename B::PhysicsMT  InvKi_n;
  /// right eigen vector matrix
  typename B::PhysicsMT  Rv;
  /// left eigen vector matrix
  typename B::PhysicsMT  Lv;
  /// Ki matrix
  typename B::PhysicsMT  Ki;
  /// diagonal matrix with eigen values
  typename B::PhysicsVT  Dv;
  /// diagonal matrix with positive eigen values
  typename B::PhysicsVT  DvP [SF::nb_nodes];
  /// diagonal matrix with negative eigen values
  typename B::PhysicsVT  DvM [SF::nb_nodes];

};

/////////////////////////////////////////////////////////////////////////////////////

template<typename SF,typename QD, typename PHYS>
void CSysN::Term<SF,QD,PHYS>::execute()
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

      DvP[n] = Dv.unaryExpr(std::ptr_fun(plus));
      DvM[n] = Dv.unaryExpr(std::ptr_fun(minus));

      KiP_n[n] = Rv * DvP[n].asDiagonal() * Lv;
      KiM_n[n] = Rv * DvM[n].asDiagonal() * Lv;
    }

    // compute L(u)

    PHYS::residual(B::phys_props,
             B::dFdU,
             B::LU );

    // compute sum L(N)-

    sumKmin = KiP_n[0];
    for(Uint n = 1; n < SF::nb_nodes; ++n)
      sumKmin += KiP_n[n];

    // invert the sum K min

    InvKi_n = sumKmin.inverse();

//    for(Uint i = 0; i < SF::nb_nodes; ++i)
//      for(Uint j = 0; j < SF::nb_nodes; ++j)
//      {
//        typename B::PhysicsVT Udiff =  B::U_n.row(i).transpose()  - B::U_n.row(j).transpose();
//        if ( Udiff.array().abs().sum() > 10E-6 )
//        {
//          std::cout << "B::U_n.row(i).transpose() : " << B::U_n.row(i).transpose() << std::endl;
//          std::cout << "B::U_n.row(j).transpose() : " << B::U_n.row(j).transpose() << std::endl;
//          std::cout << "Udiff : " << Udiff << std::endl;
//        }
//      }

    // N scheme

    for(Uint i = 0; i < SF::nb_nodes; ++i)
    {
      Ki = KiP_n[i] * InvKi_n;

//      std::cout << "Ki : " << Ki << std::endl;

      for(Uint j = 0; j < SF::nb_nodes; ++j)
      {
        if (i==j) continue;
        B::Phi_n.row(i) -= Ki * KiM_n[j] * ( B::U_n.row(i).transpose() - B::U_n.row(j).transpose() ) * B::wj[q];

//        std::cout << "Phi -= " << Ki * KiM_n[j] * ( B::U_n.row(i).transpose() - B::U_n.row(j).transpose() ) << std::endl;

      }
    }

    // compute the wave_speed for scaling the update

    for(Uint n = 0; n < SF::nb_nodes; ++n)
      (*B::wave_speed)[nodes_idx[n]][0] += DvP[n].maxCoeff() * B::wj[q];

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
//    std::cout << "phi: " << B::Phi_n << std::endl;

  //  std::cout << " AREA : " << wj.sum() << std::endl;

  //  std::cout << "phi [";

  //  for (Uint n=0; n < SF::nb_nodes; ++n)
  //    for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
  //      std::cout << Phi_n(n,v) << " ";
  //  std::cout << "]" << std::endl;


//  if( B::idx() == 307 ) exit(0);

}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_SchemeCSysN_hpp
