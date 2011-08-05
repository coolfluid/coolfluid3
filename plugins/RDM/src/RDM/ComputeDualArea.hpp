// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_ComputeDualArea_hpp
#define CF_RDM_ComputeDualArea_hpp

#include "Common/OptionComponent.hpp"

#include "Math/Checks.hpp"

#include "Mesh/CTable.hpp"
#include "Mesh/ElementData.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/ElementType.hpp"
#include "Solver/Actions/CLoopOperation.hpp"

#include "RDM/CellTerm.hpp"

#include "RDM/LibRDM.hpp"

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

class RDM_API ComputeDualArea : public RDM::CellTerm {

public: // typedefs

  template < typename SF, typename QD > class Term;

  typedef boost::shared_ptr< ComputeDualArea > Ptr;
  typedef boost::shared_ptr< ComputeDualArea const > ConstPtr;

public: // functions

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  /// Contructor
  /// @param name of the component
  ComputeDualArea ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeDualArea();

  /// Get the class name
  static std::string type_name () { return "ComputeDualArea"; }

  /// Execute the loop for all elements
  virtual void execute();

};

////////////////////////////////////////////////////////////////////////////////////////////


template < typename SF, typename QD >
class RDM_API ComputeDualArea::Term : public Solver::Actions::CLoopOperation {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr< Term > Ptr;
  typedef boost::shared_ptr< Term const> ConstPtr;

public: // functions

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  /// Contructor
  /// @param name of the component
  Term ( const std::string& name );

  /// Virtual destructor
  virtual ~Term() {}

  /// Get the class name
  static std::string type_name () { return "ComputeDualArea.Term<" + SF::type_name() + ">"; }

  /// execute the action
  virtual void execute ();

protected: // helper functions

  void change_elements()
  {
    connectivity_table =
        elements().as_ptr<Mesh::CElements>()->node_connectivity().as_ptr< Mesh::CTable<Uint> >();
    coordinates =
        elements().nodes().coordinates().as_ptr< Mesh::CTable<Real> >();

    cf_assert( is_not_null(connectivity_table) );
    cf_assert( is_not_null(coordinates) );

    solution   = csolution.lock()->data_ptr();
  }

protected: // data

  boost::weak_ptr< Mesh::CField > csolution;   ///< solution field

  /// pointer to solution table, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr solution;
  /// pointer to connectivity table, may reset when iterating over element types
  Mesh::CTable<Uint>::Ptr connectivity_table;
  /// pointer to nodes coordinates, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr coordinates;

  /// helper object to compute the quadrature information
  const QD& m_quadrature;
};

////////////////////////////////////////////////////////////////////////////////////////////



template<typename SF, typename QD>
ComputeDualArea::Term<SF,QD>::Term ( const std::string& name ) :
  CLoopOperation(name),
  m_quadrature( QD::instance() )
{
  regist_typeinfo(this); // template class so must force type registration @ construction

  // options

  m_options.add_option(
        Common::OptionComponent<Mesh::CField>::create( RDM::Tags::solution(), &csolution));

  m_options["Elements"]
      .attach_trigger ( boost::bind ( &ComputeDualArea::Term<SF,QD>::change_elements, this ) );

  // initializations
#if 0
  // Values of shape functions in reference space
  typename SF::ShapeFunctionsT ValueSF;

  // initialize the interpolation matrix

  for(Uint q = 0; q < QD::nb_points; ++q)
  {
    // compute values of all functions in this quadrature point

    SF::shape_function_value   ( m_quadrature.coords.col(q), ValueSF );

    // copy the values to interpolation matrix

    Ni.row(q) = ValueSF.transpose();
  }
#endif
}



template<typename SF,typename QD >
void ComputeDualArea::Term<SF,QD>::execute()
{
#if 0
  using namespace CF::Math;

  // get element connectivity

  const Mesh::CTable<Uint>::ConstRow nodes_idx = this->connectivity_table->array()[B::idx()];

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

    // compute L(N)+

    sumLplus = Ki_n[0];
    for(Uint n = 1; n < SF::nb_nodes; ++n)
      sumLplus += Ki_n[n];

    // invert the sum L plus

    InvKi_n = sumLplus.inverse();

    // compute the phi_i ComputeDualArea intergral

    LUwq = InvKi_n * B::LU * B::wj[q];

//    LUwq.unaryExpr(std::ptr_fun(lolo));  /* NOT CALLLED ??? */

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

}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_ComputeDualArea_hpp
