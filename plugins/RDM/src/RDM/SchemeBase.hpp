// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_SchemeBase_hpp
#define cf3_RDM_SchemeBase_hpp

#include <functional>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/assign.hpp>

#include "common/EigenAssertions.hpp"
#include <Eigen/Dense>

/// @todo remove when done
#include "common/Log.hpp"

#include "common/Core.hpp"
#include "common/OptionList.hpp"
#include "common/OptionComponent.hpp"
#include "common/BasicExceptions.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/ElementData.hpp"
#include "mesh/Field.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"

#include "physics/PhysModel.hpp"

#include "solver/actions/LoopOperation.hpp"

#include "RDM/LibRDM.hpp"
#include "RDM/CellLoop.hpp"
#include "RDM/Tags.hpp"

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

/// Base class for RD schemes
/// Templatized with the shape function, the quadrature rule and
/// the physical variables
/// @author Tiago Quintino
template < typename SF, typename QD, typename PHYS >
class RDM_API SchemeBase : public solver::actions::LoopOperation {

public: // typedefs

public: // functions

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  /// Contructor
  /// @param name of the component
  SchemeBase ( const std::string& name );

  /// Virtual destructor
  virtual ~SchemeBase() {}

  /// Get the class name
  static std::string type_name () { return "SchemeBase<" + SF::type_name() + ">"; }

  /// interpolates the shape functions and gradient values
  /// @post zeros the local residual matrix
  void interpolate ( const common::Table<Uint>::ConstRow& nodes_idx );

  void sol_gradients_at_qdpoint(const Uint q);

protected: // helper functions

  void change_elements()
  {
    //Handle<Component> fields=parent()->handle<ComputeDualArea>()->dual_area().parent();
    CFinfo << uri().path() << CFendl;

    connectivity = elements().template handle<mesh::Elements>()->geometry_space().connectivity().template handle< mesh::Connectivity >();
    coordinates = csolution->parent()->get_child(mesh::Tags::coordinates())->handle< mesh::Field >();
    solution   = csolution;
    residual   = cresidual;
    wave_speed = cwave_speed;

    cf3_assert( is_not_null(connectivity) );
    cf3_assert( is_not_null(coordinates) );
    cf3_assert( is_not_null(solution) );
    cf3_assert( is_not_null(residual) );
    cf3_assert( is_not_null(wave_speed) );
//    cf3_assert( is_not_null(elements().get_child("spaces")->get_child(RDM::Tags::solution())) );

    if( is_not_null(elements().get_child("spaces")->get_child(RDM::Tags::solution())) )
    {
      Handle<mesh::Space> space=elements().get_child("spaces")->get_child(RDM::Tags::solution())->template handle<mesh::Space>();
      cf3_assert( is_not_null(space) );
      connectivity = space->connectivity().handle<mesh::Connectivity>();
      coordinates  = space->dict().coordinates().handle<mesh::Field>();
    }

//    if (elements().handle<mesh::Elements>()->exists_space(std::string(RDM::Tags::solution())))
//    {
//      connectivity = elements().handle<mesh::Elements>()->space(std::string(RDM::Tags::solution())).connectivity().handle< mesh::Connectivity >();
//    }

    cf3_assert( is_not_null(connectivity) );
    cf3_assert( is_not_null(coordinates) );

    CFinfo << "PPPPPPPPPPPPPP1: " << connectivity->uri().path() << CFendl;
    CFinfo << "PPPPPPPPPPPPPP2: " << coordinates->uri().path() << CFendl;
    CFinfo << "PPPPPPPPPPPPPP3: " << solution->uri().path() << CFendl;
    CFinfo << "PPPPPPPPPPPPPP4: " << residual->uri().path() << CFendl;
    CFinfo << "PPPPPPPPPPPPPP5: " << wave_speed->uri().path() << CFendl;
  }

protected: // typedefs

  typedef typename SF::NodesT                                               NodeMT;

  typedef Eigen::Matrix<Real, QD::nb_points, 1u>                                 WeightVT;

  typedef Eigen::Matrix<Real, QD::nb_points, PHYS::MODEL::_neqs>                 ResidualMT;

  typedef Eigen::Matrix<Real, PHYS::MODEL::_neqs, PHYS::MODEL::_neqs >           EigenValueMT;

  typedef Eigen::Matrix<Real, PHYS::MODEL::_neqs, PHYS::MODEL::_neqs>            PhysicsMT;
  typedef Eigen::Matrix<Real, PHYS::MODEL::_neqs, 1u>                            PhysicsVT;

  typedef Eigen::Matrix<Real, SF::nb_nodes,   PHYS::MODEL::_neqs>                SolutionMT;
  typedef Eigen::Matrix<Real, 1u, PHYS::MODEL::_neqs >                           SolutionVT;

  typedef Eigen::Matrix<Real, QD::nb_points, SF::nb_nodes>                       SFMatrixT;
  typedef Eigen::Matrix<Real, 1u, SF::nb_nodes >                                 SFVectorT;

  typedef Eigen::Matrix<Real, PHYS::MODEL::_ndim, 1u>                            DimVT;

  typedef Eigen::Matrix<Real, PHYS::MODEL::_ndim, PHYS::MODEL::_ndim>            JMT;

  typedef Eigen::Matrix<Real, QD::nb_points, PHYS::MODEL::_ndim>                 QCoordMT;
  typedef Eigen::Matrix<Real, QD::nb_points, PHYS::MODEL::_neqs>                 QSolutionMT;

  typedef Eigen::Matrix<Real, PHYS::MODEL::_neqs, PHYS::MODEL::_ndim>            QSolutionVT;

protected: // data

  Handle< mesh::Field > csolution;   ///< solution field
  Handle< mesh::Field > cresidual;   ///< residual field
  Handle< mesh::Field > cwave_speed; ///< wave_speed field

  /// pointer to connectivity table, may reset when iterating over element types
  Handle< mesh::Connectivity > connectivity;
  /// pointer to nodes coordinates, may reset when iterating over element types
  Handle< mesh::Field > coordinates;
  /// pointer to solution table, may reset when iterating over element types
  Handle< mesh::Field > solution;
  /// pointer to solution table, may reset when iterating over element types
  Handle< mesh::Field > residual;
  /// pointer to solution table, may reset when iterating over element types
  Handle< mesh::Field > wave_speed;

  /// helper object to compute the quadrature information
  const QD& m_quadrature;

  /// coordinates of quadrature points in physical space
  QCoordMT X_q;
  /// stores dX/dksi and dx/deta at each quadrature point, one matrix per dimension
  QCoordMT dX[PHYS::MODEL::_ndim];

  /// solution at quadrature points in physical space
  QSolutionMT U_q;
  /// derivatives of solution to X at each quadrature point, one matrix per dimension
  QSolutionMT dUdX[PHYS::MODEL::_ndim];

  /// derivatives of solution to X at ONE quadrature point, each column stores derivatives
  /// with respect to given coordinate
  QSolutionVT dUdXq;

  /// contribution to nodal residuals
  SolutionMT Phi_n;
  /// node values
  NodeMT     X_n;
  /// Values of the solution located in the dof of the element
  SolutionMT U_n;
  /// Values of the operator L(u) computed in quadrature points.
  PhysicsVT  LU;
  /// flux jacobians
  PhysicsMT  dFdU[PHYS::MODEL::_ndim];
  /// interporlation matrix - values of shapefunction at each quadrature point
  SFMatrixT  Ni;
  /// derivative matrix - values of shapefunction derivative in Ksi at each quadrature point
  SFMatrixT  dNdKSI[PHYS::MODEL::_ndim];
  /// derivatives of shape functions on physical space at all quadrature points,
  /// one matrix per dimenison
  SFMatrixT  dNdX[PHYS::MODEL::_ndim];
  /// jacobian of transformation at each quadrature point
  WeightVT jacob;
  /// Integration factor (jacobian multiplied by quadrature weight)
  WeightVT wj;
  /// temporary local gradient of 1 shape function
  DimVT dN;

  typename PHYS::MODEL::Properties phys_props; ///< physical properties

protected:

  /// temporary local gradient of 1 shape function in reference and physical space
  DimVT dNphys;
  DimVT dNref;
  /// Jacobi matrix at each quadrature point
  JMT JM;
  /// Inverse of the Jacobi matrix at each quadrature point
  JMT JMinv;

};

////////////////////////////////////////////////////////////////////////////////////////////

template<typename SF, typename QD, typename PHYS>
SchemeBase<SF,QD,PHYS>::SchemeBase ( const std::string& name ) :
  LoopOperation(name),
  m_quadrature( QD::instance() )
{
  regist_typeinfo(this); // template class so must force type registration @ construction

  // options

  options().add(RDM::Tags::solution(), csolution).link_to(&csolution);
  options().add(RDM::Tags::wave_speed(), cwave_speed).link_to(&cwave_speed);
  options().add(RDM::Tags::residual(), cresidual).link_to(&cresidual);


  options()["elements"]
      .attach_trigger ( boost::bind ( &SchemeBase<SF,QD,PHYS>::change_elements, this ) );

  // initializations

  for(Uint d = 0; d < PHYS::MODEL::_ndim; ++d)
    dFdU[d].setZero();

  // Gradient of the shape functions in reference space
  typename SF::SF::GradientT GradSF;
  // Values of shape functions in reference space
  typename SF::SF::ValueT ValueSF;

  // initialize the interpolation matrix

  for(Uint q = 0; q < QD::nb_points; ++q)
  {
    // compute values and gradients of all functions in this quadrature point

    SF::SF::compute_gradient( m_quadrature.coords.col(q), GradSF  );
    SF::SF::compute_value   ( m_quadrature.coords.col(q), ValueSF );

    // copy the values to interpolation matrix

    Ni.row(q) = ValueSF.transpose();

    // copy the gradients

    for(Uint d = 0; d < PHYS::MODEL::_ndim; ++d)
      dNdKSI[d].row(q) = GradSF.row(d);
  }

  // debug

  //  std::cout << "QD points [" << QD::nb_points << "]"  << std::endl;
  //  std::cout << "Ki_qn   is " << Ki_n[0].rows()<< "x" << Ki_n[0].cols() << std::endl;
  //  std::cout << "LU      is " << LU.rows() << "x" << LU.cols()  << std::endl;
  //  std::cout << "Phi_n   is " << Phi_n.rows()   << "x" << Phi_n.cols()    << std::endl;
  //  std::cout << "U_n     is " << U_n.rows()     << "x" << U_n.cols()      << std::endl;
  //  std::cout << "DvPlus  is " << DvPlus[0].rows()  << "x" << DvPlus[0].cols()   << std::endl;

}


template<typename SF,typename QD, typename PHYS>
void SchemeBase<SF, QD,PHYS>::interpolate( const common::Table<Uint>::ConstRow& nodes_idx )
{
  /// @todo must be tested for 3D

  // copy the coordinates from the large array to a small

  mesh::fill(X_n, *coordinates, nodes_idx );

  // copy the solution from the large array to a small

  for(Uint n = 0; n < SF::nb_nodes; ++n)
    for (Uint v=0; v < PHYS::MODEL::_neqs; ++v)
      U_n(n,v) = (*solution)[ nodes_idx[n] ][v];

  // coordinates of quadrature points in physical space

  X_q  = Ni * X_n;

  // solution at all quadrature points in physical space

  U_q = Ni * U_n;

  // Jacobian of transformation phys -> ref:
  //    |   dx/dksi    dx/deta    |
  //    |   dy/dksi    dy/deta    |

  // dX[XX].col(KSI) has the values of dx/dksi at all quadrature points
  // dX[XX].col(ETA) has the values of dx/deta at all quadrature points

  for(Uint dimx = 0; dimx < PHYS::MODEL::_ndim; ++dimx)
    for(Uint dimksi = 0; dimksi < PHYS::MODEL::_ndim; ++dimksi)
    {
      dX[dimx].col(dimksi) = dNdKSI[dimksi] * X_n.col(dimx);
    }

  // Fill Jacobi matrix (matrix of transformation phys. space -> ref. space) at qd. point q

  for(Uint q = 0; q < QD::nb_points; ++q)
  {

    for(Uint dimx = 0; dimx < PHYS::MODEL::_ndim; ++dimx)
      for(Uint dimksi = 0; dimksi < PHYS::MODEL::_ndim; ++dimksi)
        JM(dimksi,dimx) = dX[dimx](q,dimksi);

    // Once the jacobi matrix at one quadrature point is assembled, let's re-use it
    // to compute the gradients of of all shape functions in phys. space
    jacob[q] = JM.determinant();
    JMinv = JM.inverse();

    for(Uint n = 0; n < SF::nb_nodes; ++n)
    {

      for(Uint dimksi = 0; dimksi < PHYS::MODEL::_ndim; ++ dimksi)
        dNref[dimksi] = dNdKSI[dimksi](q,n);

      dNphys = JMinv * dNref;

      for(Uint dimx = 0; dimx < PHYS::MODEL::_ndim; ++ dimx)
        dNdX[dimx](q,n) = dNphys[dimx];

    }

  } // loop over quadrature points

  // compute transformed integration weights (sum is element area)

  for(Uint q = 0; q < QD::nb_points; ++q)
    wj[q] = jacob[q] * m_quadrature.weights[q];

  // solution derivatives in physical space at quadrature point

  for(Uint dim = 0; dim < PHYS::MODEL::_ndim; ++dim)
    dUdX[dim] = dNdX[dim] * U_n;

  // zero element residuals

  Phi_n.setZero();
}


template<typename SF,typename QD, typename PHYS>
void SchemeBase<SF, QD,PHYS>::sol_gradients_at_qdpoint(const Uint q)
{

  for(Uint dim = 0; dim < PHYS::MODEL::_ndim; ++dim)
    dUdXq.col(dim) = dUdX[dim].row(q).transpose();

}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // cf3_RDM_SchemeBase_hpp
