// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CSchemeLDAT_hpp
#define CF_Solver_CSchemeLDAT_hpp

#include <boost/assign.hpp>

#include "Common/OptionT.hpp"
#include "Common/BasicExceptions.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/ElementType.hpp"

#include "Actions/CLoopOperation.hpp"

#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE>
class RDM_API CSchemeLDAT : public Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CSchemeLDAT> Ptr;
  typedef boost::shared_ptr<CSchemeLDAT const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CSchemeLDAT ( const std::string& name );

  /// Virtual destructor
  virtual ~CSchemeLDAT() {};

  /// Get the class name
  static std::string type_name () { return "CSchemeLDAT<" + SHAPEFUNC::type_name() + ">"; }

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
      inverse_updatecoeff(geometry_elements.get_field_elements(op.properties()["InverseUpdateCoeff"].value<std::string>()).data()),
      // Assume coordinates and connectivity_table are the same for solution and residual (pretty safe)
      coordinates(geometry_elements.get_field_elements(op.properties()["SolutionField"].value<std::string>()).nodes().coordinates()),
      connectivity_table(geometry_elements.get_field_elements(op.properties()["SolutionField"].value<std::string>()).connectivity_table())
    { }
    Mesh::CTable<Real>& solution;
    Mesh::CTable<Real>& residual;
    Mesh::CTable<Real>& inverse_updatecoeff;
    Mesh::CTable<Real>& coordinates;
    Mesh::CTable<Uint>& connectivity_table;
  };

  boost::shared_ptr<LoopHelper> m_loop_helper;

  const QUADRATURE& m_quadrature;

};

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE>
void CSchemeLDAT<SHAPEFUNC, QUADRATURE>::create_loop_helper (Mesh::CElements& geometry_elements )
{
  if ( Mesh::IsElementType<SHAPEFUNC>()(geometry_elements.element_type()) )
    m_loop_helper.reset( new LoopHelper(geometry_elements , *this ) );
  else
   throw Common::BadValue ( FromHere() , "Tried to solve on elements with wrong type: [" + geometry_elements.full_path().string() + "]");
}

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE>
CSchemeLDAT<SHAPEFUNC,QUADRATURE>::CSchemeLDAT ( const std::string& name ) :
  CLoopOperation(name),
  m_quadrature( QUADRATURE::instance() )
{
  regist_typeinfo(this);

  properties()["brief"] = std::string("Element Loop component that computes the residual and update coefficient using the LDA scheme");
  properties()["description"] = std::string("Write here the full description of this component");

  m_properties.add_option< Common::OptionT<std::string> > ("SolutionField","Solution Field for calculation", "solution")->mark_basic();
  m_properties.add_option< Common::OptionT<std::string> > ("ResidualField","Residual Field updated after calculation", "residual")->mark_basic();
  m_properties.add_option< Common::OptionT<std::string> > ("InverseUpdateCoeff","Inverse update coefficient Field updated after calculation", "inverse_updatecoeff")->mark_basic();
}

/////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC,typename QUADRATURE>
void CSchemeLDAT<SHAPEFUNC, QUADRATURE>::execute()
{
  // inside element with index m_idx

  const Mesh::CTable<Uint>::ConstRow node_idx = m_loop_helper->connectivity_table[m_idx];
  typename SHAPEFUNC::NodeMatrixT nodes;
  fill(nodes, m_loop_helper->coordinates, m_loop_helper->connectivity_table[m_idx]);

  typename SHAPEFUNC::MappedGradientT mapped_grad; //Gradient of the shape functions in reference space
  typename SHAPEFUNC::ShapeFunctionsT shapefunc;     //Values of shape functions in reference space
  typename SHAPEFUNC::CoordsT grad_solution;
  typename SHAPEFUNC::CoordsT grad_x;
  typename SHAPEFUNC::CoordsT grad_y;
  Real denominator;
  RealVector nominator(SHAPEFUNC::nb_nodes);
  RealVector phi(SHAPEFUNC::nb_nodes);

  phi.setZero();

  for (Uint q=0; q<QUADRATURE::nb_points; ++q) //Loop over quadrature points
  {
    SHAPEFUNC::mapped_gradient( m_quadrature.coords.col(q), mapped_grad );
    SHAPEFUNC::shape_function ( m_quadrature.coords.col(q), shapefunc   );

    Real x=0;
    Real y=0;

    for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    {
      x += shapefunc[n] * nodes(n, XX);
      y += shapefunc[n] * nodes(n, YY);
    }

    grad_x.setZero();
    grad_y.setZero();

    // Compute the components of the Jacobian matrix representing the transformation
    // physical -> reference space
    for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    {
      grad_x[XX] += mapped_grad(XX,n) * nodes(n, XX);
      grad_x[YY] += mapped_grad(YY,n) * nodes(n, XX);
      grad_y[XX] += mapped_grad(XX,n) * nodes(n, YY);
      grad_y[YY] += mapped_grad(YY,n) * nodes(n, YY);
    }

    const Real jacobian = grad_x[XX]*grad_y[YY]-grad_x[YY]*grad_y[XX];

    //Compute the gradient of the solution in physical space
    grad_solution.setZero();
    denominator = 0;

    for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    {
      const Real dNdx = 1.0/jacobian * (  grad_y[YY]*mapped_grad(XX,n) - grad_y[XX]*mapped_grad(YY,n) );
      const Real dNdy = 1.0/jacobian * ( -grad_x[YY]*mapped_grad(XX,n) + grad_x[XX]*mapped_grad(YY,n) );

      grad_solution[XX] += dNdx*m_loop_helper->solution[node_idx[n]][0];
      grad_solution[YY] += dNdy*m_loop_helper->solution[node_idx[n]][0];

      nominator[n] = std::max(0.0,y*dNdx - x*dNdy);
      denominator += nominator[n];
    }


    const Real nablaF = (y*grad_solution[XX] - x*grad_solution[YY]);

    for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    {
      phi[n] += nominator[n]/denominator * nablaF * m_quadrature.weights[q] * jacobian;
    }

  }

  // Loop over quadrature nodes

  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    m_loop_helper->residual[node_idx[n]][0] += phi[n];

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
    m_loop_helper->inverse_updatecoeff[node_idx[n]][0] +=
        std::sqrt( dx*dx+dy*dy) *
        std::sqrt( centroid[XX]*centroid[XX] + centroid[YY]*centroid[YY] );
  }


}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_CSchemeLDAT_hpp
