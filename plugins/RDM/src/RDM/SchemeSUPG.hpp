// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_SchemeSUPG_hpp
#define CF_Solver_SchemeSUPG_hpp

#include <boost/assign.hpp>

#include <Eigen/Dense>

#include "Common/Core.hpp"
#include "Common/OptionT.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/FindComponentss.hpp"

#include "Mesh/ElementData.hpp"
#include "Mesh/CField2.hpp"
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
class RDM_API SchemeSUPG : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr< SchemeSUPG > Ptr;
  typedef boost::shared_ptr< SchemeSUPG const> ConstPtr;

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

    connectivity_table = elements().as_ptr<Mesh::CElements>()->connectivity_table().as_ptr< Mesh::CTable<Uint> >();
    coordinates = elements().nodes().coordinates().as_ptr< Mesh::CTable<Real> >();

    cf_assert( is_not_null(connectivity_table) );

    /// @todo modify these to option components configured from

    Mesh::CField2::Ptr csolution = Common::find_component_ptr_recursively_with_tag<Mesh::CField2>( *Common::Core::instance().root(), "solution" );
    cf_assert( is_not_null( csolution ) );
    solution = csolution->data_ptr();

    Mesh::CField2::Ptr cresidual = Common::find_component_ptr_recursively_with_tag<Mesh::CField2>( *Common::Core::instance().root(), "residual" );
    cf_assert( is_not_null( cresidual ) );
    residual = cresidual->data_ptr();

    Mesh::CField2::Ptr cwave_speed = Common::find_component_ptr_recursively_with_tag<Mesh::CField2>( *Common::Core::instance().root(), "wave_speed" );
    cf_assert( is_not_null( cwave_speed ) );
    wave_speed = cwave_speed->data_ptr();
  }


private: // data

  Mesh::CTable<Uint>::Ptr connectivity_table;

  Mesh::CTable<Real>::Ptr coordinates;

  Mesh::CTable<Real>::Ptr solution;
  Mesh::CTable<Real>::Ptr residual;
  Mesh::CTable<Real>::Ptr wave_speed;


  //Remark: numerical quadrature and shape function values have to be
  //redefined in this class even though they were already computed in FluxOp2D
  //Reason: FluxOp2D holds the values of all sf. at all quadrature points privately,
  //and we need them here to define the values of SUPG test functions

  const QUADRATURE& m_quadrature;

  //Values of all shape functions at all quadrature points
  Eigen::Matrix<Real,QUADRATURE::nb_points, SHAPEFUNC::nb_nodes> m_N;


  typedef FluxOp2D<SHAPEFUNC,QUADRATURE,PHYSICS> DiscreteOpType;

  DiscreteOpType m_oper;

  // Values of the solution located in the dof of the element
  // RealVector m_solution_values;
  typename DiscreteOpType::SolutionMatrixT m_solution_values;

  // The operator L in the advection equation Lu = f
  // Matrix m_sf_oper_values stores the value L(N_i) at each quadrature point for each shape function N_i
  typename DiscreteOpType::SFMatrixT m_sf_oper_values;

  // Values of the operator L(u) computed in quadrature points. These operator L returns these values
  // multiplied by Jacobian and quadrature weight
  RealVector m_flux_oper_values;

  // Nodal residuals
  RealVector m_phi;

  //Integration factor (jacobian multiplied by quadrature weight)
  Eigen::Matrix<Real, QUADRATURE::nb_points, 1u> m_wj;

};

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
SchemeSUPG<SHAPEFUNC,QUADRATURE,PHYSICS>::SchemeSUPG ( const std::string& name ) :
  CLoopOperation(name),
  m_quadrature( QUADRATURE::instance() )
{
  regist_typeinfo(this);

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &SchemeSUPG<SHAPEFUNC,QUADRATURE,PHYSICS>::change_elements, this ) );

  m_flux_oper_values.resize(QUADRATURE::nb_points);
  m_phi.resize(SHAPEFUNC::nb_nodes);


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

 typename SHAPEFUNC::NodeMatrixT nodes;

 Mesh::fill(nodes, *coordinates, nodes_idx );

 for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   m_solution_values[n] = (*solution)[nodes_idx[n]][0];


 m_phi.setZero();

 m_oper.compute(nodes,m_solution_values, m_sf_oper_values, m_flux_oper_values,m_wj);


 // computing average advection speed on element

 typename SHAPEFUNC::CoordsT centroid;

 centroid.setZero();

 for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
 {
   centroid[XX] += nodes(n, XX);
   centroid[YY] += nodes(n, YY);
 }
 centroid /= SHAPEFUNC::nb_nodes;


 // compute a bounding box of the element:

 Real xmin = nodes(0, XX);
 Real xmax = nodes(0, XX);
 Real ymin = nodes(0, YY);
 Real ymax = nodes(0, YY);

 for(Uint inode = 1; inode < SHAPEFUNC::nb_nodes; ++inode)
 {
   xmin = std::min(xmin,nodes(inode, XX));
   xmax = std::max(xmax,nodes(inode, XX));

   ymin = std::min(ymin,nodes(inode, YY));
   ymax = std::max(ymax,nodes(inode, YY));

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
     m_phi[n] += ( m_N(q,n) + tau_stab * m_sf_oper_values(q,n) ) * m_flux_oper_values[q] * m_wj[q];
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
