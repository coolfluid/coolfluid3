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
#include "Mesh/Field.hpp"
#include "Mesh/FieldView.hpp"
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

  Mesh::Field& dual_area() { return *cdual_area.lock(); }

protected: // helper functions

  void create_dual_area_field();

private: // data

  boost::weak_ptr< Mesh::Field > cdual_area;  ///< dual area

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
    dual_area   = parent().as_type<ComputeDualArea>().dual_area().data_ptr();
  }

protected: // typedefs

  typedef typename SF::NodeMatrixT                          NodeMT;
  typedef Eigen::Matrix<Real, QD::nb_points, 1u>            WeightVT;
  typedef Eigen::Matrix<Real, QD::nb_points, SF::nb_nodes>  SFMatrixT;
  typedef Eigen::Matrix<Real, 1u, SF::nb_nodes >            SFVectorT;
  typedef Eigen::Matrix<Real, QD::nb_points, SF::dimension> QCoordMT;
  typedef Eigen::Matrix<Real, SF::dimension, SF::dimension> JMT;

protected: // data

  boost::weak_ptr< Mesh::Field > csolution;  ///< solution field

  /// pointer to solution table, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr solution;
  /// pointer to dual area table
  Mesh::CTable<Real>::Ptr dual_area;
  /// pointer to connectivity table, may reset when iterating over element types
  Mesh::CTable<Uint>::Ptr connectivity_table;
  /// pointer to nodes coordinates, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr coordinates;

  /// helper object to compute the quadrature information
  const QD& m_quadrature;

  /// node values
  NodeMT     X_n;
  /// interporlation matrix - values of shapefunction at each quadrature point
  SFMatrixT  Ni;
  /// derivative matrix - values of shapefunction derivative in Ksi at each quadrature point
  SFMatrixT  dNdKSI[SF::dimension];
  /// local dual area, per node per element
  SFVectorT  wi;
  /// Jacobi matrix at each quadrature point
  JMT JM;
  /// coordinates of quadrature points in physical space
  QCoordMT X_q;
  /// stores dX/dksi and dx/deta at each quadrature point, one matrix per dimension
  QCoordMT dX[SF::dimension];
  /// jacobian of transformation at each quadrature point
  WeightVT jacob;
  /// Integration factor (jacobian multiplied by quadrature weight)
  WeightVT wj;

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
        Common::OptionComponent<Mesh::Field>::create( RDM::Tags::solution(), &csolution));

  m_options["Elements"]
      .attach_trigger ( boost::bind ( &ComputeDualArea::Term<SF,QD>::change_elements, this ) );

  // initializations

  // Gradient of the shape functions in reference space
  typename SF::MappedGradientT GradSF;
  // Values of shape functions in reference space
  typename SF::ShapeFunctionsT ValueSF;

  // initialize the interpolation matrix

  for(Uint q = 0; q < QD::nb_points; ++q)
  {
    // compute values of all functions in this quadrature point

    SF::shape_function_gradient( m_quadrature.coords.col(q), GradSF  );
    SF::shape_function_value   ( m_quadrature.coords.col(q), ValueSF );

    // copy the values to interpolation matrix

    Ni.row(q) = ValueSF.transpose();

    // copy the gradients

    for(Uint d = 0; d < SF::dimension; ++d)
      dNdKSI[d].row(q) = GradSF.row(d);

  }
}



template<typename SF,typename QD >
void ComputeDualArea::Term<SF,QD>::execute()
{
  using namespace CF::Math;

//  std::cout << " dual area @ cell [" << idx() << "]" << std::endl;


  // get element connectivity

  const Mesh::CTable<Uint>::ConstRow nodes_idx = this->connectivity_table->array()[idx()];

  // copy the coordinates from the large array to a small

  Mesh::fill(X_n, *coordinates, nodes_idx );

  // interpolation of coordinates to quadrature points

  X_q  = Ni * X_n;

  // interpolation of gradients to quadrature points

  for(Uint dimx = 0; dimx < SF::dimension; ++dimx)
    for(Uint dimksi = 0; dimksi < SF::dimension; ++dimksi)
      dX[dimx].col(dimksi) = dNdKSI[dimksi] * X_n.col(dimx);

  // sum @ each quadrature point

  wi.setZero();

  for(Uint q=0; q < QD::nb_points; ++q)
  {

    // jacobian of transformation phys -> ref:

    for(Uint dimx = 0; dimx < SF::dimension; ++dimx)
      for(Uint dimksi = 0; dimksi < SF::dimension; ++dimksi)
        JM(dimksi,dimx) = dX[dimx](q,dimksi);

    jacob[q] = JM.determinant();

    // integration point weight

    wj[q] = jacob[q] * m_quadrature.weights[q];

    wi += Ni.row(q).transpose() * wj[q];

  } // loop qd points

  // sum contribution to global table of dual areas

  for (Uint n=0; n<SF::nb_nodes; ++n)
    (*dual_area)[nodes_idx[n]][0] += wi[n];

}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_ComputeDualArea_hpp
