// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_SchemeLDA_hpp
#define CF_Solver_SchemeLDA_hpp

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
#include "RDM/FluxOp2D.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template < typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS >
class RDM_API SchemeLDA : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr< SchemeLDA > Ptr;
  typedef boost::shared_ptr< SchemeLDA const> ConstPtr;

  /// type of the helper object to compute the physical operator Lu
  typedef FluxOp2D<SHAPEFUNC,QUADRATURE,PHYSICS> DiscreteOpType;

public: // functions

  /// Contructor
  /// @param name of the component
  SchemeLDA ( const std::string& name );

  /// Virtual destructor
  virtual ~SchemeLDA() {};

  /// Get the class name
  static std::string type_name () { return "SchemeLDA<" + SHAPEFUNC::type_name() + ">"; }
	
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
  typename DiscreteOpType::SolutionMatrixT m_solution_nd;
  /// Values of the operator L(u) computed in quadrature points.
  typename DiscreteOpType::ResidualMatrixT m_Lu_qd;
  /// Nodal residuals
  typename DiscreteOpType::SolutionMatrixT m_phi;
  /// The operator L in the advection equation Lu = f
  /// Matrix m_sf_qd stores the value L(N_i) at each quadrature point for each shape function N_i
  typename DiscreteOpType::SFMatrixT m_sf_qd;
  /// node values
  typename SHAPEFUNC::NodeMatrixT m_nodes;

  /// Integration factor (jacobian multiplied by quadrature weight)
  Eigen::Matrix<Real, QUADRATURE::nb_points, 1u> m_wj;

  typename DiscreteOpType::SolutionVectorT sumLNplus;
};

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
SchemeLDA<SHAPEFUNC,QUADRATURE,PHYSICS>::SchemeLDA ( const std::string& name ) :
  CLoopOperation(name)
{
  regist_typeinfo(this);

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &SchemeLDA<SHAPEFUNC,QUADRATURE,PHYSICS>::change_elements, this ) );

  std::cout << "m_sf_qd       is " << m_sf_qd.rows() << "x" << m_sf_qd.cols() << std::endl;
  std::cout << "m_Lu_qd       is " << m_Lu_qd.rows() << "x" << m_Lu_qd.cols() << std::endl;
  std::cout << "m_phi         is " << m_phi.rows()   << "x" << m_phi.cols() << std::endl;
  std::cout << "m_solution_nd is " << m_solution_nd.rows() << "x" << m_solution_nd.cols() << std::endl;

}

/////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC,typename QUADRATURE, typename PHYSICS>
void SchemeLDA<SHAPEFUNC, QUADRATURE,PHYSICS>::execute()
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

  // copy the solution from the large array to a small

  for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    for (Uint v=0; v < PHYSICS::nb_eqs; ++v)
      m_solution_nd(n,v) = (*solution)[ nodes_idx[n] ][v];

  m_phi.setZero(); // reset element residuals

//  std::cout << "solution: " << m_solution_nd << std::endl;
//  std::cout << "phi: " << m_phi << std::endl;

  // compute L(u) and L(N) @ each quadrature point

  m_oper.compute(m_nodes, m_solution_nd, m_sf_qd, m_Lu_qd, m_wj);



  std::cout << "solution_values  [" << m_solution_nd << "]" << std::endl;
  std::cout << std::endl;
  std::cout << "sf_oper_values " <<  m_sf_qd << std::endl;
  std::cout << std::endl;
  std::cout << "flux_oper_values " << m_Lu_qd.rows() << "x" << m_Lu_qd.cols()  << " [" << m_Lu_qd << "]" << std::endl;

  exit(0);

  //// if (m_Lu_qd.norm() > 0.0)
  ////     std::cin.get();
  // std::cout << std::endl;

  // compute L(N)+


  for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
  {
    Real sumLplus = 0;

    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
       sumLplus += std::max(0.0, m_sf_qd(q,n) );

    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
        m_phi(n,0) += std::max(0.0,m_sf_qd(q,n)) / sumLplus * m_Lu_qd[q] * m_wj[q];
  }

//     std::cout << "phi [";
//     for (Uint n=0; n < SHAPEFUNC::nb_nodes; ++n)
//       for (Uint v=0; v < PHYSICS::nb_eqs; ++v)
//        std::cout << m_phi(n,v) << " ";
//     std::cout << "]" << std::endl;

  // update the residual
  
  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
      (*residual)[nodes_idx[n]][0] += m_phi(n,0);

  // computing average advection speed on element

	typename SHAPEFUNC::CoordsT centroid;
	
	centroid.setZero();

  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
  {
    centroid[XX] += m_nodes(n, XX);
    centroid[YY] += m_nodes(n, YY);
  }
  centroid /= SHAPEFUNC::nb_nodes;


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

  // The update coeff is updated by a product of bb radius and norm of advection velocity
  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
  {
    (*wave_speed)[nodes_idx[n]][0] += std::sqrt( dx*dx+dy*dy);
    //       * std::sqrt( centroid[XX]*centroid[XX] + centroid[YY]*centroid[YY] );
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_SchemeLDA_hpp
