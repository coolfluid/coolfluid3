// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CreateComponent.hpp"

#include "Mesh/COperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {
namespace Mesh {

///////////////////////////////////////////////////////////////////////////////////////
  
COperation::COperation ( const std::string& name ) : 
  Component(name), m_counter(0)
{
    tag_component(this);
}

///////////////////////////////////////////////////////////////////////////////////////

void COperation::set_loophelper (CElements& geometry_elements )
{
  throw Common::NotImplemented(FromHere(), "Must create child that overloads this function");
}
  
void COperation::set_loophelper ( CTable<Real>& coordinates )
{
  throw Common::NotImplemented(FromHere(), "Must create child that overloads this function");
}

///////////////////////////////////////////////////////////////////////////////////////

//void COperation::execute (  Uint index )
//{
//  throw NotImplemented(FromHere(), "Must create child that overloads this function");
//}

///////////////////////////////////////////////////////////////////////////////////////

COperation& COperation::operation()
{
  throw NotImplemented(FromHere(), "Must create child that overloads this function");
}

///////////////////////////////////////////////////////////////////////////////////////

COperation& COperation::create_operation(const std::string operation_type)
{
  // The execuation of operations must be in chronological order, hence
  // they get an alphabetical name
  std::string name = "operation_"+String::to_str(++m_counter);
  COperation::Ptr sub_operation = 
    create_component_abstract_type<COperation>(operation_type,name);
  add_component(sub_operation);
  return *sub_operation;
}

///////////////////////////////////////////////////////////////////////////////////////
  
Common::ComponentBuilder < COperationMerge, COperation, LibMesh > COperationMerge_Builder;

Common::ComponentBuilder < COutputField,    COperation, LibMesh > COutputField_Builder;

Common::ComponentBuilder < CComputeVolumes, COperation, LibMesh > CComputeVolume_Builder;

Common::ComponentBuilder < CSetValue,       COperation, LibMesh > CSetValue_Builder;

//
//struct SetX
//{
//  CTable<Real>::Ptr x_coord;
//  CTable<Real>::Ptr coordinates;
//  CTable<Uint>::Ptr connectivity_table;
//  
//  SetX () { }
//  
//  void setup (CFieldElements& field_elements )
//  {
//    coordinates = field_elements.get_geometry_elements().coordinates().get_type<CTable<Real> >();
//    connectivity_table = field_elements.get_geometry_elements().connectivity_table().get_type<CTable<Uint> >();
//    x_coord = field_elements.elemental_data().get_type<CTable<Real> >();
//  }
//  
//  template < typename SFType >
//  void execute ( Uint elem )
//  {
//    cf_assert(x_coord.get());
//    //std::vector<RealVector> nodes;
//    //fill_node_list( nodes, *coordinates, *connectivity_table, elem );
//    
//    
//    //RealVector mapped_coord = RealVector(0.0,SFType::dimension);
//    //RealVector shape_func = RealVector(SFType::nb_nodes);
//    
//    //SFType::shape_function(mapped_coord,shape_func);
//
//    x_coord->array()[elem][0] = 0;
//    for (Uint i=0; i<SFType::nb_nodes; i++)
//    {
//      x_coord->array()[elem][0] += (*coordinates)[ (*connectivity_table)[elem][i] ][XX];
//    }
//    x_coord->array()[elem][0] /= SFType::nb_nodes;
//  }
//  
//};
//  
///////////////////////////////////////////////////////////////////////////////////////
//  
//struct ComputeGradient
//{
//  CTable<Real>::Ptr gradx;
//  CTable<Real>::Ptr coordinates;
//  CTable<Uint>::Ptr connectivity_table;
//  
//  ComputeGradient () { }
//  
//  void setup (CFieldElements& field_elements )
//  {
//    gradx = field_elements.elemental_data().get_type<CTable<Real> >();
//    
//    coordinates = field_elements.coordinates().get_type<CTable<Real> >();
//    connectivity_table = field_elements.connectivity_table().get_type<CTable<Uint> >();
//    
//  }
//  
//  template < typename SFType >
//  void execute ( Uint elem )
//  {
//    cf_assert(gradx.get());
//    std::vector<RealVector> nodes;
//    fill_node_list( nodes, *coordinates, *connectivity_table, elem );
//    
//    RealVector nodal_function_values(SFType::nb_nodes);
//    for (Uint i=0; i<SFType::nb_nodes; i++)
//    {
//      nodal_function_values[i] = (*coordinates)[ (*connectivity_table)[elem][i] ][XX];
//    }
//    
//    RealVector mapped_coords = RealVector(0.0,SFType::dimension);
//    SFType::mapped_coordinates(nodes[0],nodes,mapped_coords);    
//    
//    // Get the gradient in mapped coordinates
//    RealMatrix mapped_grad(SFType::dimensionality,SFType::nb_nodes);
//    SFType::mapped_gradient(mapped_coords,mapped_grad);
//    
//    // The Jacobian adjugate
//    RealMatrix jacobian_adj(SFType::dimension, SFType::dimensionality);
//    SFType::jacobian_adjoint(mapped_coords, nodes, jacobian_adj);
//    
//    // The gradient operator matrix in the absolute frame
//    RealMatrix grad(SFType::dimension,SFType::nb_nodes);
//    grad = (jacobian_adj * mapped_grad) / SFType::jacobian_determinant(mapped_coords, nodes);
//    
//    // Apply the gradient to the function values
//    RealVector result(SFType::dimension);
//    result = grad * nodal_function_values;
//    gradx->set_row(elem,result);
//  }
//  
//};
//  
///////////////////////////////////////////////////////////////////////////////////////
//
//struct ComputeVolumes
//{
//  CTable<Real>::Ptr volumes;
//  CTable<Real>::Ptr coordinates;
//  CTable<Uint>::Ptr connectivity_table;
//
//  ComputeVolumes () { }
//  
//  void setup (CFieldElements& field_elements )
//  {
//    volumes = field_elements.elemental_data().get_type<CTable<Real> >();
//    volumes->array().resize(boost::extents[field_elements.elements_count()][1]);
//    
//    coordinates = field_elements.get_geometry_elements().coordinates().get_type<CTable<Real> >();
//    connectivity_table = field_elements.get_geometry_elements().connectivity_table().get_type<CTable<Uint> >();
//    
//  }
//  
//  template < typename SFType >
//  void execute ( Uint elem )
//  {
//    cf_assert(volumes.get());
//    std::vector<RealVector> nodes;
//    fill_node_list( nodes, *coordinates, *connectivity_table, elem );
//    volumes->array()[elem][0] = SFType::volume( nodes );
//  }
//
//};
//  
///////////////////////////////////////////////////////////////////////////////////////
//
//struct OutputScalarField
//{
//  CTable<Real>::Ptr scalars;
//  std::string scalar_name;
//
//  OutputScalarField () { }
//  
//  void setup (CFieldElements& field_elements )
//  {
//    scalar_name = field_elements.get_parent()->get_type<CField>()->field_name();
//    scalars = field_elements.elemental_data().get_type<CTable<Real> >();
//    CFinfo << field_elements.full_path().string() << CFendl;
//  }
//  
//  template < typename SFType >
//  void execute ( Uint elem )
//  {
//    cf_assert(scalars.get());
//    CFinfo << "   " << scalar_name << "["<<elem<<"] = " << scalars->array()[elem][0] << CFendl;
//  }
//};
//  
///////////////////////////////////////////////////////////////////////////////////////
//
//struct OutputVectorField
//{
//  CTable<Real>::Ptr vectors;
//  std::string vector_name;
//  
//  OutputVectorField () { }
//  
//  void setup (CFieldElements& field_elements )
//  {
//    vector_name = field_elements.get_parent()->get_type<CField>()->field_name();
//    vectors = field_elements.elemental_data().get_type<CTable<Real> >();
//    CFinfo << field_elements.full_path().string() << CFendl;
//  }
//  
//  template < typename SFType >
//  void execute ( Uint elem )
//  {
//    cf_assert(vectors.get());
//    CFinfo << "   " << vector_name << "["<<elem<<"] = " << RealVector(vectors->array()[elem]) << CFendl;
//  }
//};
//  
///////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////////

