// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_SchemeCSysLDA_hpp
#define CF_Solver_SchemeCSysLDA_hpp

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
class RDM_API SchemeCSysLDA : public Solver::Actions::CLoopOperation {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr< SchemeCSysLDA > Ptr;
  typedef boost::shared_ptr< SchemeCSysLDA const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  SchemeCSysLDA ( const std::string& name );

  /// Virtual destructor
  virtual ~SchemeCSysLDA() {};

  /// Get the class name
  static std::string type_name () { return "SchemeCSysLDA<" + SHAPEFUNC::type_name() + ">"; }
	
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

  /// Values of the solution located in the dof of the element
  SolutionMT U_n;
  /// Values of the operator L(u) computed in quadrature points.
  PhysicsVT LU_q [QUADRATURE::nb_points];
  /// Nodal residuals
  SolutionMT Phi_n;
  /// The operator L in the advection equation Lu = f
  /// Matrix Ki_qn stores the value L(N_i) at each quadrature point for each shape function N_i
  PhysicsMT Ki_qn [QUADRATURE::nb_points*SHAPEFUNC::nb_nodes];
  /// inverse Ki+ matix
  PhysicsMT InvKi_qn;
  /// diagonal matrix with positive eigen values
  PhysicsMT DvPlus [QUADRATURE::nb_points*SHAPEFUNC::nb_nodes];
  /// inflow vector for N dissipation
  PhysicsVT SumKu;
  /// node values
  NodeMT nodes;

  /// Integration factor (jacobian multiplied by quadrature weight)
  WeightVT wj;

  PhysicsMT sumLplus;

  Real elem_area;

  // -----------------------------------------------------

  RealVector2 m_qd_pt; // quadrature point in physical space
  RealVector2 m_dN;    // derivatives of shape functions in physical space

  Real m_u_q; //Solution value at quadrature point

  SFMatrixT m_N;
  SFMatrixT m_dNdksi;
  SFMatrixT m_dNdeta;

  typename SHAPEFUNC::MappedGradientT m_sf_grad_ref; // Gradient of the shape functions in reference space
  typename SHAPEFUNC::ShapeFunctionsT m_sf_ref;      // Values of shape functions in reference space

  Eigen::Matrix<Real,QUADRATURE::nb_points, DIM_2D> m_qdpos; //coordinates of quadrature points in physical space
  Eigen::Matrix<Real,QUADRATURE::nb_points, PHYSICS::nb_eqs > m_u_qd; //solution at quadrature points in physical space

  Eigen::Matrix<Real,QUADRATURE::nb_points, PHYSICS::nb_eqs> dUdx; //derivatives of solution x
  Eigen::Matrix<Real,QUADRATURE::nb_points, PHYSICS::nb_eqs> dUdy; //derivatives of solution y

  Eigen::Matrix<Real,QUADRATURE::nb_points, DIM_2D> m_dx; // stores dx/dksi and dx/deta at each quadrature point
  Eigen::Matrix<Real,QUADRATURE::nb_points, DIM_2D> m_dy; // stores dy/dksi and dy/deta at each quadrature point

  WeightVT jacob;      // jacobian of transformation phys->ref at each qd. pt

  SFMatrixT m_dNdx; //Derivatives of shape functions
                                                                        //at all quadrature points in phys. space
  SFMatrixT m_dNdy;

  /// flux jacobians
  PhysicsMT dFdU[DIM_2D];
  /// right eigen vector matrix
  PhysicsMT Rv;
  /// left eigen vector matrix
  PhysicsMT Lv;
  /// diagonal matrix with eigen values
  PhysicsMT Dv;
  /// gradient of shape function
  RealVector2 dN;

};

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
SchemeCSysLDA<SHAPEFUNC,QUADRATURE,PHYSICS>::SchemeCSysLDA ( const std::string& name ) :
  CLoopOperation(name),
  m_quadrature( QUADRATURE::instance() )
{
  regist_typeinfo(this);

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &SchemeCSysLDA<SHAPEFUNC,QUADRATURE,PHYSICS>::change_elements, this ) );

  std::cout << "QD points [" << QUADRATURE::nb_points << "]"  << std::endl;
  std::cout << "Ki_qn   is " << Ki_qn[0].rows()<< "x" << Ki_qn[0].cols() << std::endl;
  std::cout << "LU_q    is " << LU_q[0].rows() << "x" << LU_q[0].cols()  << std::endl;
  std::cout << "Phi_n   is " << Phi_n.rows()   << "x" << Phi_n.cols()    << std::endl;
  std::cout << "U_n     is " << U_n.rows()     << "x" << U_n.cols()      << std::endl;
  std::cout << "DvPlus  is " << DvPlus[0].rows()  << "x" << DvPlus[0].cols()   << std::endl;

  for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
      DvPlus[q*QUADRATURE::nb_points + n].setZero();

  // initialize the interpolation matrix
  for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    {
       SHAPEFUNC::mapped_gradient( m_quadrature.coords.col(q), m_sf_grad_ref );
       SHAPEFUNC::shape_function ( m_quadrature.coords.col(q), m_sf_ref   );

       m_N(q,n) = m_sf_ref[n];
       m_dNdksi(q,n) = m_sf_grad_ref(KSI,n);
       m_dNdeta(q,n) = m_sf_grad_ref(ETA,n);
    }
}

/////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC,typename QUADRATURE, typename PHYSICS>
void SchemeCSysLDA<SHAPEFUNC, QUADRATURE,PHYSICS>::execute()
{

//  std::cout << "ELEM [" << idx() << "]" << std::endl;

  Phi_n.setZero();     // zero element residuals
  elem_area = 0.;      // zero element area

  // get element connectivity

  const Mesh::CTable<Uint>::ConstRow nodes_idx = connectivity_table->array()[idx()];

  // copy the coordinates from the large array to a small

  Mesh::fill(nodes, *coordinates, nodes_idx );

  // copy the solution from the large array to a small

  for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    for (Uint v=0; v < PHYSICS::nb_eqs; ++v)
      U_n(n,v) = (*solution)[ nodes_idx[n] ][v];

//  m_oper.compute(nodes, U_n, Ki_qn, LU_q, DvPlus, wj);

  m_qdpos  = m_N * nodes; // coordinates of quadrature points in physical space

  m_u_qd = m_N * U_n; // solution at all quadrature points in physical space

  //Jacobian of transformation phys -> ref:
  //    |   dx/dksi    dx/deta    |
  //    |   dy/dksi    dy/deta    |
  // m_dx.col(KSI) has the values of dx/dksi at all quadrature points
  // m_dx.col(ETA) has the values of dx/deta at all quadrature points
  m_dx.col(KSI) = m_dNdksi * nodes.col(XX);
  m_dx.col(ETA) = m_dNdeta * nodes.col(XX);
  m_dy.col(KSI) = m_dNdksi * nodes.col(YY);
  m_dy.col(ETA) = m_dNdeta * nodes.col(YY);

  // compute L(u) and L(N) @ each quadrature point

  for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
      jacob[q] = m_dx(q,XX) * m_dy(q,YY) - m_dx(q,YY) * m_dy(q,XX);

  // Shape function derivatives in physical space at quadrature pts
  for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    {
      m_dNdx(q,n) = 1.0/jacob[q] * (  m_dNdksi(q,n)*m_dy(q,YY) - m_dNdeta(q,n) * m_dy(q,XX));
      m_dNdy(q,n) = 1.0/jacob[q] * ( -m_dNdksi(q,n)*m_dx(q,YY) + m_dNdeta(q,n) * m_dx(q,XX));
    }

  dUdx = m_dNdx * U_n;
  dUdy = m_dNdy * U_n;

 for(Uint q=0; q < QUADRATURE::nb_points; ++q)
 {
   for(Uint n=0; n < SHAPEFUNC::nb_nodes; ++n)
   {
     dN[XX] = m_dNdx(q,n);
     dN[YY] = m_dNdy(q,n);

     PHYSICS::jacobian_eigen_structure(m_qdpos.row(q),
                                       m_u_qd.row(q),
                                       dN,
                                       Rv,
                                       Lv,
                                       Dv,
                                       DvPlus [ q * QUADRATURE::nb_points + n ],
                                       Ki_qn    [ q * QUADRATURE::nb_points + n ] );
   }

   PHYSICS::Lu(m_qdpos.row(q),
               m_u_qd.row(q),
               dUdx.row(q).transpose(),
               dUdy.row(q).transpose(),
               dFdU,
               LU_q[q]);

   wj[q] = jacob[q]*m_quadrature.weights[q];


 } // loop over quadrature points

  // compute L(N)+]

  for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
  {
    elem_area += wj[q];

    sumLplus.setZero();

    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
      sumLplus += Ki_qn[q*QUADRATURE::nb_points + n];

    // invert the sum L plus

    InvKi_qn = sumLplus.inverse();

    // compute the phi_i LDA intergral

    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    {
      PhysicsMT& KiPlus = Ki_qn[q*QUADRATURE::nb_points + n];

      Phi_n.row(n) +=  KiPlus * InvKi_qn * LU_q[q] * wj[q];
    }

    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    {
      Real max_eigen_value = 0;
      for (Uint v=0; v < PHYSICS::nb_eqs; ++v)
        max_eigen_value = std::max( max_eigen_value, DvPlus[q*QUADRATURE::nb_points + n](v,v) );

      (*wave_speed)[nodes_idx[n]][0] += max_eigen_value * wj[q];
    }

	} // loop qd points

  // update the residual
  
  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    for (Uint v=0; v < PHYSICS::nb_eqs; ++v)
      (*residual)[nodes_idx[n]][v] += Phi_n(n,v);

  //    std::cout << "X    [" << q << "] = " << m_qdpos.row(q)    << std::endl;
  //    std::cout << "U    [" << q << "] = " << m_u_qd.row(q)     << std::endl;
  //    std::cout << "dUdx [" << q << "] = " << dUdx.row(q)       << std::endl;
  //    std::cout << "dUdy [" << q << "] = " << dUdy.row(q)       << std::endl;
  //    std::cout << "LU   [" << q << "] = " << Lu_q[q].transpose() << std::endl;
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

  //  for (Uint n=0; n < SHAPEFUNC::nb_nodes; ++n)
  //    for (Uint v=0; v < PHYSICS::nb_eqs; ++v)
  //      std::cout << Phi_n(n,v) << " ";
  //  std::cout << "]" << std::endl;

  //  if( idx() > 2 ) exit(0);

}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#if 0
    // compute the phi_i N dissipation intergral

    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    {
      SumKu.setZero();
      for(Uint j = 0; j < SHAPEFUNC::nb_nodes; ++j)
        SumKu += Ki_qn[q*QUADRATURE::nb_points+n] * ( U_n.row(n).transpose() - U_n.row(j).transpose() );

      typename PhysicsMT& KiPlus = Ki_qn[q*QUADRATURE::nb_points + n];

      Phi_n.row(n) += KiPlus * InvKi_qn * SumKu * wj[q];
    }
#endif


#endif // CF_RDM_SchemeCSysLDA_hpp
