// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_Schemes_LF_hpp
#define cf3_RDM_Schemes_LF_hpp

#include "RDM/CellTerm.hpp"
#include "RDM/SchemeBase.hpp"

#include "RDM/Schemes/LibSchemes.hpp"

namespace cf3 {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_SCHEMES_API LF : public RDM::CellTerm {

public: // typedefs

  /// the actual scheme implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Term;

  typedef boost::shared_ptr< LF > Ptr;
  typedef boost::shared_ptr< LF const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  LF ( const std::string& name );

  /// Virtual destructor
  virtual ~LF();

  /// Get the class name
  static std::string type_name () { return "LF"; }

  /// Execute the loop for all elements
  virtual void execute();

};

///////////////////////////////////////////////////////////////////////////////////////

template < typename SF, typename QD, typename PHYS >
class RDM_SCHEMES_API LF::Term : public SchemeBase<SF,QD,PHYS> {

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
  Term ( const std::string& name ) : SchemeBase<SF,QD,PHYS>(name) {     regist_typeinfo(this); }

  /// Get the class name
  static std::string type_name () { return "LF.Term<" + SF::type_name() + "," + PHYS::type_name() + ">"; }

  /// execute the action
  virtual void execute ();

private: // data

  /// diagonal matrix with positive eigen values
  typename B::PhysicsVT Dv [SF::nb_nodes];

};

/////////////////////////////////////////////////////////////////////////////////////

template<typename SF,typename QD, typename PHYS>
void LF::Term<SF,QD,PHYS>::execute()
{
  // get element connectivity

  const Mesh::CConnectivity::ConstRow nodes_idx = (*B::connectivity)[B::idx()];

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

      PHYS::flux_jacobian_eigen_values(B::phys_props,
                                  B::dN,
                                  Dv[n] );
    }

    // compute L(u)

    PHYS::residual(B::phys_props,
             B::dFdU,
             B::LU );

    // compute the phi_i LF intergral

    Real alpha = 0;
    for(Uint n = 0; n < SF::nb_nodes; ++n)
      alpha = std::max( alpha, Dv[n].array().abs().maxCoeff() );

    const Real invdofs = 1./SF::nb_nodes;

    for(Uint i = 0; i < SF::nb_nodes; ++i)
    {
      B::Phi_n.row(i) += invdofs * B::LU * B::wj[q];  // central part
      for(Uint j=0; j < SF::nb_nodes; ++j)            // plus dissipation
      {
        if (i == j) continue;
        B::Phi_n.row(i) += invdofs * alpha * (B::U_n.row(i).transpose() - B::U_n.row(j).transpose()) * B::wj[q]; // dissipation
      }

      // compute the wave_speed for scaling the update

      (*B::wave_speed)[nodes_idx[i]][0] += alpha * B::wj[q];
    }

  } // loop qd points

  // update the residual

  for (Uint n=0; n<SF::nb_nodes; ++n)
    for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
      (*B::residual)[nodes_idx[n]][v] += B::Phi_n(n,v);

}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // CF3_RDM_Schemes_LF_hpp
