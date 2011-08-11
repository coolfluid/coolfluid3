// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_Schemes_CDiffGalerkin_hpp
#define CF_RDM_Schemes_CDiffGalerkin_hpp

#include "RDM/CellTerm.hpp"
#include "RDM/SchemeBase.hpp"

#include "RDM/Schemes/LibSchemes.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_SCHEMES_API CDiffGalerkin : public RDM::CellTerm {

public: // typedefs

  /// the actual scheme implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Term;

  typedef boost::shared_ptr< CDiffGalerkin > Ptr;
  typedef boost::shared_ptr< CDiffGalerkin const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CDiffGalerkin ( const std::string& name );

  /// Virtual destructor
  virtual ~CDiffGalerkin();

  /// Get the class name
  static std::string type_name () { return "CDiffGalerkin"; }

  /// Execute the loop for all elements
  virtual void execute();

};

///////////////////////////////////////////////////////////////////////////////////////

template < typename SF, typename QD, typename PHYS >
class RDM_SCHEMES_API CDiffGalerkin::Term : public SchemeBase<SF,QD,PHYS> {

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
  { regist_typeinfo(this); }

  /// Get the class name
  static std::string type_name () { return "CDiffGalerkin.Term<" + SF::type_name() + "," + PHYS::type_name() + ">"; }

  /// execute the action
  virtual void execute ();

private: // data

  /// The operator L in the advection equation Lu = f
  /// Matrix Ki_n stores the value L(N_i) at each quadrature point for each shape function N_i
  typename B::PhysicsMT Ki_n [SF::nb_nodes];
  /// right eigen vector matrix
  typename B::PhysicsMT Rv;
  /// left eigen vector matrix
  typename B::PhysicsMT Lv;
  /// diagonal matrix with eigen values
  typename B::PhysicsVT Dv [SF::nb_nodes];
  /// vector of shaep functions
  typename B::PhysicsVT Ni;
};

/////////////////////////////////////////////////////////////////////////////////////

template<typename SF,typename QD, typename PHYS>
void CDiffGalerkin::Term<SF,QD,PHYS>::execute()
{
  // get element connectivity

  /// @todo NOT FINISHED!!!

  const Mesh::CConnectivity::ConstRow nodes_idx = (*B::connectivity)[B::idx()];

  B::interpolate( nodes_idx );

  // L(N)+ @ each quadrature point

  for(Uint q=0; q < QD::nb_points; ++q)
  {
    for(Uint n=0; n < SF::nb_nodes; ++n)
    {
      B::dN[XX] = B::dNdX[XX](q,n);
      B::dN[YY] = B::dNdX[YY](q,n);

      PHYS::flux_jacobian_eigen_structure(B::phys_props,
                                     B::dN,
                                     Rv,
                                     Lv,
                                     Dv[n] );

       Ki_n[n] = Rv * Dv[n].asDiagonal() * Lv;
    }

    B::sol_gradients_at_qdpoint(q);

    // compute L(u)

    PHYS::residual(B::phys_props,
             B::dFdU,
             B::LU );

  } // loop qd points

  // update the residual

  for (Uint n=0; n<SF::nb_nodes; ++n)
    for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
      (*B::residual)[nodes_idx[n]][v] += B::Phi_n(n,v);

}

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_Schemes_CDiffGalerkin_hpp
