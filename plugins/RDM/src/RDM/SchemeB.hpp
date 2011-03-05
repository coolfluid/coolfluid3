// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_SchemeB_hpp
#define CF_Solver_SchemeB_hpp

#include <boost/assign.hpp>

#include <Eigen/Dense>

#include "Common/Core.hpp"
#include "Common/OptionT.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/ComponentPredicates.hpp"

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

template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
class RDM_API SchemeB : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr< SchemeB > Ptr;
  typedef boost::shared_ptr< SchemeB const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  SchemeB ( const std::string& name );

  /// Virtual destructor
  virtual ~SchemeB() {};

  /// Get the class name
  static std::string type_name () { return "SchemeB<" + SHAPEFUNC::type_name() + ">"; }

  /// execute the action
  virtual void execute ();
    
private: // helper functions

  void trigger_elements()
  {
    /// @todo improve this (ugly)

    connectivity_table = elements().as_ptr<Mesh::CElements>()->connectivity_table().self()->as_ptr< Mesh::CTable<Uint> >();
    coordinates = elements().nodes().coordinates().self()->as_ptr< Mesh::CTable<Real> >();

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

  typedef FluxOp2D<SHAPEFUNC,QUADRATURE,PHYSICS> DiscreteOpType;

  DiscreteOpType m_oper;

  //Values of the solution located in the dof of the element
  typename DiscreteOpType::SolutionMatrixT m_solution_values;

  //The operator L in the advection equation Lu = f
  //Matrix m_sf_oper_values stores the value L(N_i) at each quadrature point for each shape function N_i
  typename DiscreteOpType::SFMatrixT m_sf_oper_values;

  //Values of the operator L(u) computed in quadrature points. These operator L returns these values
  //multiplied by Jacobian and quadrature weight
  
  RealVector m_flux_oper_values;

  //Nodal residuals
  RealVector m_phi;

  //CrossWind Dissipation term for N scheme
  RealVector m_diss;

  //Integration factor (jacobian multiplied by quadrature weight)
  Eigen::Matrix<Real, QUADRATURE::nb_points, 1u> m_wj;
};

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
SchemeB<SHAPEFUNC,QUADRATURE,PHYSICS>::SchemeB ( const std::string& name ) :
  CLoopOperation(name)
{
  regist_typeinfo(this);

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &SchemeB<SHAPEFUNC,QUADRATURE,PHYSICS>::trigger_elements,   this ) );

  m_flux_oper_values.resize(QUADRATURE::nb_points);
  m_phi.resize(SHAPEFUNC::nb_nodes);
  m_diss.resize(SHAPEFUNC::nb_nodes);

}

/////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC,typename QUADRATURE, typename PHYSICS>
void SchemeB<SHAPEFUNC, QUADRATURE, PHYSICS>::execute()
{
  // inside element with index m_idx

  const Mesh::CTable<Uint>::ConstRow node_idx = connectivity_table->array()[idx()];
  typename SHAPEFUNC::NodeMatrixT nodes;

  Mesh::fill(nodes, *coordinates, node_idx);

  for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    m_solution_values[n] = (*solution)[node_idx[n]][0];


 m_phi.setZero(); 
 m_diss.setZero();

 m_oper.compute(nodes,m_solution_values, m_sf_oper_values, m_flux_oper_values,m_wj);

 /// THE LOOP IMPLEMENTS THE INTEGRAL
 /// phiN_i = phiLDA_i + integral[ kplus_i * frac{ sum( kplus_j(u_i - u_out) ) }{ sum(kplus_j) }    ] dX

 for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
 {
   Real sumLplus = 0.0;
   Real u_out = 0.0;

   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   {
     sumLplus += std::max(0.0,m_sf_oper_values(q,n));
   }

   /// phiLDA:
   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   {
     m_phi[n] += std::max(0.0,m_sf_oper_values(q,n))/sumLplus * m_flux_oper_values[q]*m_wj[q];
   }

   Real sum_kdiff_u;

   /// dissipative part:
   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   {

    sum_kdiff_u = 0.0;

    for(Uint l = 0; l < SHAPEFUNC::nb_nodes; ++l)
    {
      sum_kdiff_u += std::max(0.0,m_sf_oper_values(q,l)) * ( m_solution_values[n] - m_solution_values[l] );
    }
     m_diss[n] += 1./sumLplus * std::max(0.0,m_sf_oper_values(q,n)) * sum_kdiff_u * m_wj[q];
   }
 }

 Real sum_phi = 0.0;
 Real sum_phi_N = 0.0;

 for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
 {
   sum_phi += m_phi[n] + m_diss[n];
 }

 for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
 {
   sum_phi_N += std::abs(m_diss[n]);
 }

 const Real eps = 1.e-14;

 const Real theta = sum_phi_N > eps ? std::abs(sum_phi)/sum_phi_N : 0.0;

  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    (*residual)[node_idx[n]][0] += m_phi[n] + theta * m_diss[n];


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
    Real k_plus_max = 0.0;

    for (Uint q  = 0; q<QUADRATURE::nb_points; ++q)
    {
      k_plus_max = std::max(k_plus_max,m_sf_oper_values(q,n));
    }

    (*wave_speed)[node_idx[n]][0] +=
        std::sqrt( dx*dx+dy*dy) * k_plus_max;
  }


}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_SchemeB_hpp
