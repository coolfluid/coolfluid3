// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CSchemeN_hpp
#define CF_Solver_CSchemeN_hpp

#include <boost/assign.hpp>

#include "Common/OptionT.hpp"
#include "Common/BasicExceptions.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/ElementType.hpp"

#include "Solver/Actions/CLoopOperation.hpp"

#include "RDM/LibRDM.hpp"
#include "RDM/FluxOp2D.hpp"
#include "RDM/RotationAdv2D.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE>
class RDM_API CSchemeN : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr< CSchemeN > Ptr;
  typedef boost::shared_ptr< CSchemeN const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CSchemeN ( const std::string& name );

  /// Virtual destructor
  virtual ~CSchemeN() {};

  /// Get the class name
  static std::string type_name () { return "CSchemeN<" + SHAPEFUNC::type_name() + ">"; }

  /// Set the loop_helper
  void create_loop_helper (Mesh::CElements& geometry_elements );
	
  /// execute the action
  virtual void execute ();
    
private: // data

  struct LoopHelper
  {
    LoopHelper(Mesh::CElements& geometry_elements, CLoopOperation& op) :
			solution(geometry_elements.get_field_elements(op.properties()["SolutionField"].value<std::string>()).data()),
      residual(geometry_elements.get_field_elements(op.properties()["ResidualField"].value<std::string>()).data()),
      wave_speed(geometry_elements.get_field_elements(op.properties()["InverseUpdateCoeff"].value<std::string>()).data()),
      // Assume coordinates and connectivity_table are the same for solution and residual (pretty safe)
      coordinates(geometry_elements.get_field_elements(op.properties()["SolutionField"].value<std::string>()).nodes().coordinates()),
      connectivity_table(geometry_elements.get_field_elements(op.properties()["SolutionField"].value<std::string>()).connectivity_table())
    { }
    Mesh::CTable<Real>& solution;
    Mesh::CTable<Real>& residual;
    Mesh::CTable<Real>& wave_speed;
    Mesh::CTable<Real>& coordinates;
    Mesh::CTable<Uint>& connectivity_table;
  };

  boost::shared_ptr<LoopHelper> m_loop_helper;

  typedef FluxOp2D<SHAPEFUNC,QUADRATURE,RotationAdv2D> DiscreteOpType;

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
};

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE>
void CSchemeN<SHAPEFUNC, QUADRATURE>::create_loop_helper (Mesh::CElements& geometry_elements )
{
  if ( Mesh::IsElementType<SHAPEFUNC>()(geometry_elements.element_type()) )
    m_loop_helper.reset( new LoopHelper(geometry_elements , *this ) );
  else
   throw Common::BadValue ( FromHere() , "Tried to solve on elements with wrong type: [" + geometry_elements.full_path().string() + "]");
}

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE>
CSchemeN<SHAPEFUNC,QUADRATURE>::CSchemeN ( const std::string& name ) :
  CLoopOperation(name)
{
  regist_typeinfo(this);

  properties()["brief"] = std::string("Element Loop component that computes the residual and update coefficient using the LDA scheme");
  properties()["description"] = std::string("Write here the full description of this component");

  m_properties.add_option< Common::OptionT<std::string> > ("SolutionField","Solution Field for calculation", "solution")->mark_basic();
  m_properties.add_option< Common::OptionT<std::string> > ("ResidualField","Residual Field updated after calculation", "residual")->mark_basic();
  m_properties.add_option< Common::OptionT<std::string> > ("InverseUpdateCoeff","Inverse update coefficient Field updated after calculation", "wave_speed")->mark_basic();

  m_solution_values.resize(SHAPEFUNC::nb_nodes);
  m_flux_oper_values.resize(QUADRATURE::nb_points);
  m_phi.resize(SHAPEFUNC::nb_nodes);
  m_diss.resize(SHAPEFUNC::nb_nodes);

}

/////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC,typename QUADRATURE>
void CSchemeN<SHAPEFUNC, QUADRATURE>::execute()
{
  // inside element with index m_idx

  const Mesh::CTable<Uint>::ConstRow node_idx = m_loop_helper->connectivity_table[idx()];
  typename SHAPEFUNC::NodeMatrixT nodes;
  fill(nodes, m_loop_helper->coordinates, m_loop_helper->connectivity_table[idx()]);

  for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    m_solution_values[n] = m_loop_helper->solution[node_idx[n]][0];


 m_phi.setZero(); 
 m_diss.setZero();

 m_oper.compute(nodes,m_solution_values, m_sf_oper_values, m_flux_oper_values);

 for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
 {
   Real sumLplus = 0.0;
   Real u_out = 0.0;

   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   {
     sumLplus += std::max(0.0,m_sf_oper_values(q,n));     
   }

   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   {
     m_phi[n] += std::max(0.0,m_sf_oper_values(q,n))/sumLplus * m_flux_oper_values[q]; 
     u_out    += (1./sumLplus) * std::max(0.0,m_sf_oper_values(q,n)) * m_solution_values[n];
   }

   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   {
     m_diss[n] += std::max(0.0,m_sf_oper_values(q,n)) * ( m_solution_values[n] - u_out );
   }
 }

  

  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    m_loop_helper->residual[node_idx[n]][0] += m_phi[n] + m_diss[n];

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
    m_loop_helper->wave_speed[node_idx[n]][0] +=
        std::sqrt( dx*dx+dy*dy) *
        std::sqrt( centroid[XX]*centroid[XX] + centroid[YY]*centroid[YY] );
  }


}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_CSchemeN_hpp
