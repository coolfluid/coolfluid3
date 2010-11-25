// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"

#include "Math/MatrixTypes.hpp"
#include "Math/MathChecks.hpp"

#include "Mesh/ElementData.hpp"
#include "Mesh/SF/Triag2DLagrangeP1.hpp"
#include "Actions/CSchemeLDA.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh::SF;
namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CSchemeLDA, CAction, LibActions > CSchemeLDACAction_Builder( "CSchemeLDA" );
Common::ComponentBuilder < CSchemeLDA, CLoopOperation, LibActions > CSchemeLDACLoopOperation_Builder( "CSchemeLDA" );

///////////////////////////////////////////////////////////////////////////////////////
  
void CSchemeLDA::define_config_properties( Common::PropertyList& options )
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

CSchemeLDA::CSchemeLDA ( const std::string& name ) : 
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

  typedef Triag2DLagrangeP1 SF;

  const CTable::ConstRow node_idx = data->connectivity_table[m_idx];
  SF::NodeMatrixT nodes;
  fill(nodes, data->coordinates, data->connectivity_table[m_idx]);

  SF::MappedGradientT mapped_grad; //Gradient of the shape functions in reference space
  SF::ShapeFunctionsT shapefunc;     //Values of shape functions in reference space
  SF::CoordsT grad_solution;
  RealMatrix2 grad_xy;
  Real denominator;
  RealVector3 nominator;
  RealVector3 phi;
  SF::CoordsT xy;
  
  phi.setZero();
      
  for (Uint q=0; q<nb_q; ++q) //Loop over quadrature points
  {
    Triag2DLagrangeP1::mapped_gradient(mapped_coords[q],mapped_grad);
    Triag2DLagrangeP1::shape_function(mapped_coords[q], shapefunc);
    
    xy = shapefunc * nodes;
    grad_xy = mapped_grad * nodes;

    const Real jacobian = grad_xy.determinant();
    
    //Compute the gradient of the solution in physical space
    grad_solution.setZero();
    denominator = 0;
    
		for (Uint n=0; n<3; ++n)
    {
      const Real dNdx = 1.0/jacobian * (  grad_xy(YY, YY)*mapped_grad(XX,n) - grad_xy(XX, YY)*mapped_grad(YY,n) );
      const Real dNdy = 1.0/jacobian * ( -grad_xy(YY, XX)*mapped_grad(XX,n) + grad_xy(XX, XX)*mapped_grad(YY,n) );
      
      grad_solution[XX] += dNdx*data->solution[node_idx[n]][0];
      grad_solution[YY] += dNdy*data->solution[node_idx[n]][0];
    
      nominator[n] = std::max(0.0,xy[YY]*dNdx - xy[XX]*dNdy);
      denominator += nominator[n];
    }

    const Real nablaF = (xy[YY]*grad_solution[XX] - xy[XX]*grad_solution[YY]);

    for (Uint n=0; n<3; ++n)
    {
      phi[n] += nominator[n]/denominator * nablaF * w * jacobian;
    }
    
  }
  // Loop over quadrature nodes
  
  for (Uint n=0; n<3; ++n)
    data->residual[node_idx[n]][0] += phi[n];
  
  // For time marching scheme  
  SF::CoordsT centroid = nodes.colwise().sum() / 3.;
  
  RealMatrix nodal_normals(2,3);

  nodal_normals(XX,0) = nodes(1, YY) - nodes(2, YY);
  nodal_normals(XX,1) = nodes(2, YY) - nodes(0, YY);
  nodal_normals(XX,2) = nodes(0, YY) - nodes(1, YY);

  nodal_normals(YY,0) = nodes(2, XX) - nodes(1, XX);
  nodal_normals(YY,1) = nodes(0, XX) - nodes(2, XX);
  nodal_normals(YY,2) = nodes(1, XX) - nodes(0, XX);

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

