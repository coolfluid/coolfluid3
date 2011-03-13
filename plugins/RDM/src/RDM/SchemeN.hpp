// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_SchemeN_hpp
#define CF_Solver_SchemeN_hpp

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

template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
class RDM_API SchemeN : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr< SchemeN > Ptr;
  typedef boost::shared_ptr< SchemeN const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  SchemeN ( const std::string& name );

  /// Virtual destructor
  virtual ~SchemeN() {};

  /// Get the class name
  static std::string type_name () { return "SchemeN<" + SHAPEFUNC::type_name() + ">"; }

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
SchemeN<SHAPEFUNC,QUADRATURE,PHYSICS>::SchemeN ( const std::string& name ) :
  CLoopOperation(name)
{
  regist_typeinfo(this);

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &SchemeN<SHAPEFUNC,QUADRATURE,PHYSICS>::trigger_elements,   this ) );

  m_flux_oper_values.resize(QUADRATURE::nb_points);
  m_phi.resize(SHAPEFUNC::nb_nodes);
  m_diss.resize(SHAPEFUNC::nb_nodes);

}

/////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC,typename QUADRATURE, typename PHYSICS>
void SchemeN<SHAPEFUNC, QUADRATURE, PHYSICS>::execute()
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

 /// VERSION A: THE LOOP IMPLEMENTS THE INTEGRAL
 /// phiN_i = phiLDA_i + integral[ kplus_i * frac{ sum( kplus_j(u_i - u_out) ) }{ sum(kplus_j) }    ] dX
 /// More computing is done than in version b, but the integral is split into LDA + dissipation

/*

 for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
 {
   Real sumLplus = 0.0;

   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   {
     sumLplus += std::max(0.0,m_sf_oper_values(q,n));
   }

   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   {
     m_phi[n] += std::max(0.0,m_sf_oper_values(q,n))/sumLplus * m_flux_oper_values[q]*m_wj[q];
   }

   Real sum_kdiff_u;

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

  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    (*residual)[node_idx[n]][0] += m_phi[n] + m_diss[n];
*/

 /// VERSION B: THE LOOP IMPLEMENTS THE INTEGRAL phi_i = integral{ kplus_i * (u_i - u_out) } dX
 /// No distinction between the 'LDA' and 'dissipative' components of the residual is made
 /// The variable 'm_diss' is not used


 for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
 {
   Real sumLplus = 0.0;

   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   {
     sumLplus += std::max(0.0,m_sf_oper_values(q,n));
   }

   Real u_out = -m_flux_oper_values[q];

   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   {
     u_out += std::max(0.0,m_sf_oper_values(q,n)) * m_solution_values[n];
   }

   u_out /= sumLplus;

   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   {
     m_phi[n] += std::max(0.0,m_sf_oper_values(q,n)) * (m_solution_values[n] - u_out) *m_wj[q];
   }
 }
  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    (*residual)[node_idx[n]][0] += m_phi[n];


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
    (*wave_speed)[node_idx[n]][0] +=
        std::sqrt( dx*dx+dy*dy) *
        std::sqrt( centroid[XX]*centroid[XX] + centroid[YY]*centroid[YY] );
  }


}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_SchemeN_hpp
