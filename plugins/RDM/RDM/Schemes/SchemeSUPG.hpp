// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_SchemeSUPG_hpp
#define CF_RDM_SchemeSUPG_hpp

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
class RDM_SCHEMES_API SchemeSUPG : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr< SchemeSUPG > Ptr;
  typedef boost::shared_ptr< SchemeSUPG const> ConstPtr;

  /// type of the helper object to compute the physical operator Lu
  typedef FluxOp2D<SHAPEFUNC,QUADRATURE,PHYSICS> DiscreteOpType;

public: // functions

  /// Contructor
  /// @param name of the component
  SchemeSUPG ( const std::string& name );

  /// Virtual destructor
  virtual ~SchemeSUPG() {};

  /// Get the class name
  static std::string type_name () { return "SchemeSUPG<" + SHAPEFUNC::type_name() + ">"; }
	
  /// execute the action
  virtual void execute ();

private: // helper functions

  void change_elements()
  { 
    /// @todo improve this (ugly)

    connectivity_table = elements().as_ptr<Mesh::CElements>()->node_connectivity().as_ptr< Mesh::CTable<Uint> >();
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

  /// Remark: numerical quadrature and shape function values have to be
  /// redefined in this class even though they were already computed in FluxOp2D
  /// Reason: FluxOp2D holds the values of all sf. at all quadrature points privately,
  /// and we need them here to define the values of SUPG test functions

  /// Values of all shape functions at all quadrature points
  typename DiscreteOpType::SFMatrixT m_N;

  const QUADRATURE& m_quadrature;

};

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
SchemeSUPG<SHAPEFUNC,QUADRATURE,PHYSICS>::SchemeSUPG ( const std::string& name ) :
  CLoopOperation(name),
  m_quadrature( QUADRATURE::instance() )
{
  regist_typeinfo(this);

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &SchemeSUPG<SHAPEFUNC,QUADRATURE,PHYSICS>::change_elements, this ) );

  //Compute values of shape functions at quadrature points:

  typename SHAPEFUNC::ShapeFunctionsT sf_ref;

  for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
  {
    for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    {
       SHAPEFUNC::shape_function ( m_quadrature.coords.col(q), sf_ref   );
       m_N(q,n) = sf_ref[n];
    }
  }


}

/////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC,typename QUADRATURE, typename PHYSICS>
void SchemeSUPG<SHAPEFUNC, QUADRATURE,PHYSICS>::execute()
{

 const Mesh::CTable<Uint>::ConstRow nodes_idx = connectivity_table->array()[idx()];

 Mesh::fill(m_nodes, *coordinates, nodes_idx );

 for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   m_solution_nd[n] = (*solution)[nodes_idx[n]][0];


 m_phi.setZero();

 m_oper.compute(m_nodes,m_solution_nd, m_sf_qd, m_Lu_qd,m_wj);


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


 //Characteristic length of element:
 const Real h = std::sqrt(dx*dy);

 //Stabilization parameter:
 const Real tau_stab = 0.5*h/std::sqrt(centroid[XX]*centroid[XX]+centroid[YY]*centroid[YY]);



 for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
 {
   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   {
     m_phi[n] += ( m_N(q,n) + tau_stab * m_sf_qd(q,n) ) * m_Lu_qd[q] * m_wj[q];
   }
 }

 for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
   (*residual)[nodes_idx[n]][0] += m_phi[n];


}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_SchemeSUPG_hpp
