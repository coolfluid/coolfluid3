// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_Schemes_CSysSUPG_hpp
#define CF_RDM_Schemes_CSysSUPG_hpp

#include "RDM/Core/CellTerm.hpp"
#include "RDM/Core/SchemeBase.hpp"

#include "RDM/Schemes/LibSchemes.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_SCHEMES_API CSysSUPG : public RDM::CellTerm {

public: // typedefs

  /// the actual scheme implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Term;

  typedef boost::shared_ptr< CSysSUPG > Ptr;
  typedef boost::shared_ptr< CSysSUPG const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CSysSUPG ( const std::string& name );

  /// Virtual destructor
  virtual ~CSysSUPG();

  /// Get the class name
  static std::string type_name () { return "CSysSUPG"; }

  /// Execute the loop for all elements
  virtual void execute();

};

///////////////////////////////////////////////////////////////////////////////////////

template < typename SF, typename QD, typename PHYS >
class RDM_SCHEMES_API CSysSUPG::Term : public SchemeBase<SF,QD,PHYS> {

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
  static std::string type_name () { return "CSysSUPG.Scheme<" + SF::type_name() + ">"; }

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
void CSysSUPG::Term<SF,QD,PHYS>::execute()
{
  // get element connectivity

  /// @TODO NOT FINISHED!!!

  const Mesh::CTable<Uint>::ConstRow nodes_idx = this->connectivity_table->array()[B::idx()];

  B::interpolate( nodes_idx );

  // element area

  const Real elem_area = B::wj.sum();

  const Real h = std::sqrt(elem_area); // characteristic length of element

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

    // stabilization parameter (based on max characteristic speed)
    /// @todo verify this

    Real ref_speed = 0.;
    for(Uint n = 0; n < SF::nb_nodes; ++n)
      ref_speed = std::max( ref_speed, Dv[n].array().abs().maxCoeff() );

    const Real tau_stab = 0.5 * h / ref_speed;

    // nodal residual

    for(Uint n = 0; n < SF::nb_nodes; ++n)
    {
      for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
        Ni[v] = B::Ni(q,n);

//      B::Phi_n.row(n) += (  tau_stab * Ki_n[n] + Ni.asDiagonal() ) * B::LU * B::wj[q];
    }

  } // loop qd points

  // update the residual

  for (Uint n=0; n<SF::nb_nodes; ++n)
    for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
      (*B::residual)[nodes_idx[n]][v] += B::Phi_n(n,v);

}

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_Schemes_CSysSUPG_hpp
