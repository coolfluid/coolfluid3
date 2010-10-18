// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"
#include "Common/OptionT.hpp"

#include "Math/RealMatrix.hpp"
#include "Math/MathChecks.hpp"

#include "Mesh/ElementNodes.hpp"
#include "Mesh/SF/Triag2DLagrangeP1.hpp"
#include "Actions/CSchemeLDA.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh::SF;
namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < CSchemeLDA, CAction, LibActions, NB_ARGS_1 > CSchemeLDAProviderCAction( "CSchemeLDA" );
Common::ObjectProvider < CSchemeLDA, CLoopOperation, LibActions, NB_ARGS_1 > CSchemeLDAProviderCLoopOperation( "CSchemeLDA" );

///////////////////////////////////////////////////////////////////////////////////////
  
void CSchemeLDA::defineConfigProperties( Common::PropertyList& options )
{
  options.add_option< OptionT<std::string> > ("SolutionField","Solution Field for calculation", "solution")->mark_basic();
  options.add_option< OptionT<std::string> > ("ResidualField","Residual Field updated after calculation", "residual")->mark_basic();
  options.add_option< OptionT<std::string> > ("InverseUpdateCoeff","Inverse update coefficient Field updated after calculation", "inv_updateCoeff")->mark_basic();
}

///////////////////////////////////////////////////////////////////////////////////////

void CSchemeLDA::set_loophelper (CElements& geometry_elements )
{
	data = boost::shared_ptr<LoopHelper> ( new LoopHelper(geometry_elements , *this ) );
}
	
///////////////////////////////////////////////////////////////////////////////////////

CSchemeLDA::CSchemeLDA ( const CName& name ) : 
  CLoopOperation(name)
{
  BUILD_COMPONENT;
	properties()["brief"] = std::string("Element Loop component that computes the residual and update coefficient using the LDA scheme");
	properties()["description"] = std::string("Write here the full description of this component");
  
  nb_q=3;
  mapped_coords.resize(nb_q);
  for(Uint q=0; q<nb_q; ++q)
    mapped_coords[q].resize(2);
    
  mapped_coords[0][0] = 0.5;  mapped_coords[0][1] = 0.0;
  mapped_coords[1][0] = 0.5;  mapped_coords[1][1] = 0.5;
  mapped_coords[2][0] = 0.0;  mapped_coords[2][1] = 0.5;
  w = 1./6.;
}

/////////////////////////////////////////////////////////////////////////////////////

void CSchemeLDA::execute()
{
	// inside element with index m_idx

	const CTable::ConstRow node_idx = data->connectivity_table[m_idx];
  ConstElementNodeView nodes (data->coordinates, data->connectivity_table[m_idx]);

  RealMatrix mapped_grad(2,3); //Gradient of the shape functions in reference space
  RealVector shapefunc(3);     //Values of shape functions in reference space
  RealVector grad_solution(2);
  RealVector grad_x(2);
  RealVector grad_y(2);
  Real denominator;
  RealVector nominator(3);
  RealVector phi(3);

  phi = 0.;
      
  for (Uint q=0; q<nb_q; ++q) //Loop over quadrature points
  {
    Triag2DLagrangeP1::mapped_gradient(mapped_coords[q],mapped_grad);
    Triag2DLagrangeP1::shape_function(mapped_coords[q], shapefunc);
    
    Real x=0;
    Real y=0;

		for (Uint n=0; n<3; ++n)
		{
			x += shapefunc[n] * nodes[n][XX];
      y += shapefunc[n] * nodes[n][YY];
		}
    
    grad_x = 0;
    grad_y = 0;

    // Compute the components of the Jacobian matrix representing the transformation
    // physical -> reference space
		for (Uint n=0; n<3; ++n)
    {
      grad_x[XX] += mapped_grad(XX,n) * nodes[n][XX];
      grad_x[YY] += mapped_grad(YY,n) * nodes[n][XX];
      grad_y[XX] += mapped_grad(XX,n) * nodes[n][YY];
      grad_y[YY] += mapped_grad(YY,n) * nodes[n][YY];
    }

    const Real jacobian = grad_x[XX]*grad_y[YY]-grad_x[YY]*grad_y[XX];
    
    //Compute the gradient of the solution in physical space
    grad_solution = 0;
    denominator = 0;
    
		for (Uint n=0; n<3; ++n)
    {
      const Real dNdx = 1.0/jacobian * (  grad_y[YY]*mapped_grad(XX,n) - grad_y[XX]*mapped_grad(YY,n) );
      const Real dNdy = 1.0/jacobian * ( -grad_x[YY]*mapped_grad(XX,n) + grad_x[XX]*mapped_grad(YY,n) );
      
      grad_solution[XX] += dNdx*data->solution[node_idx[n]][0];
      grad_solution[YY] += dNdy*data->solution[node_idx[n]][0];
    
      nominator[n] = std::max(0.0,y*dNdx - x*dNdy);
      denominator += nominator[n];
    }


    const Real nablaF = (y*grad_solution[XX] - x*grad_solution[YY]);

    for (Uint n=0; n<3; ++n)
    {
      phi[n] += nominator[n]/denominator * nablaF * w * jacobian;
    }
    
  }
  // Loop over quadrature nodes
  
	for (Uint n=0; n<3; ++n)
    data->residual[node_idx[n]][0] += phi[n];
  
  // For time marching scheme
  
  RealVector centroid(0.0,2);
	for (Uint n=0; n<3; ++n)
  {
    centroid[XX] += nodes[n][XX];
    centroid[YY] += nodes[n][YY];
  }
  centroid /= 3.0;
  
  RealMatrix nodal_normals(2,3);

  nodal_normals(XX,0) = nodes[1][YY] - nodes[2][YY];
  nodal_normals(XX,1) = nodes[2][YY] - nodes[0][YY];
  nodal_normals(XX,2) = nodes[0][YY] - nodes[1][YY];

  nodal_normals(YY,0) = nodes[2][XX] - nodes[1][XX];
  nodal_normals(YY,1) = nodes[0][XX] - nodes[2][XX];
  nodal_normals(YY,2) = nodes[1][XX] - nodes[0][XX];

  Real sum_kplus=0;
  for (Uint n=0; n<3; ++n)
    sum_kplus += 0.5*std::max(0.0,centroid[YY]*nodal_normals(XX,n)-centroid[XX]*nodal_normals(YY,n));
  for (Uint n=0; n<3; ++n)
  {
    // Real kplus = 0.5*std::max(0.0,centroid[YY]*nodal_normals(XX,i)-centroid[XX]*nodal_normals(YY,i));
    data->inverse_updatecoeff[node_idx[n]][0] += sum_kplus; 
  } 
}

////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

////////////////////////////////////////////////////////////////////////////////////

