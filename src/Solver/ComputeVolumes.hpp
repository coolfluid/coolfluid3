#ifndef CF_ComputeVolumes_hpp
#define CF_ComputeVolumes_hpp

#include "Mesh/CRegion.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/ElementNodes.hpp"


/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

/////////////////////////////////////////////////////////////////////////////////////

struct SetX
{
  CArray::Ptr x_coord;
  CArray::Ptr coordinates;
  CTable::Ptr connectivity_table;
  
  SetX () { }
  
  void setup (CElements& field_elements )
  {
    coordinates = field_elements.get_geometry_elements().coordinates().get_type<CArray>();
    connectivity_table = field_elements.get_geometry_elements().connectivity_table().get_type<CTable>();
    x_coord = field_elements.elemental_data().get_type<CArray>();
    x_coord->array().resize(boost::extents[field_elements.elements_count()][1]);
  }
  
  template < typename SFType >
  void execute ( Uint elem )
  {
    cf_assert(x_coord.get());
    //std::vector<RealVector> nodes;
    //fill_node_list( std::inserter(nodes, nodes.begin()), *coordinates, *connectivity_table, elem );
    
    
    //RealVector mapped_coord = RealVector(0.0,SFType::dimension);
    //RealVector shape_func = RealVector(SFType::nb_nodes);
    
    //SFType::shape_function(mapped_coord,shape_func);

    x_coord->array()[elem][0] = 0;
    for (Uint i=0; i<SFType::nb_nodes; i++)
    {
      x_coord->array()[elem][0] += (*coordinates)[ (*connectivity_table)[elem][i] ][XX];
    }
    x_coord->array()[elem][0] /= SFType::nb_nodes;
  }
  
};
  
/////////////////////////////////////////////////////////////////////////////////////
  
struct ComputeGradient
{
  CArray::Ptr gradx;
  CArray::Ptr coordinates;
  CTable::Ptr connectivity_table;
  
  ComputeGradient () { }
  
  void setup (CElements& field_elements )
  {
    gradx = field_elements.elemental_data().get_type<CArray>();
    gradx->array().resize(boost::extents[field_elements.elements_count()][1]);
    
    coordinates = field_elements.get_geometry_elements().coordinates().get_type<CArray>();
    connectivity_table = field_elements.get_geometry_elements().connectivity_table().get_type<CTable>();
    
  }
  
  template < typename SFType >
  void execute ( Uint elem )
  {
    cf_assert(gradx.get());
    std::vector<RealVector> nodes;
    fill_node_list( std::inserter(nodes, nodes.begin()), *coordinates, *connectivity_table, elem );
    
    RealVector mapped_coord = RealVector(0.0,SFType::dimension);
    SFType::mapped_coordinates(nodes[0],nodes,mapped_coord);
    gradx->array()[elem][0] = 0.0;
  }
  
};
  
/////////////////////////////////////////////////////////////////////////////////////

struct ComputeVolumes
{
  CArray::Ptr volumes;
  CArray::Ptr coordinates;
  CTable::Ptr connectivity_table;

  ComputeVolumes () { }
  
  void setup (CFieldElements& field_elements )
  {
    volumes = field_elements.elemental_data().get_type<CArray>();
    volumes->array().resize(boost::extents[field_elements.elements_count()][1]);
    
    coordinates = field_elements.get_geometry_elements().coordinates().get_type<CArray>();
    connectivity_table = field_elements.get_geometry_elements().connectivity_table().get_type<CTable>();
    
  }
  
  template < typename SFType >
  void execute ( Uint elem )
  {
    cf_assert(volumes.get());
    std::vector<RealVector> nodes;
    fill_node_list( std::inserter(nodes, nodes.begin()), *coordinates, *connectivity_table, elem );
    volumes->array()[elem][0] = SFType::volume( nodes );
  }

};
  
/////////////////////////////////////////////////////////////////////////////////////

struct OutputScalarField
{
  CArray::Ptr scalars;
  std::string scalar_name;

  OutputScalarField () { }
  
  void setup (CFieldElements& field_elements )
  {
    scalar_name = field_elements.get_parent()->get_type<CField>()->field_name();
    scalars = field_elements.elemental_data().get_type<CArray>();
    CFinfo << field_elements.full_path().string() << CFendl;
  }
  
  template < typename SFType >
  void execute ( Uint elem )
  {
    cf_assert(scalars.get());
    CFinfo << "   " << scalar_name << "["<<elem<<"] = " << scalars->array()[elem][0] << CFendl;
  }
  
};
  
/////////////////////////////////////////////////////////////////////////////////////

template < typename OP1, typename OP2 >
struct OperationMerge
{
  OP1 m_op1;
  OP2 m_op2;

  void setup (CFieldElements& field_elements )
  {
    m_op1.setup(field_elements);
    m_op2.setup(field_elements);
  }
  
  template < typename EType >
  void execute (  Uint elem )
  {
    m_op1.template execute<EType>( elem );
    m_op2.template execute<EType>( elem );
  }
};

/////////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_ComputeVolumes_hpp
