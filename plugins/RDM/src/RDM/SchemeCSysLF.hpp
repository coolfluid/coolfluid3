// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_SchemeCSysLF_hpp
#define CF_Solver_SchemeCSysLF_hpp

#include <functional>

#include <boost/assign.hpp>

#include <Eigen/Dense>

#include "Common/Core.hpp"
#include "Common/OptionT.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/FindComponents.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/ElementData.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/ElementType.hpp"

#include "Solver/Actions/CLoopOperation.hpp"

#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template < typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS >
class RDM_API SchemeCSysLF : public Solver::Actions::CLoopOperation {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr< SchemeCSysLF > Ptr;
  typedef boost::shared_ptr< SchemeCSysLF const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  SchemeCSysLF ( const std::string& name );

  /// Virtual destructor
  virtual ~SchemeCSysLF() {};

  /// Get the class name
  static std::string type_name () { return "SchemeCSysLF<" + SHAPEFUNC::type_name() + ">"; }
	
  /// execute the action
  virtual void execute ();

private: // helper functions

  void change_elements()
  { 
    /// @todo improve this (ugly)

    connectivity_table = elements().as_ptr<Mesh::CElements>()->connectivity_table().as_ptr< Mesh::CTable<Uint> >();
    coordinates = elements().nodes().coordinates().as_ptr< Mesh::CTable<Real> >();

    cf_assert( is_not_null(connectivity_table) );

    /// @todo modify these to option components configured from

    Mesh::CField::Ptr csolution = Common::find_component_ptr_recursively_with_tag<Mesh::CField>( *Common::Core::instance().root(), "solution" );
    cf_assert( is_not_null( csolution ) );
    solution = csolution->data_ptr();

    Mesh::CField::Ptr cresidual = Common::find_component_ptr_recursively_with_tag<Mesh::CField>( *Common::Core::instance().root(), "residual" );
    cf_assert( is_not_null( cresidual ) );
    residual = cresidual->data_ptr();

    Mesh::CField::Ptr cwave_speed = Common::find_component_ptr_recursively_with_tag<Mesh::CField>( *Common::Core::instance().root(), "wave_speed" );
    cf_assert( is_not_null( cwave_speed ) );
    wave_speed = cwave_speed->data_ptr();
  }

private: // typedefs

  typedef typename SHAPEFUNC::NodeMatrixT                                 NodeMT;

  typedef Eigen::Matrix<Real, QUADRATURE::nb_points, 1u>                  WeightVT;

  typedef Eigen::Matrix<Real, QUADRATURE::nb_points, PHYSICS::nb_eqs>     ResidualMT;

  typedef Eigen::Matrix<Real, PHYSICS::nb_eqs, PHYSICS::nb_eqs >          EigenValueMT;

  typedef Eigen::Matrix<Real, PHYSICS::nb_eqs, PHYSICS::nb_eqs>           PhysicsMT;
  typedef Eigen::Matrix<Real, PHYSICS::nb_eqs, 1u>                        PhysicsVT;

  typedef Eigen::Matrix<Real, SHAPEFUNC::nb_nodes,   PHYSICS::nb_eqs>     SolutionMT;
  typedef Eigen::Matrix<Real, 1u, PHYSICS::nb_eqs >                       SolutionVT;

  typedef Eigen::Matrix<Real, QUADRATURE::nb_points, SHAPEFUNC::nb_nodes> SFMatrixT;
  typedef Eigen::Matrix<Real, 1u, SHAPEFUNC::nb_nodes >                   SFVectorT;

private: // data

  /// pointer to connectivity table, may reset when iterating over element types
  Mesh::CTable<Uint>::Ptr connectivity_table;
  /// pointer to nodes coordinates, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr coordinates;
  /// pointer to solution table, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr solution;
  /// pointer to solution table, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr residual;
  /// pointer to solution table, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr wave_speed;

  /// helper object to compute the quadrature information
  const QUADRATURE& m_quadrature;

  /// Nodal residuals
  SolutionMT Phi_n;
  /// node values
  NodeMT X_n;
  /// Values of the solution located in the dof of the element
  SolutionMT U_n;
  /// Values of the operator L(u) computed in quadrature points.
  PhysicsVT LU;
  /// temporary hold of Values of the operator L(u) computed in quadrature points.
  PhysicsVT LUwq;
  /// The operator L in the advection equation Lu = f
  /// Matrix Ki_qn stores the value L(N_i) at each quadrature point for each shape function N_i
  PhysicsMT Ki_n [SHAPEFUNC::nb_nodes];
  /// sum of Lplus to be inverted
  PhysicsMT sumLplus;
  /// inverse Ki+ matix
  PhysicsMT InvKi_n;
  /// flux jacobians
  PhysicsMT dFdU[DIM_2D];
  /// right eigen vector matrix
  PhysicsMT Rv;
  /// left eigen vector matrix
  PhysicsMT Lv;
  /// diagonal matrix with eigen values
  PhysicsVT Dv;
  /// diagonal matrix with positive eigen values
  PhysicsVT DvPlus [SHAPEFUNC::nb_nodes];

  /// gradient of shape function
  RealVector2 dN;

  /// jacobian of transformation at each quadrature point
  WeightVT jacob;
  /// Integration factor (jacobian multiplied by quadrature weight)
  WeightVT wj;

  // -----------------------------------------------------

  /// interporlation matrix - values of shapefunction at each quadrature point
  SFMatrixT Ni;
  /// derivative matrix - values of shapefunction derivative in Ksi at each quadrature point
  SFMatrixT dNdKsi;
  /// derivative matrix - values of shapefunction derivative in Eta at each quadrature point
  SFMatrixT dNdEta;

  /// coordinates of quadrature points in physical space
  Eigen::Matrix<Real,QUADRATURE::nb_points, DIM_2D>           X_q;
  /// solution at quadrature points in physical space
  Eigen::Matrix<Real,QUADRATURE::nb_points, PHYSICS::nb_eqs > U_q;

  /// derivatives of solution x
  Eigen::Matrix<Real,QUADRATURE::nb_points, PHYSICS::nb_eqs> dUdx;
  /// derivatives of solution y
  Eigen::Matrix<Real,QUADRATURE::nb_points, PHYSICS::nb_eqs> dUdy;

  /// stores dx/dksi and dx/deta at each quadrature point
  Eigen::Matrix<Real,QUADRATURE::nb_points, DIM_2D> dX;
  /// stores dy/dksi and dy/deta at each quadrature point
  Eigen::Matrix<Real,QUADRATURE::nb_points, DIM_2D> dY;

  /// derivatives of shape functions on XX at all quadrature points in physical space
  SFMatrixT dNdx;
  /// derivatives of shape functions on YY at all quadrature points in physical space
  SFMatrixT dNdy;

};

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
SchemeCSysLF<SHAPEFUNC,QUADRATURE,PHYSICS>::SchemeCSysLF ( const std::string& name ) :
  CLoopOperation(name),
  m_quadrature( QUADRATURE::instance() )
{
  regist_typeinfo(this);

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &SchemeCSysLF<SHAPEFUNC,QUADRATURE,PHYSICS>::change_elements, this ) );

  for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    DvPlus[n].setZero();

  dFdU[XX].setZero();
  dFdU[YY].setZero();

  // Gradient of the shape functions in reference space
  typename SHAPEFUNC::MappedGradientT GradSF;
  // Values of shape functions in reference space
  typename SHAPEFUNC::ShapeFunctionsT SF;

  // initialize the interpolation matrix
  for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    {
       SHAPEFUNC::mapped_gradient( m_quadrature.coords.col(q), GradSF );
       SHAPEFUNC::shape_function ( m_quadrature.coords.col(q), SF     );

       Ni(q,n) = SF[n];
       dNdKsi(q,n) = GradSF(KSI,n);
       dNdEta(q,n) = GradSF(ETA,n);
    }
}

/////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC,typename QUADRATURE, typename PHYSICS>
void SchemeCSysLF<SHAPEFUNC, QUADRATURE,PHYSICS>::execute()
{
  Phi_n.setZero();     // zero element residuals

  // get element connectivity

  const Mesh::CTable<Uint>::ConstRow nodes_idx = connectivity_table->array()[idx()];

  // copy the coordinates from the large array to a small

  Mesh::fill(X_n, *coordinates, nodes_idx );

  // copy the solution from the large array to a small

  for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    for (Uint v=0; v < PHYSICS::nb_eqs; ++v)
      U_n(n,v) = (*solution)[ nodes_idx[n] ][v];

  // coordinates of quadrature points in physical space

  X_q  = Ni * X_n;

  // solution at all quadrature points in physical space

  U_q = Ni * U_n;

  // Jacobian of transformation phys -> ref:
  //    |   dx/dksi    dx/deta    |
  //    |   dy/dksi    dy/deta    |

  // dX.col(KSI) has the values of dx/dksi at all quadrature points
  // dX.col(ETA) has the values of dx/deta at all quadrature points

  dX.col(KSI) = dNdKsi * X_n.col(XX);
  dX.col(ETA) = dNdEta * X_n.col(XX);
  dY.col(KSI) = dNdKsi * X_n.col(YY);
  dY.col(ETA) = dNdEta * X_n.col(YY);

  // transformation jacobian at quadrature point

  for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
    jacob[q] = dX(q,XX) * dY(q,YY) - dX(q,YY) * dY(q,XX);

  // compute transformed integration weights (sum is element area)

  for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
    wj[q] = jacob[q] * m_quadrature.weights[q];

  // shape function derivatives in physical space at quadrature point

  for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
  {
    const Real inv_jacob = 1.0 / jacob[q];
    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    {
      dNdx(q,n) = inv_jacob * (  dNdKsi(q,n)*dY(q,YY) - dNdEta(q,n) * dY(q,XX));
      dNdy(q,n) = inv_jacob * ( -dNdKsi(q,n)*dX(q,YY) + dNdEta(q,n) * dX(q,XX));
    }
  }

  // solution derivatives in physical space at quadrature point

  dUdx = dNdx * U_n;
  dUdy = dNdy * U_n;

  // L(N)+ @ each quadrature point

  for(Uint q=0; q < QUADRATURE::nb_points; ++q)
  {
    for(Uint n=0; n < SHAPEFUNC::nb_nodes; ++n)
    {
      dN[XX] = dNdx(q,n);
      dN[YY] = dNdy(q,n);

      PHYSICS::jacobian_eigen_structure(X_q.row(q),
                                        U_q.row(q),
                                        dN,
                                        Rv,
                                        Lv,
                                        Dv );

      // diagonal matrix of positive eigen values

      DvPlus[n] = Dv.unaryExpr(std::ptr_fun(plus));

      // Ki_n[n] = Rv * DvPlus[n].asDiagonal() * Lv;
    }

    // compute L(u)

    PHYSICS::Lu(X_q.row(q),
                U_q.row(q),
                dUdx.row(q).transpose(),
                dUdy.row(q).transpose(),
                dFdU,
                LU );

    // compute the phi_i LF intergral

    Real alpha = 0;
    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
      alpha = std::max( alpha, DvPlus[n].array().abs().maxCoeff() );

    const Real invdofs = 1./SHAPEFUNC::nb_nodes;

    for(Uint i = 0; i < SHAPEFUNC::nb_nodes; ++i)
    {
      Phi_n.row(i) += invdofs * LU * wj[q];          // central part
      for(Uint j=0; j < SHAPEFUNC::nb_nodes; ++j)    // plus dissipation
      {
        if (i == j) continue;
        Phi_n.row(i) += invdofs * alpha * (U_n.row(i).transpose() - U_n.row(j).transpose()) * wj[q]; // dissipation
      }

      // compute the wave_speed for scaling the update

      (*wave_speed)[nodes_idx[i]][0] += alpha * wj[q];
    }

  } // loop qd points

  // update the residual
  
  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    for (Uint v=0; v < PHYSICS::nb_eqs; ++v)
      (*residual)[nodes_idx[n]][v] += Phi_n(n,v);

}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_SchemeCSysLF_hpp
