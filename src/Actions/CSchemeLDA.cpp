// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"
#include "Common/OptionT.hpp"

#include "Math/RealMatrix.hpp"
#include "Math/MathChecks.hpp"

#include "Mesh/SF/Triag2DLagrangeP1.hpp"
#include "Actions/CSchemeLDA.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh::SF;
namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < CSchemeLDA, CAction, LibActions, NB_ARGS_1 > CSchemeLDAProviderCAction( "CSchemeLDA" );
Common::ObjectProvider < CSchemeLDA, CElementOperation, LibActions, NB_ARGS_1 > CSchemeLDAProviderCElementOperation( "CSchemeLDA" );

///////////////////////////////////////////////////////////////////////////////////////
  
void CSchemeLDA::defineConfigProperties( Common::PropertyList& options )
{
  options.add_option< OptionT<URI> > ("SolutionField","Solution Field for calculation", URI("cpath://"))->mark_basic();
  options.add_option< OptionT<URI> > ("ResidualField","Residual Field updated after calculation", URI("cpath://"))->mark_basic();
  options.add_option< OptionT<URI> > ("InverseUpdateCoeff","Inverse update coefficient Field updated after calculation", URI("cpath://"))->mark_basic();
}

///////////////////////////////////////////////////////////////////////////////////////

CSchemeLDA::CSchemeLDA ( const CName& name ) : 
  CElementOperation(name)
{
  BUILD_COMPONENT;
  m_property_list["SolutionField"].as_option().attach_trigger ( boost::bind ( &CSchemeLDA::trigger_SolutionField,   this ) );  
  m_property_list["ResidualField"].as_option().attach_trigger ( boost::bind ( &CSchemeLDA::trigger_ResidualField,   this ) );  
  m_property_list["InverseUpdateCoeff"].as_option().attach_trigger ( boost::bind ( &CSchemeLDA::trigger_InverseUpdateCoeff,   this ) );  
  
  
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

void CSchemeLDA::trigger_SolutionField()
{
  CPath field_path (property("SolutionField").value<URI>());
  m_solution_field = look_component_type<CField>(field_path);
}

void CSchemeLDA::trigger_ResidualField()
{
  CPath field_path (property("ResidualField").value<URI>());
  m_residual_field = look_component_type<CField>(field_path);
}

void CSchemeLDA::trigger_InverseUpdateCoeff()
{
  CPath field_path (property("InverseUpdateCoeff").value<URI>());
  m_inverseUpdateCoeff = look_component_type<CField>(field_path);
}

/////////////////////////////////////////////////////////////////////////////////////

void CSchemeLDA::execute()
{
  static const Uint fix = 0;

  // inside element with index m_elm_idx
  RealMatrix mapped_grad(2,3);
  RealVector shapefunc(3);
  RealVector grad_solution(2);
  RealVector grad_x(2);
  RealVector grad_y(2);
  CF::Real denominator;
  RealVector nominator(3);
  RealVector phi(3);

  phi = 0.;
  
  if (m_elm_idx == fix)
    CFinfo << "elem [" << m_elm_idx << "]" << CFendl;
  if (m_elm_idx == fix)
    BOOST_FOREACH(const Uint node, data->connectivity_table[m_elm_idx])
      CFinfo << "n(" << node << ") ["  << data->coordinates[node][XX] << ":" << data->coordinates[node][YY] << "]" << CFendl;

  for (Uint q=0; q<nb_q; ++q)
  {
    Triag2DLagrangeP1::mapped_gradient(mapped_coords[q],mapped_grad);
    Triag2DLagrangeP1::shape_function(mapped_coords[q], shapefunc);
    
    Real x=0;
    Real y=0;
    Uint node_counter = 0;
    BOOST_FOREACH(const Uint node, data->connectivity_table[m_elm_idx])
    {
      x += shapefunc[node_counter] * data->coordinates[node][XX];
      y += shapefunc[node_counter] * data->coordinates[node][YY];
      ++node_counter;
    }
    
    denominator = 0;
    grad_solution = 0;
    grad_x = 0;
    grad_y = 0;
    node_counter = 0;
    BOOST_FOREACH(const Uint node, data->connectivity_table[m_elm_idx])
    {
      nominator[node_counter] = std::max(0.0,y*mapped_grad(XX,node_counter) - x*mapped_grad(YY,node_counter));
      denominator += nominator[node_counter];
      grad_solution[XX] += mapped_grad(XX,node_counter) * data->solution[node][0];
      grad_solution[YY] += mapped_grad(YY,node_counter) * data->solution[node][0];
      grad_x[XX] += mapped_grad(XX,node_counter) * data->coordinates[node][XX];
      grad_x[YY] += mapped_grad(YY,node_counter) * data->coordinates[node][XX];
      grad_y[XX] += mapped_grad(XX,node_counter) * data->coordinates[node][YY];
      grad_y[YY] += mapped_grad(YY,node_counter) * data->coordinates[node][YY];

      ++node_counter;

    }

    Real jacobian = grad_x[XX]*grad_y[YY]-grad_x[YY]*grad_y[XX];

    for (Uint i=0; i<3; ++i)
    {
      phi[i] += nominator[i]/denominator * (y*grad_solution[XX] - x*grad_solution[YY]) * w * jacobian;
    }
  }
  
  
  Uint node_counter = 0;
  BOOST_FOREACH(const Uint node, data->connectivity_table[m_elm_idx])
  {
    if (m_elm_idx == fix)
      CFinfo << "phi(" << node_counter << ") ["  << phi[node_counter] << "]" << CFendl;
    data->residual[node][0] += phi[node_counter];
    ++node_counter;
  }  
  
  
  // For time marching scheme
  
  RealVector centroid(0.0,2);
  BOOST_FOREACH(const Uint node, data->connectivity_table[m_elm_idx])
  {
    centroid[XX] += data->coordinates[node][XX];
    centroid[YY] += data->coordinates[node][YY];
    ++node_counter;
  }
  centroid /= 3.0;
  
  RealMatrix nodal_normals(2,3);
  CTable::ConstRow nodes = data->connectivity_table[m_elm_idx];

  nodal_normals(XX,0) = data->coordinates[nodes[1]][YY] - data->coordinates[nodes[2]][YY];
  nodal_normals(XX,1) = data->coordinates[nodes[2]][YY] - data->coordinates[nodes[0]][YY];
  nodal_normals(XX,2) = data->coordinates[nodes[0]][YY] - data->coordinates[nodes[1]][YY];

  nodal_normals(YY,0) = data->coordinates[nodes[2]][XX] - data->coordinates[nodes[1]][XX];
  nodal_normals(YY,1) = data->coordinates[nodes[0]][XX] - data->coordinates[nodes[2]][XX];
  nodal_normals(YY,2) = data->coordinates[nodes[1]][XX] - data->coordinates[nodes[0]][XX];

  if (m_elm_idx == fix)
    CFinfo << "nnormals(" << nodal_normals << "]" << CFendl;

  Real sumK=0;
  for (Uint i=0; i<3; ++i)
  {
    sumK += 0.5*std::max(0.0,centroid[YY]*nodal_normals(XX,i)-centroid[XX]*nodal_normals(YY,i));
  }   
  for (Uint i=0; i<3; ++i)
  {
    // Real kplus = 0.5*std::max(0.0,centroid[YY]*nodal_normals(XX,i)-centroid[XX]*nodal_normals(YY,i));
    data->inverse_updatecoeff[nodes[i]][0] += sumK; 
  } 

  if (m_elm_idx == fix)
    CFinfo << "nnormals(" << nodal_normals << "]" << CFendl;


//  if ( Math::MathChecks::isZero( data->coordinates[nodes[0]][YY] )
//    || Math::MathChecks::isZero( data->coordinates[nodes[1]][YY] )
//    || Math::MathChecks::isZero( data->coordinates[nodes[2]][YY] ) )
//  {
//    CFinfo << "elem [" << m_elm_idx << "]" << CFendl;
//  }


}

////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

////////////////////////////////////////////////////////////////////////////////////

