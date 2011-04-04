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

#include "Mesh/ElementData.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/ElementType.hpp"

#include "Solver/Actions/CLoopOperation.hpp"

#include "RDM/LibRDM.hpp"
#include "RDM/CSysFluxOp2D.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template < typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS >
class RDM_API SchemeCSysLDA : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr< SchemeCSysLDA > Ptr;
  typedef boost::shared_ptr< SchemeCSysLDA const> ConstPtr;

  /// type of the helper object to compute the physical operator Lu
  typedef CSysFluxOp2D<SHAPEFUNC,QUADRATURE,PHYSICS> DiscreteOpType;

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

  /// helper object to compute the physical operator Lu
  DiscreteOpType m_oper;

  /// Values of the solution located in the dof of the element
  typename DiscreteOpType::SolutionMT U_n;
  /// Values of the operator L(u) computed in quadrature points.
  typename DiscreteOpType::PhysicsVT LU_q [QUADRATURE::nb_points];
  /// Nodal residuals
  typename DiscreteOpType::SolutionMT Phi_n;
  /// The operator L in the advection equation Lu = f
  /// Matrix Ki_qn stores the value L(N_i) at each quadrature point for each shape function N_i
  typename DiscreteOpType::PhysicsMT Ki_qn [QUADRATURE::nb_points*SHAPEFUNC::nb_nodes];
  /// inverse Ki+ matix
  typename DiscreteOpType::PhysicsMT InvKi_qn;
  /// diagonal matrix with positive eigen values
  typename DiscreteOpType::PhysicsMT DvPlus [QUADRATURE::nb_points*SHAPEFUNC::nb_nodes];
  /// inflow vector for N dissipation
  typename DiscreteOpType::PhysicsVT SumKu;
  /// node values
  typename SHAPEFUNC::NodeMatrixT m_nodes;

  /// Integration factor (jacobian multiplied by quadrature weight)
  Eigen::Matrix<Real, QUADRATURE::nb_points, 1u> m_wj;

  typename DiscreteOpType::PhysicsMT sumLplus;
};

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
SchemeCSysLDA<SHAPEFUNC,QUADRATURE,PHYSICS>::SchemeCSysLDA ( const std::string& name ) :
  CLoopOperation(name)
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
}

/////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC,typename QUADRATURE, typename PHYSICS>
void SchemeCSysLDA<SHAPEFUNC, QUADRATURE,PHYSICS>::execute()
{
//  std::cout << "ELEM [" << idx() << "]" << std::endl;

  // copy the coordinates from the large array to a small

  const Mesh::CTable<Uint>::ConstRow nodes_idx = connectivity_table->array()[idx()];

//  std::cout << "nodes_idx";
//  for ( Uint i = 0; i < nodes_idx.size(); ++i)
//     std::cout << " " << nodes_idx[i];

  Mesh::fill(m_nodes, *coordinates, nodes_idx );

//  std::cout << "mesh::fill function" <<  std::endl;
//  std::cout << "nodes: " << m_nodes << std::endl;

  // compute a bounding box of the element:

  Real xmin = m_nodes(0, XX);
  Real xmax = m_nodes(0, XX);
  Real ymin = m_nodes(0, YY);
  Real ymax = m_nodes(0, YY);

  for(Uint inode = 1; inode < SHAPEFUNC::nb_nodes; ++inode)
  {
    xmin = std::min(xmin,m_nodes(inode, XX));
    xmax = std::max(xmax,m_nodes(inode, XX));

    ymin = std::min(ymin,m_nodes(inode, YY));
    ymax = std::max(ymax,m_nodes(inode, YY));

  }

  const Real dx = xmax - xmin;
  const Real dy = ymax - ymin;
	const Real bb_area = std::sqrt( dx*dx+dy*dy);

  // copy the solution from the large array to a small

  for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    for (Uint v=0; v < PHYSICS::nb_eqs; ++v)
      U_n(n,v) = (*solution)[ nodes_idx[n] ][v];

  Phi_n.setZero(); // reset element residuals

//  std::cout << "solution: " << U_n << std::endl;
//  std::cout << "phi: " << Phi_n << std::endl;

  // compute L(u) and L(N) @ each quadrature point

  m_oper.compute(m_nodes, U_n, Ki_qn, LU_q, DvPlus, m_wj);

//  std::cout << "solution_values  [" << U_n << "]" << std::endl;
//  std::cout << std::endl;
//  std::cout << "sf_oper_values " << std::endl;
//  for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
//      std::cout << "[ " << Ki_qn[q] << " ]" << std::endl;
//  std::cout << std::endl;
//  std::cout << "flux_oper_values " << LU_q.rows() << "x" << LU_q.cols()  << " [" << LU_q << "]" << std::endl;

  Real elem_area = 0;
  Real elem_wave_speed = 0.; // elemement area times max eigen value

  // compute L(N)+]

  for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
  {
    elem_area += m_wj[q];

    sumLplus.setZero();

    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
      sumLplus += Ki_qn[q*QUADRATURE::nb_points + n];

    // invert the sum L plus

    InvKi_qn = sumLplus.inverse();

    // compute the phi_i LDA intergral

    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    {
      typename DiscreteOpType::PhysicsMT& KiPlus = Ki_qn[q*QUADRATURE::nb_points + n];

      Phi_n.row(n) +=  KiPlus * InvKi_qn * LU_q[q] * m_wj[q];
    }

    // compute the phi_i N dissipation intergral
#if 0
    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    {
      SumKu.setZero();
      for(Uint j = 0; j < SHAPEFUNC::nb_nodes; ++j)
        SumKu += Ki_qn[q*QUADRATURE::nb_points+n] * ( U_n.row(n).transpose() - U_n.row(j).transpose() );

      typename DiscreteOpType::PhysicsMT& KiPlus = Ki_qn[q*QUADRATURE::nb_points + n];

      Phi_n.row(n) += KiPlus * InvKi_qn * SumKu * m_wj[q];
    }
#endif

    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    {
      Real max_eigen_value = 0;
      for (Uint v=0; v < PHYSICS::nb_eqs; ++v)
        max_eigen_value = std::max( max_eigen_value, DvPlus[q*QUADRATURE::nb_points + n](v,v) );

      (*wave_speed)[nodes_idx[n]][0] += max_eigen_value * m_wj[q];
    }

	} // loop qd points

//  std::cout << "phi [";
//  for (Uint n=0; n < SHAPEFUNC::nb_nodes; ++n)
//    for (Uint v=0; v < PHYSICS::nb_eqs; ++v)
//      std::cout << Phi_n(n,v) << " ";
//  std::cout << "]" << std::endl;

  // update the residual
  
  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    for (Uint v=0; v < PHYSICS::nb_eqs; ++v)
      (*residual)[nodes_idx[n]][v] += Phi_n(n,v);
}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_SchemeCSysLDA_hpp
